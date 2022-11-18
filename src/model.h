#ifndef __MODEL_H__
#define __MODEL_H__

#include <memory>
#include <string_view>

#include "mesh.h"
#include "shader.h"


class Model {
public:
  Model(){};
  Model(const std::string_view &file_path) { load(file_path); }
  ~Model();

  void load(const std::string_view &file_path);
  void draw(std::shared_ptr<ShaderProgram> shader);

private:
  std::vector<Mesh> meshs;
};


#endif  // !__MODEL_H__
