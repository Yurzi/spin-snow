#include "model.h"

#include <algorithm>
#include <iostream>
#include <string_view>


#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdint.h>

#include "utils.h"

Model::Model(const Model &oth) {
  this->translate = oth.translate;
  this->rotate = oth.rotate;
  this->scale = oth.scale;

  this->model_path = oth.model_path;
  this->aiProcessFlags = oth.aiProcessFlags;
  this->root_dir = oth.root_dir;

  this->meshs = oth.meshs;
  this->texture_loaded = oth.texture_loaded;

  this->has_loaded = oth.has_loaded;

  load(this->model_path, this->aiProcessFlags);
}
Model::Model(Model &&oth) {
  this->translate = std::move(oth.translate);
  this->rotate = std::move(oth.rotate);
  this->scale = std::move(oth.scale);

  this->model_path = std::move(oth.model_path);
  this->aiProcessFlags = oth.aiProcessFlags;
  this->root_dir = std::move(oth.root_dir);

  this->meshs = std::move(oth.meshs);
  oth.meshs.clear();
  this->texture_loaded = std::move(oth.texture_loaded);
  oth.texture_loaded.clear();

  this->has_loaded = oth.has_loaded;

  load(this->model_path, this->aiProcessFlags);
}
Model &Model::operator=(const Model &oth) noexcept {
  this->translate = oth.translate;
  this->rotate = oth.rotate;
  this->scale = oth.scale;

  this->model_path = oth.model_path;
  this->aiProcessFlags = oth.aiProcessFlags;
  this->root_dir = oth.root_dir;

  this->meshs = oth.meshs;
  this->texture_loaded = oth.texture_loaded;

  this->has_loaded = oth.has_loaded;

  load(this->model_path, this->aiProcessFlags);
  return (*this);
}
Model &Model::operator=(Model &&oth) noexcept {
  this->translate = std::move(oth.translate);
  this->rotate = std::move(oth.rotate);
  this->scale = std::move(oth.scale);

  this->model_path = std::move(oth.model_path);
  this->aiProcessFlags = oth.aiProcessFlags;
  this->root_dir = std::move(oth.root_dir);

  this->meshs = std::move(oth.meshs);
  oth.meshs.clear();
  this->texture_loaded = std::move(oth.texture_loaded);
  oth.texture_loaded.clear();

  this->has_loaded = oth.has_loaded;

  load(this->model_path, this->aiProcessFlags);
  return (*this);
}

Model::~Model() {}

void Model::load(const std::string &file_path, bool flipUV, bool genNormal) noexcept {
  if (has_loaded) {
    return;
  }
  uint32_t pFlags = aiProcess_Triangulate;
  if (flipUV)
    pFlags |= aiProcess_FlipUVs;
  if (genNormal)
    pFlags |= aiProcess_GenSmoothNormals;

  load(file_path, pFlags);
}

void Model::load(const std::string &file_path, uint32_t aiProcessFlags) {
  if (has_loaded) {
    return;
  }

  this->model_path = file_path;
  this->aiProcessFlags = aiProcessFlags;
  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(file_path, aiProcessFlags);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    std::cout << "[ERROR::ASSIMP] Failed to load model: " << importer.GetErrorString() << std::endl;
    return;
  }

  root_dir = file_path.substr(0, file_path.find_last_of('/'));
  for (uint32_t i = 0; i < scene->mNumMeshes; ++i) {
    aiMesh *aimesh = scene->mMeshes[i];
    meshs.push_back(std::move(processMesh(aimesh, scene)));
  }
  has_loaded = true;
}

Mesh Model::processMesh(const aiMesh *mesh, const aiScene *scene) noexcept {
  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;
  std::vector<Texture::Ptr> textures;

  // 处理顶点
  for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
    Vertex vertex;
    // 位置
    vertex.Position.x = mesh->mVertices[i].x;
    vertex.Position.y = mesh->mVertices[i].y;
    vertex.Position.z = mesh->mVertices[i].z;

    // 法线
    vertex.Normal.x = mesh->mNormals[i].x;
    vertex.Normal.y = mesh->mNormals[i].y;
    vertex.Normal.z = mesh->mNormals[i].z;

    uint32_t j = 0;
    while (mesh->HasTextureCoords(j)) {
      vertex.TexCoords.push_back({mesh->mTextureCoords[j][i].x, mesh->mTextureCoords[j][i].y});
      ++j;
    }

    vertices.push_back(std::move(vertex));
  }

  // 处理索引
  for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
    const aiFace &face = mesh->mFaces[i];
    for (uint32_t j = 0; j < face.mNumIndices; ++j) {
      indices.push_back(face.mIndices[j]);
    }
  }

  // 处理材质
  if (mesh->mMaterialIndex >= 0) {
    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
    std::vector<Texture::Ptr> diffuseMaps = loadMaterialTextures(scene, material, aiTextureType_DIFFUSE);
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    std::vector<Texture::Ptr> specularMaps = loadMaterialTextures(scene, material, aiTextureType_SPECULAR);
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
  }
  return std::move(Mesh(vertices, indices, textures));
}

void Model::draw(ShaderProgram::Ptr shader, Camera::Ptr camera) noexcept {
  // 传模型矩阵
  glm::mat4 unit(1.0f);  // 单位矩阵
  glm::mat4 scale = glm::scale(unit, this->scale);
  glm::mat4 translate = glm::translate(unit, this->translate);

  glm::mat4 rotate = unit;  // 旋转
  rotate = glm::rotate(rotate, glm::radians(this->rotate.x), glm::vec3(1, 0, 0));
  rotate = glm::rotate(rotate, glm::radians(this->rotate.y), glm::vec3(0, 1, 0));
  rotate = glm::rotate(rotate, glm::radians(this->rotate.z), glm::vec3(0, 0, 1));

  // 模型变换矩阵
  glm::mat4 model = translate * rotate * scale;
  shader->set_uniform("model", model);

  // 计算模型矩阵逆矩阵的转置
  glm::mat4 NormalMatrix = glm::transpose(glm::inverse(model));
  shader->set_uniform("NormalMatrix", NormalMatrix);

  shader->set_uniform("view", camera->getViewMatrix());
  shader->set_uniform("projection", camera->getProjectionMatrix());

  for (uint32_t i = 0; i < meshs.size(); ++i) {
    meshs[i].draw(shader);
  }
}


Texture::Type convert_from_aiTextureType(aiTextureType aitype) {
  Texture::Type custom_type;
  switch (aitype) {
  case aiTextureType_DIFFUSE:
    custom_type = Texture::diffuse;
    break;
  case aiTextureType_SPECULAR:
    custom_type = Texture::specular;
    break;
  default:
    custom_type = Texture::unknown;
    break;
  }
  return custom_type;
}

std::vector<Texture::Ptr>
Model::loadMaterialTextures(const aiScene *scene, const aiMaterial *material, const aiTextureType type) {
  std::vector<Texture::Ptr> textures_tmp;
  uint32_t i = 0;
  for (i = 0; i < material->GetTextureCount(type); ++i) {
    aiString str;
    material->GetTexture(type, i, &str);
    std::string texture_path = str.C_Str();
    int64_t last_slash_pos = texture_path.find_last_of('/');
    texture_path = root_dir + '/' + texture_path;

    // 检查是否已经加载
    if (texture_loaded.find(texture_path) == texture_loaded.end()) {
      // 未加载
      Texture::Ptr texture = std::make_shared<Texture>(convert_from_aiTextureType(type));
      const aiTexture *aitexture = scene->GetEmbeddedTexture(texture_path.c_str());
      if (aitexture != nullptr) {
        texture->id = Texture2DFromAssimp(aitexture, GL_CLAMP_TO_EDGE);
      } else {
        texture->id = Texture2DFromFile(texture_path, GL_CLAMP_TO_EDGE);
      }

      texture->type = convert_from_aiTextureType(type);
      texture->path = texture_path;
      textures_tmp.push_back(texture);
      texture_loaded.insert(std::pair<std::string, Texture::Ptr>(texture->path, texture));
    } else {
      textures_tmp.push_back(texture_loaded.find(texture_path)->second);
    }
  }
  // 检查是否为不存在而退出
  if (i == 0) {
    Texture::Ptr texture = std::make_shared<Texture>(convert_from_aiTextureType(type));
    const std::string default_texture_path = texture->path;
    // 添加默认材质 [hard code may be unsafe consider random string]
    if (texture_loaded.find(default_texture_path) == texture_loaded.end()) {
      texture->id = Texture2DFromUChar(nullptr);
      texture->path = default_texture_path;
      textures_tmp.push_back(texture);
      texture_loaded.insert(std::pair<std::string, Texture::Ptr>(texture->path, texture));
    } else {
      textures_tmp.push_back(texture_loaded.find(default_texture_path)->second);
    }
  }
  return textures_tmp;
}

void Model::add_texture(Texture::Ptr texture) noexcept {
  for (auto &i : meshs) {
    i.add_texture(texture);
  }
}
