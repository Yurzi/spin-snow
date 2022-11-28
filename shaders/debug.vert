#version 330 core

in vec3 position;
in vec2 texcoord0;

out vec2 f_texcoord0;

void main() {
  gl_Position = vec4(position, 1.0f);
  f_texcoord0 = texcoord0;
}
