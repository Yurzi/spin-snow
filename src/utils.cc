#include "utils.h"

#include <cstring>
#include <iostream>

#include <stdint.h>

#include <stb_image.h>

GLuint Texture2DFromFile(const std::string &file_path, GLenum wrapMode, GLenum magFilterMode, GLenum minFilterMode) noexcept {
  GLuint texture_id = GL_ZERO;

  int32_t width, height, nrChannels;
  //  stbi_set_flip_vertically_on_load(true);
  unsigned char *data = stbi_load(file_path.c_str(), &width, &height, &nrChannels, 0);

  if (data != nullptr) {
    GLenum format;
    switch (nrChannels) {
    case 1:
      format = GL_RED;
      break;
    case 3:
      format = GL_RGB;
      break;
    case 4:
      format = GL_RGBA;
      break;
    default:
      format = GL_RGBA;
    }

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilterMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilterMode);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, GL_ZERO);

  } else {
    std::cout << "[ERROR::Utils::Texture2DFromFile] Failed to load texture at path: " << file_path << std::endl;
    std::cout << "not a absolate path or file not exists" << std::endl;
  }

  return texture_id;
}


GLuint Texture2DFromAssimp(const aiTexture *ai_texture, GLenum wrapMode, GLenum magFilterMode, GLenum minFilterMode) noexcept {
  if (ai_texture == nullptr)
    return GL_ZERO;

  int32_t width, height, nrChannels;
  unsigned char *data = nullptr;
  if (ai_texture->mHeight == 0) {
    data = stbi_load_from_memory(
      reinterpret_cast<unsigned char *>(ai_texture->pcData), ai_texture->mWidth, &width, &height, &nrChannels, 0);
  } else {
    data = stbi_load_from_memory(reinterpret_cast<unsigned char *>(ai_texture->pcData),
                                 ai_texture->mWidth * ai_texture->mHeight,
                                 &width,
                                 &height,
                                 &nrChannels,
                                 0);
  }

  GLuint texture_id = GL_ZERO;
  if (data != nullptr) {
    GLenum format;
    switch (nrChannels) {
    case 1:
      format = GL_RED;
      break;
    case 3:
      format = GL_RGB;
      break;
    case 4:
      format = GL_RGBA;
      break;
    default:
      format = GL_RGBA;
    }
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilterMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilterMode);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, GL_ZERO);
  } else {
    std::cout << "[ERROR::Utils::Texture2DFromAssimp] Failed to load texture from memroy " << std::endl;
  }

  return texture_id;
}

GLuint Texture2DFromUChar(const unsigned char data[],
                          GLuint width,
                          GLuint height,
                          GLenum formatMode,
                          GLenum wrapMode,
                          GLenum magFilterMode,
                          GLenum minFilterMode) noexcept {
  unsigned char *image = nullptr;
  GLuint unit_size = 4;
  GLuint length = 0;
  if (data == nullptr) {
    formatMode = GL_RGBA;
    length = width * height * unit_size;
    image = (unsigned char *)malloc(sizeof(unsigned char) * length);
    memset(image, 0, length);
  } else {
    switch (formatMode) {
    case GL_RED:
      unit_size = 1;
      break;
    case GL_RGB:
      unit_size = 3;
      break;
    case GL_RGBA:
      unit_size = 4;
      break;
    default:
      unit_size = 4;
    }
    length = width * height * unit_size;
    image = (unsigned char *)malloc(sizeof(unsigned char) * length);

#ifdef _WIN32_
    memcpy_s(image, length, data, length);
#else
    memcpy(image, data, length);
#endif
  }

  GLuint texture_id = GL_ZERO;
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilterMode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilterMode);

  glTexImage2D(GL_TEXTURE_2D, 0, formatMode, width, height, 0, formatMode, GL_UNSIGNED_BYTE, image);
  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, GL_ZERO);

  free(image);
  return texture_id;
}


GLuint Texture2DForShadowMap(
  GLuint width, GLuint height, GLenum wrapMode, GLenum magFilterMode, GLenum minFilterMode, GLfloat *borderColor) noexcept {
  GLuint texture_id = GL_ZERO;
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

  if (wrapMode == GL_CLAMP_TO_BORDER) {
    GLfloat default_border_color[] = {1.0, 1.0, 1.0, 1.0};
    if (borderColor == nullptr) {
      borderColor = default_border_color;
    }
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilterMode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilterMode);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

  glBindTexture(GL_TEXTURE_2D, GL_ZERO);
  return texture_id;
}


GLuint CubeMapFromFile(const std::vector<std::string> &file_path,
                       GLenum wrapMode,
                       GLenum magFilterMode,
                       GLenum minFilterMode) noexcept {
  GLuint texture_id = GL_ZERO;

  int32_t width, height, nrChannels;
  //  stbi_set_flip_vertically_on_load(true);
  unsigned char *data[6] = {nullptr};
  for (int32_t i = 0; i < file_path.size(); ++i) {
    data[i] = stbi_load(file_path[i].c_str(), &width, &height, &nrChannels, 0);
  }

  if (data[0] != nullptr) {
    GLenum format;
    switch (nrChannels) {
    case 1:
      format = GL_RED;
      break;
    case 3:
      format = GL_RGB;
      break;
    case 4:
      format = GL_RGBA;
      break;
    default:
      format = GL_RGBA;
    }

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrapMode);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrapMode);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minFilterMode);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, magFilterMode);

    for (int32_t i = 0; i < file_path.size(); ++i) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data[i]);
      if (data[i]) {
        stbi_image_free(data[i]);
      }
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glBindTexture(GL_TEXTURE_CUBE_MAP, GL_ZERO);

  } else {
    std::cout << "[ERROR::Utils::CubeMapFromFile] Failed to load texture" << std::endl;
    std::cout << "not a absolate path or file not exists" << std::endl;
  }

  return texture_id;
}
