#ifndef __UTILS_H__
#define __UTILS_H__

#include <glad/glad.h>

#include <assimp/scene.h>

#include <string>

// 从给定的绝对路径中导入材质
GLuint Texture2DFromFile(const std::string &file_path,
                         GLenum wrapMode = GL_REPEAT,
                         GLenum magFilterMode = GL_LINEAR,
                         GLenum minFilterMode = GL_LINEAR_MIPMAP_LINEAR) noexcept;

GLuint Texture2DFromAssimp(const aiTexture *aitexture,
                           GLenum wrapMode = GL_REPEAT,
                           GLenum magFilterMode = GL_LINEAR,
                           GLenum minFilterMode = GL_LINEAR_MIPMAP_LINEAR) noexcept;
#endif  // !__UTILS_H__
