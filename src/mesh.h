#ifndef __MESH_H__
#define __MESH_H__

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <unordered_map>
#include <vector>

#include "camera.h"
#include "shader.h"


const static std::string shader_postion_in = "position";
const static std::string shader_normal_in = "normal";
const static std::string shader_texcoord_prefix_in = "texcoord";

/** 顶点
 * 为了解决一个顶点多个纹理坐标的问题，现在约定:
 * position 放置顶点坐标
 * normal 放置法线
 * texcoordN 纹理坐标
 */
struct Vertex {
  glm::vec3 Position = glm::vec3(0, 0, 0);  // 位置向量
  glm::vec3 Normal = glm::vec3(0, 0, 0);    // 法线
  std::vector<glm::vec2> TexCoords;         // 纹理坐标 assimp 允许一个顶点有多个纹理坐标
  Vertex() = default;
  Vertex(glm::vec3 Position, glm::vec3 Normal = {0, 0, 0}, glm::vec2 TexCoord = {0, 0}) {
    this->Position = Position;
    this->Normal = Normal;
    this->TexCoords.push_back(TexCoord);
  }
};

/** 材质
 * 对于着色器中材质变量的命名方式为
 * 漫反射纹理：textures.diffuseN; N >= 0;
 * 镜面反射纹理: textures.specularN; N >=0;
 * 阴影纹理：textures.shadowN; N>=0;
 */
struct Texture {
  enum Type { diffuse = 0x0, specular = 0x1, shadow = 0x10 };
  GLuint id;                                              // 材质id
  Type type;                                              // 材质类型
  std::string path = "<**{YURZI::BUILT-IN::TEXTURE}**>";  // 纹理文件的位置
};


class Mesh {
public:
  // 方法
  Mesh(){};
  Mesh(const std::vector<Vertex> &vertices, const std::vector<GLuint> &indices, const std::vector<Texture> &textures);
  Mesh(const Mesh &oth);
  Mesh(Mesh &&oth);
  Mesh &operator=(const Mesh &oth) noexcept;
  Mesh &operator=(Mesh &&oth) noexcept;
  ~Mesh();

  void setup() noexcept;
  void draw(std::shared_ptr<ShaderProgram> shader, std::shared_ptr<Camera> camera = nullptr) noexcept;

  void add_texture(const Texture &texture) noexcept;

public:
  // 基础数据
  std::vector<Vertex> vertices;   // 顶点
  std::vector<GLuint> indices;    // 索引
  std::vector<Texture> textures;  // 材质
public:
  glm::vec3 translate = glm::vec3(0, 0, 0);
  glm::vec3 rotate = glm::vec3(0, 0, 0);  // 角度制
  glm::vec3 scale = glm::vec3(1, 1, 1);

private:
  void prepare_draw(std::shared_ptr<ShaderProgram> shader) noexcept;

private:
  // 一些网格参数
  GLuint texcoords_layers = 0;  // 网格中顶点对应的纹理坐标的层数：
  bool has_setup = false;

  std::unordered_map<GLuint, GLuint> shader_vao_map;
  GLuint current_shader = GL_ZERO;

private:
  // 渲染数据
  GLuint VAO = GL_ZERO;  // 现在使用的VAO
  GLuint VBO = GL_ZERO;
  GLuint EBO = GL_ZERO;
};
#endif  // !__MESH_H__
