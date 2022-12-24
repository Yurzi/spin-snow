#version 330 core

in vec2 f_texcoord0;
out vec4 f_depth;

struct Texture {
// texture map
  sampler2D diffuse0;
  sampler2D specular0;
  sampler2D shadow0;
};

uniform Texture textures;

void main() {
  
  if (texture(textures.diffuse0, f_texcoord0).a == 0) {
    discard;
  }else {
    f_depth = gl_FragCoord;
  }
  

}
