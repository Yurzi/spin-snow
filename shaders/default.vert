#version 330 core
in vec3 position;
in vec2 texcoord0;
in vec3 normal;

out vec2 texcoordOut0;
out vec3 worldPos;
out vec3 normalOut;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 NormalMatrix;


void main() {
  gl_Position = projection * view * model * vec4(position, 1.0);
  texcoordOut0 = texcoord0;
  worldPos = (model * vec4(position, 1.0)).xyz;
  normalOut = normalize(NormalMatrix * vec4(normal, 0.0f)).xyz;
}
