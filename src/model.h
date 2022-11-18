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
  ~Model();

  void load(const std::string &file_path, bool filpUV = true, bool genNormal = true) noexcept;
  void load(const std::string &file_path, uint32_t aiProcessFlags);
  void draw(std::shared_ptr<ShaderProgram> shader) noexcept;

public:
  glm::vec3 translate = glm::vec3(0, 0, 0);
  glm::vec3 rotate = glm::vec3(0, 0, 0);
  glm::vec3 scale = glm::vec3(1, 1, 1);

private:
  Mesh processMesh(const aiMesh *mesh, const aiScene *scene) noexcept;
  std::vector<Mesh> meshs;

private:
  std::vector<Texture> loadMaterialTextures(const aiScene *scene, const aiMaterial *material, const aiTextureType type);

private:
  std::unordered_map<std::string, Texture> texture_loaded;

private:
  std::string root_dir;  // 模型所处的文件夹
};


#endif  // !__MODEL_H__
