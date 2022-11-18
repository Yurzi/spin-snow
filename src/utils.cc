#include "utils.h"

#include <iostream>

#include <stdint.h>

#include <stb_image.h>

GLuint Texture2DFromFile(const std::string &file_path, GLenum wrapMode, GLenum magFilterMode, GLenum minFilterMode) noexcept {
  GLuint texture_id;

  int32_t width, height, nrChannels;
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
      format = GL_RGB;
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


GLuint Texture2DFromAssimp(const aiTexture *aitexture, GLenum wrapMode, GLenum magFilterMode, GLenum minFilterMode) noexcept {
  if (aitexture == nullptr)
    return GL_ZERO;

  int32_t width, height, nrChannels;
  unsigned char *data = nullptr;
  if (aitexture->mHeight == 0) {
    data = stbi_load_from_memory(
      reinterpret_cast<unsigned char *>(aitexture->pcData), aitexture->mWidth, &width, &height, &nrChannels, 0);
  } else {
    data = stbi_load_from_memory(reinterpret_cast<unsigned char *>(aitexture->pcData),
                                 aitexture->mWidth * aitexture->mHeight,
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
      format = GL_RGB;
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
