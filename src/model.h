#ifndef __MODEL_H__
#define __MODEL_H__

#include <memory>
#include <string_view>

#include <stdint.h>

#include <assimp/scene.h>
#include <unordered_map>

#include "mesh.h"
#include "shader.h"


class Model {
public:
  Model(){};
  Model(const std::string &file_path) { load(file_path); }
  Model(const Model &oth);
  Model(Model &&oth);
  Model &operator=(const Model &oth) noexcept;
  Model &operator=(Model &&oth) noexcept;
  ~Model();

  void load(const std::string &file_path, bool filpUV = true, bool genNormal = true) noexcept;
  void load(const std::string &file_path, uint32_t aiProcessFlags);
  void add_mesh(const Mesh &mesh) noexcept { meshs.push_back(mesh); }
  void draw(std::shared_ptr<ShaderProgram> shader) noexcept;

public:
  glm::vec3 translate = glm::vec3(0, 0, 0);
  glm::vec3 rotate = glm::vec3(0, 0, 0);
  glm::vec3 scale = glm::vec3(1, 1, 1);

private:
  Mesh processMesh(const aiMesh *mesh, const aiScene *scene) noexcept;
  std::vector<Texture> loadMaterialTextures(const aiScene *scene, const aiMaterial *material, const aiTextureType type);

private:
  std::vector<Mesh> meshs;
  std::unordered_map<std::string, Texture> texture_loaded;
  bool has_loaded = false;

private:
  std::string root_dir;     // 模型所处的文件夹
  std::string model_path;   // 模型描述文件所在的路径
  uint32_t aiProcessFlags;  // 保留的aiProcessFlags 用于拷贝构造
};


#endif  // !__MODEL_H__
