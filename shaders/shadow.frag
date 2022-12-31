#version 330 core

in vec2 texcoordOut0;
out vec4 f_color;

struct Texture {
// texture map
  sampler2D diffuse0;
  sampler2D specular0;
  sampler2D shadow0;
  sampler2D alpha0;
};

uniform Texture textures;

void main() {
  vec4 texcolor = texture(textures.diffuse0, texcoordOut0);
  if (texcolor.a == 0) {
    discard;
  }else {
    f_color = texcolor.agba;
  }

}
