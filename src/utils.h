#ifndef __UTILS_H__
#define __UTILS_H__

#include <glad/glad.h>

#include <assimp/scene.h>

#include <string>
#include <vector>

// 从给定的绝对路径中导入材质
GLuint Texture2DFromFile(const std::string &file_path,
                         GLenum wrapMode = GL_REPEAT,
                         GLenum magFilterMode = GL_LINEAR,
                         GLenum minFilterMode = GL_LINEAR_MIPMAP_LINEAR) noexcept;

GLuint Texture2DFromAssimp(const aiTexture *aitexture,
                           GLenum wrapMode = GL_REPEAT,
                           GLenum magFilterMode = GL_LINEAR,
                           GLenum minFilterMode = GL_LINEAR_MIPMAP_LINEAR) noexcept;

GLuint Texture2DFromUChar(const unsigned char data[],
                          GLuint width = 1,
                          GLuint height = 1,
                          GLenum formatMode = GL_RGBA,
                          GLenum wrapMode = GL_REPEAT,
                          GLenum magFilterMode = GL_LINEAR,
                          GLenum minFilterMode = GL_LINEAR_MIPMAP_LINEAR) noexcept;

GLuint Texture2DForShadowMap(GLuint width,
                             GLuint height,
                             GLenum wrapMode = GL_CLAMP_TO_EDGE,
                             GLenum magFilterMode = GL_NEAREST,
                             GLenum minFilterMode = GL_NEAREST,
                             GLfloat *borderColor = nullptr) noexcept;

GLuint CubeMapFromFile(const std::vector<std::string> &file_paths,
                       GLenum wrapMode = GL_CLAMP_TO_EDGE,
                       GLenum magFilterMode = GL_LINEAR,
                       GLenum minFilterMode = GL_LINEAR_MIPMAP_LINEAR) noexcept;
#endif  // !__UTILS_H__
