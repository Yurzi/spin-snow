#version 330 core

in vec2 f_texcoord0;

out vec4 f_color;

struct Texture {
// texture map
  sampler2D diffuse0;
  sampler2D specular0;
  sampler2D shadow0;
};

uniform Texture textures;

void main() {
  f_color = vec4(vec3(texture(textures.shadow0, f_texcoord0).r *0.5 + 0.5), 1);
}
