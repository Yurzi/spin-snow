#ifndef __MESH_H__
#define __MESH_H__

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <unordered_map>
#include <vector>


#include "shader.h"

/** 顶点
 * 为了解决一个顶点多个纹理坐标的问题，现在约定:
 * location 0 : position 放置顶点坐标
 * location 1 : normal 放置法线
 * location 8~15 : texcoord 纹理坐标
 */
struct Vertex {
  glm::vec3 Position;                // 位置向量
  glm::vec3 Normal;                  // 法线
  std::vector<glm::vec2> TexCoords;  // 纹理坐标 assimp 允许一个顶点有多个纹理坐标
};

/** 材质
 * 对于着色器中材质变量的命名方式为
 * 漫反射纹理：material.texture_diffuseN; N >= 0;
 * 镜面反射纹理: material.texture_specularN; N >=0;
 */
struct Texture {
  enum Type {
    diffuse = 0,
    specular = 1,
  };
  GLuint id;         // 材质id
  Type type;         // 材质类型
  std::string path;  // 纹理文件的位置
};


class Mesh {
public:
  // 方法
  Mesh(){};
  Mesh(const std::vector<Vertex> &vertices, const std::vector<GLuint> &indices, const std::vector<Texture> &textures);
  ~Mesh();

  void setup() noexcept;
  void draw(std::shared_ptr<ShaderProgram> shader) noexcept;

public:
  // 基础数据
  std::vector<Vertex> vertices;   // 顶点
  std::vector<GLuint> indices;    // 索引
  std::vector<Texture> textures;  // 材质
private:
  // 一些网格参数
  GLuint texcoords_layers = 0;                        // 网格中顶点对应的纹理坐标的层数：

  bool has_setup = false;

private:
  // 渲染数据
  GLuint VAO = GL_ZERO;  // 现在使用的VAO
  GLuint VBO = GL_ZERO;
  GLuint EBO = GL_ZERO;
};
#endif  // !__MESH_H__
