#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include <glm/glm.hpp>

struct Material {
  glm::vec3 ambient = {0, 0, 0};
  glm::vec3 diffuse = {0, 0, 0};
  glm::vec3 specular = {0, 0, 0};
};


#endif  // !__MATERIAL_H__
