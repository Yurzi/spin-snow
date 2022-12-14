#include "mesh.h"

#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>

#include <ctime>
#include <iostream>
#include <string>


#include "utils.h"


struct VertexInner {
  glm::vec3 Position;      // 位置向量
  glm::vec3 Normal;        // 法线
  glm::vec2 TexCoords[0];  // 纹理坐标 assimp 允许一个顶点有多个纹理坐标
};


Texture::Texture(Texture::Type type, GLuint id) {
  this->type = type;
  this->id = id;
  this->path += std::to_string(type);
  if (type == Texture::unknown) {
    this->path += ":";
    this->path += std::to_string(clock());
  }
}
Texture::Texture(
  const std::string &path, Texture::Type type, bool need_vFlip, GLenum wrapMode, GLenum magFilterMode, GLenum minFilterMode) {
  this->path = path;
  this->type = type;
  stbi_set_flip_vertically_on_load_thread(need_vFlip);
  this->id = Texture2DFromFile(path, wrapMode, magFilterMode, minFilterMode);
  stbi_set_flip_vertically_on_load_thread(false);
}

Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<GLuint> &indices, const std::vector<Texture::Ptr> &textures) {
  // 赋值
  this->vertices = vertices;
  this->indices = indices;
  this->textures = textures;


  setup();
}

Mesh::Mesh(const Mesh &oth) {
  // 基础属性
  this->vertices = oth.vertices;
  this->indices = oth.indices;
  this->textures = oth.textures;

  this->translate = oth.translate;
  this->rotate = oth.rotate;
  this->scale = oth.scale;

  this->texcoords_layers = oth.texcoords_layers;
  this->has_setup = oth.has_setup;
  this->vao = oth.vao;
  this->vbo = oth.vbo;
  this->ebo = oth.ebo;

  setup();
}

Mesh::Mesh(Mesh &&oth) {
  this->vertices = std::move(oth.vertices);
  this->indices = std::move(oth.indices);
  this->textures = std::move(oth.textures);

  this->translate = std::move(oth.translate);
  this->rotate = std::move(oth.rotate);
  this->scale = std::move(oth.scale);

  this->texcoords_layers = oth.texcoords_layers;
  oth.texcoords_layers = 0;
  this->current_shader = oth.current_shader;
  oth.current_shader = 0;

  this->has_setup = oth.has_setup;
  this->shader_vao_map = std::move(oth.shader_vao_map);
  oth.shader_vao_map.clear();

  this->vao = oth.vao;
  oth.vao = GL_ZERO;
  this->vbo = oth.vbo;
  oth.vbo.reset();
  this->ebo = oth.ebo;
  oth.ebo.reset();

  setup();
}
Mesh &Mesh::operator=(const Mesh &oth) noexcept {
  // 基础属性
  this->vertices = oth.vertices;
  this->indices = oth.indices;
  this->textures = oth.textures;

  this->translate = oth.translate;
  this->rotate = oth.rotate;
  this->scale = oth.scale;

  this->texcoords_layers = oth.texcoords_layers;
  this->has_setup = oth.has_setup;
  this->vao = oth.vao;
  this->vbo = oth.vbo;
  this->ebo = oth.ebo;

  setup();
  return (*this);
}
Mesh &Mesh::operator=(Mesh &&oth) noexcept {
  this->vertices = std::move(oth.vertices);
  this->indices = std::move(oth.indices);
  this->textures = std::move(oth.textures);

  this->translate = std::move(oth.translate);
  this->rotate = std::move(oth.rotate);
  this->scale = std::move(oth.scale);

  this->texcoords_layers = oth.texcoords_layers;
  oth.texcoords_layers = 0;
  this->current_shader = oth.current_shader;
  oth.current_shader = 0;

  this->has_setup = oth.has_setup;
  this->shader_vao_map = std::move(oth.shader_vao_map);
  oth.shader_vao_map.clear();

  this->vao = oth.vao;
  oth.vao = GL_ZERO;
  this->vbo = oth.vbo;
  oth.vbo.reset();
  this->ebo = oth.ebo;
  oth.ebo.reset();

  setup();
  return (*this);
}
Mesh::~Mesh() {
  // 释放缓冲对象
  std::vector<GLuint> VAOs;
  for (const auto &i : shader_vao_map) {
    VAOs.push_back(i.second);
  }
  glDeleteBuffers(VAOs.size(), VAOs.data());
}

void Mesh::setup() noexcept {
  if (has_setup)
    return;
  // 获取一个顶点最多能绑定的纹理坐标
  for (auto idx : vertices) {
    if (idx.TexCoords.size() > this->texcoords_layers) {
      this->texcoords_layers = idx.TexCoords.size();
    }
  }
  // 确保顶点纹理坐标层数一直
  for (auto &idx : this->vertices) {
    for (GLuint current_layers = idx.TexCoords.size(); current_layers < this->texcoords_layers; ++current_layers) {
      idx.TexCoords.push_back({0.0f, 0.0f});
    }
  }

  // 对于VBO指针的解析需要着色器对象，需要在draw call时进行
  // 同时要进行记忆, 所以将其和VAO的绑定将移动至draw call前进行
  this->vbo = std::make_shared<VBO>();
  this->ebo = std::make_shared<EBO>();

  /*--------------------EBO----------------------------*/
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo->id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

  /*--------------------VBO----------------------------*/
  glBindBuffer(GL_ARRAY_BUFFER, vbo->id);
  // 申请缓冲空间
  GLuint perVertexSize = sizeof(VertexInner) + sizeof(glm::vec2) * this->texcoords_layers;
  // 将位置信息加入到缓冲中
  // 1.构建传输用数组 [unsafe]
  unsigned char *inner_data = (unsigned char *)malloc(perVertexSize * this->vertices.size());
  // 2.复制数据到此连续内存空间
  for (int i = 0; i < this->vertices.size(); ++i) {
    VertexInner *ptr = (VertexInner *)(inner_data + i * perVertexSize);
    ptr->Position = this->vertices[i].Position;
    ptr->Normal = this->vertices[i].Normal;
    // memory copy [unsafe]
#ifdef _WIN32_
    memcpy_s(ptr->TexCoords,
             sizeof(glm::vec2) * this->texcoords_layers,
             this->vertices[i].TexCoords.data(),
             sizeof(glm::vec2) * this->texcoords_layers);
#else
    memcpy(ptr->TexCoords, this->vertices[i].TexCoords.data(), sizeof(glm::vec2) * this->texcoords_layers);
#endif
  }

  // 3.从内存空间将数据发送到GPU buffer
  glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * perVertexSize, inner_data, GL_STATIC_DRAW);
  // 5.finish
  free(inner_data);

  glBindBuffer(GL_ARRAY_BUFFER, GL_ZERO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_ZERO);
  this->has_setup = true;
}

void Mesh::prepare_draw(ShaderProgram::Ptr shader) noexcept {
  // O(1) check
  if (shader->get_id() == this->current_shader) {
    return;
  }
  // 切换为指定的shader
  this->current_shader = shader->get_id();
  // 查找是否已经生成VAO
  if (shader_vao_map.find(current_shader) != shader_vao_map.end()) {
    this->vao = shader_vao_map.find(current_shader)->second;
    return;
  }

  // 如果未找到对应的VAO，则需要生成
  glGenVertexArrays(1, &(this->vao));
  shader_vao_map.insert({current_shader, this->vao});
  // 进行VBO 和 EBO的绑定
  glBindVertexArray(this->vao);

  /*-----VBO-------*/
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo->id);
  // 绑定指针
  GLint location = -1;
  GLuint perVertexSize = sizeof(VertexInner) + sizeof(glm::vec2) * this->texcoords_layers;
  // 顶点位置
  location = glGetAttribLocation(current_shader, shader_postion_in.c_str());
  if (location >= 0) {
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, perVertexSize, (GLvoid *)0);
  }
  // 法线向量
  location = glGetAttribLocation(current_shader, shader_normal_in.c_str());
  if (location >= 0) {
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, perVertexSize, (GLvoid *)offsetof(VertexInner, Normal));
  }

  // 纹理坐标
  for (GLuint i = 0; i < this->texcoords_layers; ++i) {
    std::string shader_texcoord_in = shader_texcoord_prefix_in + std::to_string(i);
    location = glGetAttribLocation(current_shader, shader_texcoord_in.c_str());
    if (location >= 0) {
      glEnableVertexAttribArray(location);
      glVertexAttribPointer(
        location, 2, GL_FLOAT, GL_FALSE, perVertexSize, (GLvoid *)(sizeof(glm::vec2) * i + offsetof(VertexInner, TexCoords)));
    }
  }

  /*-----EBO-------*/
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo->id);

  glBindVertexArray(GL_ZERO);
  glBindBuffer(GL_ARRAY_BUFFER, GL_ZERO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_ZERO);
}

void Mesh::draw(ShaderProgram::Ptr shader, Camera::Ptr camera) noexcept {
  shader->use();
  prepare_draw(shader);
  if (camera != nullptr) {
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
  }

  GLuint diffuseNr = 0;
  GLuint specularNr = 0;
  GLuint shadowNr = 0;
  GLuint alphaNr = 0;
  const std::string prefix = "textures.";

  for (GLint i = 0; i < textures.size(); ++i) {
    glActiveTexture(GL_TEXTURE0 + i);
    std::string number;
    std::string name;

    if (textures[i]->type == Texture::diffuse) {
      name = "diffuse";
      number = std::to_string(diffuseNr++);
    } else if (textures[i]->type == Texture::specular) {
      name = "specular";
      number = std::to_string(specularNr++);
    } else if (textures[i]->type == Texture::shadow) {
      name = "shadow";
      number = std::to_string(shadowNr++);
    } else if (textures[i]->type == Texture::alpha){
      name = "alpha";
      number = std::to_string(alphaNr++);
    }
    glBindTexture(GL_TEXTURE_2D, textures[i]->id);
    shader->set_uniform(prefix + name + number, i);
  }
  glActiveTexture(GL_TEXTURE0);

  // 绘制mesh
  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(GL_ZERO);
  glBindTexture(GL_TEXTURE_2D, GL_ZERO);
}

void Mesh::add_texture(Texture::Ptr texture) noexcept { textures.push_back(texture); }
