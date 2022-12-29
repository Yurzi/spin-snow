#version 330 core

in vec2 texcoordOut0;
//out vec4 fColor;
out vec4 f_color;
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
  if (texcolor.a == 0) {
    discard;
  }else {
    //f_depth = gl_FragCoord;
    //f_color = vec4(texcolor.aaaa);
    float a = texcolor.a;
    f_color = vec4(a,texcolor.gba);
  }
}
