#version 330 core
in vec2 texcoordOut0;

out vec4 fColor;

uniform sampler2D texture_diffuse0;

void main() {
  fColor = texture(texture_diffuse0, texcoordOut0);
}
