#version 330 core

in vec2 texcoordOut0;
out vec4 fColor;
//out vec4 f_depth;

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
  if (texcolor.a != 1) {
    discard;
  }else {
    //f_depth = gl_FragCoord;
    fColor = vec4(vec3(texcolor.aaa), 1.0);
  }
  

}
