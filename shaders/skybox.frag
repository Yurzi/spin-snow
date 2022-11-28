#version 330 core

in vec3 f_texcoord;
out vec4 f_color;

uniform samplerCube skybox;

void main() {
  f_color = texture(skybox, f_texcoord);
}
