#include "camera.h"

#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 Camera::getViewMatrix() noexcept {
  if (mode & Mode::EulerAngle)  // 使用欧拉角更新相机朝向
  {
    direction.x = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    direction.y = sin(glm::radians(pitch));
    direction.z = -cos(glm::radians(pitch)) * cos(glm::radians(yaw));  // 相机看向z轴负方向
  }
  return glm::lookAt(position, position + direction, up);
}
glm::mat4 Camera::getProjectionMatrix() noexcept {
  if (mode & Mode::Perspective)  // 透视投影
  {
    return glm::perspective(glm::radians(fovy), aspect, zNear, zFar);
  }
  return glm::ortho(left, right, bottom, top, zNear, zFar);
}
