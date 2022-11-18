#ifndef __UTILS_H__
#define __UTILS_H__

#include <glad/glad.h>

#include <string>

// 从给定的绝对路径中导入材质
GLuint Texture2DFromFile(const std::string &file_path) noexcept;

#endif  // !__UTILS_H__
