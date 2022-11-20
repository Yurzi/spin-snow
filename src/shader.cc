#include "shader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <memory>
#include <ostream>
#include <stdint.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

Shader::Shader(const std::string_view &src_path) : m_id(GL_ZERO) {
  // read the source into 'm_src'
  std::ifstream fd;
  std::stringstream ss;
  try {
    fd.open(std::string(src_path), std::ios::in);
    if (!fd.is_open()) {
      throw std::ifstream::failure("file not exists");
    }
    ss << fd.rdbuf();
    fd.close();
  } catch (std::ifstream::failure e) {
    std::cout << "[ERROR::Shader] Source File Not Successfully Read: " << e.what() << std::endl;
  }
  m_src = ss.str();
}

Shader::~Shader() {
  if (m_id != 0) {
    glDeleteShader(m_id);
  }
}

int32_t Shader::check_compile_status(const Shader *shader, GLenum shader_type) {
  int32_t sucess;
  char log[512];
  if (!shader) {
    std::cout << "[WARN::Shader] check compile status faild, no vaild shader" << std::endl;
    return 0x2;
  }
  GLuint shader_id = shader->get_id();
  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &sucess);
  if (sucess == GL_TRUE) {
    return 0x0;
  }
  // cout error info
  std::string shader_type_str;
  switch (shader_type) {
  case GL_VERTEX_SHADER:
    shader_type_str = "Vertex Shader";
    break;
  case GL_FRAGMENT_SHADER:
    shader_type_str = "Fragment Shader";
    break;
  default:
    shader_type_str = "Unknow Type Shader";
    break;
  }
  std::cout << "[ERROR::Shader] " << shader_type_str << " Compiled Failed:\n" << log << std::endl;
  return 0x1;
}


VertexShader::VertexShader(const std::string_view &src_path) : Shader(src_path) {
  this->m_id = glCreateShader(GL_VERTEX_SHADER);
  auto src_cstr = this->m_src.c_str();
  glShaderSource(this->m_id, 1, &src_cstr, nullptr);
  glCompileShader(this->m_id);
  int32_t rt = check_compile_status(this, GL_VERTEX_SHADER);
  if (rt != 0x0) {
    m_status = false;
  } else {
    m_status = true;
  }
}

FragmentShader::FragmentShader(const std::string_view &src_path) : Shader(src_path) {
  this->m_id = glCreateShader(GL_FRAGMENT_SHADER);
  auto src_cstr = this->m_src.c_str();
  glShaderSource(this->m_id, 1, &src_cstr, nullptr);
  glCompileShader(this->m_id);
  int32_t rt = check_compile_status(this, GL_FRAGMENT_SHADER);
  if (rt != 0x0) {
    m_status = false;
  } else {
    m_status = true;
  }
}

ShaderProgram::ShaderProgram(const std::string_view &vertex_shader_filename,
                             const std::string_view &fragement_shader_filename) {
  std::vector<std::shared_ptr<Shader>> shaders;
  std::shared_ptr<Shader> vertex_shader = std::make_shared<VertexShader>(vertex_shader_filename);
  std::shared_ptr<Shader> fragement_shader = std::make_shared<FragmentShader>(fragement_shader_filename);
  if (vertex_shader->get_status()) {
    shaders.push_back(vertex_shader);
  }

  if (fragement_shader->get_status()) {
    shaders.push_back(fragement_shader);
  }
  this->initliazer(shaders);
}

ShaderProgram::ShaderProgram(const std::vector<std::shared_ptr<Shader>> &shaders) { this->initliazer(shaders); }

void ShaderProgram::initliazer(const std::vector<std::shared_ptr<Shader>> &shaders) noexcept {
  this->m_id = glCreateProgram();
  for (auto idx : shaders) {
    glAttachShader(this->m_id, idx->get_id());
  }
  glLinkProgram(this->m_id);
  int success{};
  char log_info[512];
  glGetProgramiv(this->m_id, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(this->m_id, 512, nullptr, log_info);
    std::cout << "[ERROR::ShaderProgram] Program Link Failed\n" << log_info << std::endl;
  }
}
ShaderProgram::~ShaderProgram() {
  if (this->m_id != GL_ZERO) {
    glDeleteProgram(this->m_id);
  }
}

void ShaderProgram::set_unifom(const std::string_view &name, bool value) const noexcept {
  glUniform1i(glGetUniformLocation(this->m_id, name.data()), static_cast<int>(value));
}
void ShaderProgram::set_unifom(const std::string_view &name, GLint value) const noexcept {
  glUniform1i(glGetUniformLocation(this->m_id, name.data()), value);
}
void ShaderProgram::set_unifom(const std::string_view &name, GLfloat value) const noexcept {
  glUniform1f(glGetUniformLocation(this->m_id, name.data()), value);
}
void ShaderProgram::set_unifom(const std::string_view &name, const glm::vec3 &value) const noexcept {
  glUniform3fv(glGetUniformLocation(this->m_id, name.data()), 1, glm::value_ptr(value));
}
void ShaderProgram::set_unifom(const std::string_view &name, const glm::vec4 &value) const noexcept {
  glUniform4fv(glGetUniformLocation(this->m_id, name.data()), 1, glm::value_ptr(value));
}
void ShaderProgram::set_unifom(const std::string_view &name, const glm::mat4 &value) const noexcept {
  glUniformMatrix4fv(glGetUniformLocation(this->m_id, name.data()), 1, GL_FALSE, glm::value_ptr(value));
}

void ShaderProgram::set_light(const std::string_view &name, const Light& value) const noexcept {
  std::string prefix = name.data();
  prefix += '.';
  set_unifom(prefix + "type", value.type);
  set_unifom(prefix + "position", value.position);
  set_unifom(prefix + "direction", value.direction);
  set_unifom(prefix + "inner_cutoff", value.inner_cutoff);
  set_unifom(prefix + "outer_cutoff", value.outer_cutoff);
  set_unifom(prefix + "ambient", value.ambient);
  set_unifom(prefix + "diffuse", value.diffuse);
  set_unifom(prefix + "specular", value.specular);
}
void ShaderProgram::set_material(const std::string_view &name, const Material& value) const noexcept {
  std::string prefix = name.data();
  prefix += '.';
  set_unifom(prefix + "ambient", value.ambient);
  set_unifom(prefix + "diffuse", value.diffuse);
  set_unifom(prefix + "specular", value.specular);
}

void ShaderProgram::use() const noexcept { glUseProgram(this->m_id); }
