#ifndef __LIGHT_H__
#define __LIGHT_H__

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <memory>

struct Light {
  typedef std::shared_ptr<Light> Ptr;

  enum Type { SunLight = 0x0, PointLight = 0x1, SpotLight = 0x2, FlashLight = 0x4 };
  Type type = Type::PointLight;
  glm::vec3 position = {0, 0, 0};
  glm::vec3 direction = {0, 0, 0};

  float inner_cutoff = 1;  // 余弦值 [1,0]
  float outer_cutoff = 1;  // 余弦值 [1,0]

  glm::vec3 ambient = {0.1, 0.1, 0.1};
  glm::vec3 diffuse = {1, 1, 1};
  glm::vec3 specular = {1, 1, 1};
};


#endif  // !__LIGHT_H__
