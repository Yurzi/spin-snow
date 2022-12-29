#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <glm/glm.hpp>
#include <stdint.h>

#include <memory>

class Camera {
public:
  typedef std::shared_ptr<Camera> Ptr;

  enum Mode {
    DefaultAngle = 0x0,
    EulerAngle = 0x1,

    Perspective = 0x10,
    Ortho = 0x20
  };

public:
  Camera() = default;
  glm::mat4 getViewMatrix() noexcept;
  glm::mat4 getProjectionMatrix() noexcept;

public:
  // 相机参数
  glm::vec3 position = glm::vec3(0, 0, 0);                     // 位置
  glm::vec3 direction = glm::vec3(0, 0, -1);                   // 视线方向
  glm::vec3 up = glm::vec3(0, 1, 0);                           // 上向量，固定(0,1,0)不变
  float pitch = 0.0f, roll = 0.0f, yaw = 0.0f;                 // 欧拉角
  float fovy = 70.0f, aspect = 1.0, zNear = 0.01, zFar = 100;  // 透视投影参数
  float left = -1.0, right = 1.0, top = 1.0, bottom = -1.0;    // 正交投影参数
public:
  uint64_t mode = EulerAngle | Perspective;
};


#endif  // !__CAMERA_H__
