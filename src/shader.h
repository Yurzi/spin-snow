#ifndef __SHADER_H__
#define __SHADER_H__

#include <glad/glad.h>

#include <stdint.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

class Shader {
public:
  explicit Shader(const std::string_view &src_path);
  ~Shader();

  constexpr GLuint get_id() const noexcept { return this->m_id; }
  constexpr bool get_status() const noexcept { return this->m_status; }

public:
  static int32_t check_compile_status(const Shader *shader, GLenum shader_type);

protected:
  GLuint m_id = 0;
  std::string m_src;
  bool m_status = false;
};

class VertexShader : public Shader {
public:
  explicit VertexShader(const std::string_view &src_path);
};

class FragmentShader : public Shader {
public:
  explicit FragmentShader(const std::string_view &src_path);
};

class ShaderProgram {
public:
  explicit ShaderProgram(const std::string_view &vertex_shader_filename, const std::string_view &fragement_shader_filename);
  explicit ShaderProgram(const std::vector<std::shared_ptr<Shader>> &shaders);

  ~ShaderProgram();

public:
  void use() const noexcept;
  constexpr GLuint get_id() const noexcept { return this->m_id; }

  void set_unifom(const std::string_view &name, bool value) const noexcept;
  void set_unifom(const std::string_view &name, GLint value) const noexcept;
  void set_unifom(const std::string_view &name, GLfloat value) const noexcept;

private:
  void initliazer(const std::vector<std::shared_ptr<Shader>> &shaders) noexcept;

private:
  GLuint m_id;
};


#endif  // !__SHADER_H__
