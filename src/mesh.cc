#include "mesh.h"
#include <iostream>
#include <string>

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
Mesh::~Mesh() {
  // 释放缓冲对象
  if (VBO != GL_ZERO) {
    glDeleteBuffers(1, &VBO);
  }
  if (EBO != GL_ZERO) {
    glDeleteBuffers(1, &EBO);
  }
  if (VAO != GL_ZERO) {
    glDeleteBuffers(1, &VAO);
  }
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

  glGenVertexArrays(1, &VAO);
  // 对于VBO指针的解析需要着色器对象，需要在draw call时进行
  // 同时要进行记忆, 所以将其和VAO的绑定将移动至draw call前进行
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  glBindVertexArray(VAO);
  /*--------------------EBO----------------------------*/
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

  /*--------------------VBO----------------------------*/
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  // 申请缓冲空间
  GLuint perVertexSize = sizeof(Vertex::Position) + sizeof(Vertex::Normal) + sizeof(glm::vec2) * this->texcoords_layers;
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
  // 4. 设置绑定
  // 顶点
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, perVertexSize, (void *)0);
  // 法线
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, perVertexSize, (void *)offsetof(VertexInner, Normal));
  // 纹理
  GLuint texcoord_start = 8;
  for (int i = 0; i < this->texcoords_layers; ++i) {
    GLuint texcoord_location = texcoord_start + i;
    glEnableVertexAttribArray(texcoord_location);
    glVertexAttribPointer(texcoord_location,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          perVertexSize,
                          (void *)(sizeof(glm::vec2) * i + offsetof(VertexInner, TexCoords)));
  }
  // 5.finish
  free(inner_data);

  glBindVertexArray(GL_ZERO);
  glBindBuffer(GL_ARRAY_BUFFER, GL_ZERO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_ZERO);
}


void Mesh::draw(std::shared_ptr<ShaderProgram> shader) noexcept {
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
    shader->set_unifom(prefix + name + number, i);
  }
  glActiveTexture(GL_TEXTURE0);

  // 绘制mesh
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(GL_ZERO);
}
