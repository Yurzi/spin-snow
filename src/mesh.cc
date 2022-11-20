#include "mesh.h"
#include <iostream>
#include <string>
#include <glm/gtc/matrix_transform.hpp>

struct VertexInner {
  glm::vec3 Position;      // 位置向量
  glm::vec3 Normal;        // 法线
  glm::vec2 TexCoords[0];  // 纹理坐标 assimp 允许一个顶点有多个纹理坐标
};

Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<GLuint> &indices, const std::vector<Texture> &textures) {
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
  this->has_setup = false;

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

  this->has_setup = true;
  this->shader_vao_map = std::move(oth.shader_vao_map);
  oth.shader_vao_map.clear();

  this->VAO = oth.VAO;
  oth.VAO = GL_ZERO;
  this->VBO = oth.VBO;
  oth.VBO = GL_ZERO;
  this->EBO = oth.EBO;
  oth.EBO = GL_ZERO;

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
  this->has_setup = false;

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

  this->has_setup = true;
  this->shader_vao_map = std::move(oth.shader_vao_map);
  oth.shader_vao_map.clear();

  this->VAO = oth.VAO;
  oth.VAO = GL_ZERO;
  this->VBO = oth.VBO;
  oth.VBO = GL_ZERO;
  this->EBO = oth.EBO;
  oth.EBO = GL_ZERO;

  setup();
  return (*this);
}
Mesh::~Mesh() {
  // 释放缓冲对象
  if (VBO != GL_ZERO) {
    glDeleteBuffers(1, &VBO);
  }
  if (EBO != GL_ZERO) {
    glDeleteBuffers(1, &EBO);
  }
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
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  /*--------------------EBO----------------------------*/
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

  /*--------------------VBO----------------------------*/
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
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
    memcpy_s(ptr->TexCoords,
             sizeof(glm::vec2) * this->texcoords_layers,
             this->vertices[i].TexCoords.data(),
             sizeof(glm::vec2) * this->texcoords_layers);
  }

  // 3.从内存空间将数据发送到GPU buffer
  glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * perVertexSize, inner_data, GL_STATIC_DRAW);
  // 5.finish
  free(inner_data);

  glBindBuffer(GL_ARRAY_BUFFER, GL_ZERO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_ZERO);
  this->has_setup = true;
}

void Mesh::prepare_draw(std::shared_ptr<ShaderProgram> shader) noexcept {
  // O(1) check
  if (shader->get_id() == this->current_shader) {
    return;
  }
  // 切换为指定的shader
  this->current_shader = shader->get_id();
  // 查找是否已经生成VAO
  if (shader_vao_map.find(current_shader) != shader_vao_map.end()) {
    this->VAO = shader_vao_map.find(current_shader)->second;
    return;
  }

  // 如果未找到对应的VAO，则需要生成
  glGenVertexArrays(1, &(this->VAO));
  shader_vao_map.insert({current_shader, this->VAO});
  // 进行VBO 和 EBO的绑定
  glBindVertexArray(this->VAO);

  /*-----VBO-------*/
  glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
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
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);

  glBindVertexArray(GL_ZERO);
  glBindBuffer(GL_ARRAY_BUFFER, GL_ZERO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_ZERO);
}

void Mesh::draw(std::shared_ptr<ShaderProgram> shader, std::shared_ptr<Camera> camera) noexcept {
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
  const std::string prefix = "material.";

  for (GLint i = 0; i < textures.size(); ++i) {
    glActiveTexture(GL_TEXTURE0 + i);
    std::string number;
    std::string name;

    if (textures[i].type == Texture::diffuse) {
      name = "texture_diffuse";
      number = std::to_string(diffuseNr++);
    } else if (textures[i].type == Texture::specular) {
      name = "texture_specular";
      number = std::to_string(specularNr++);
    }
    glBindTexture(GL_TEXTURE_2D, textures[i].id);
    shader->set_uniform(prefix + name + number, i);
  }
  glActiveTexture(GL_TEXTURE0);

  // 绘制mesh
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(GL_ZERO);
  glBindTexture(GL_TEXTURE_2D, GL_ZERO);
}
