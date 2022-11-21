#version 330 core

in vec2 f_texcoord0;

struct Texture {
// texture map
  sampler2D diffuse0;
  sampler2D specular0;
  sampler2D shadow0;
};

uniform Texture textures;

void main() {
}
