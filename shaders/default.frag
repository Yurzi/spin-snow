#version 330 core
in vec2 texcoordOut0;
in vec2 texcoordOut1;

out vec4 fColor;

uniform sampler2D texture_diffuse0;

void main() {
  fColor= mix(texture(texture_diffuse0, texcoordOut0), texture(texture_diffuse0, texcoordOut1), 0.5);
}
