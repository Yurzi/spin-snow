#include "model.h"

#include <algorithm>
#include <iostream>
#include <string_view>


#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <stdint.h>

#include "utils.h"

void Model::load(const std::string &file_path, bool filpUV, bool genNormal) noexcept {
  uint32_t pFlags = aiProcess_Triangulate;
  if (filpUV)
    pFlags |= aiProcess_FlipUVs;
  if (genNormal)
    pFlags |= aiProcess_GenNormals;

  load(file_path, pFlags);
}

void Model::load(const std::string &file_path, uint32_t aiProcessFlags) {
  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(file_path, aiProcessFlags);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    std::cout << "[ERROR::ASSIMP] Failed to load model: " << importer.GetErrorString() << std::endl;
    return;
  }

  root_dir = file_path.substr(0, file_path.find_last_of('/'));
  for (uint32_t i = 0; i < scene->mNumMeshes; ++i) {
    aiMesh *aimesh = scene->mMeshes[i];
    meshs.push_back(processMesh(aimesh, scene));
  }
}

Mesh Model::processMesh(const aiMesh *mesh, const aiScene *scene) noexcept {
  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;
  std::vector<Texture> textures;

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
    std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE);
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR);
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
  }
  return std::move(Mesh(vertices, indices, textures));
}

void Model::draw(std::shared_ptr<ShaderProgram> shader) noexcept {}


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
    break;
  }
  return custom_type;
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *material, aiTextureType type) {
  std::vector<Texture> textures_tmp;
  for (uint32_t i = 0; i < material->GetTextureCount(type); ++i) {
    aiString str;
    material->GetTexture(type, i, &str);
    std::string texture_path = str.C_Str();
    texture_path = texture_path.substr(texture_path.find_last_of('/'));
    texture_path = root_dir + '/' + texture_path;

    // 检查是否已经加载
    if (texture_loaded.find(texture_path) == texture_loaded.end()) {
      // 未加载
      Texture texture;
      texture.id = Texture2DFromFile(texture_path);
      texture.type = convert_from_aiTextureType(type);
      texture_path = texture_path;

      textures_tmp.push_back(texture);
      texture_loaded.insert(std::pair<std::string, Texture>(texture_path, texture));
    } else {
      textures_tmp.push_back(texture_loaded.find(texture_path)->second);
    }
  }
  return textures_tmp;
}
