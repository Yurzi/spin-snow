#version 330 core
in vec3 position;
in vec3 normal;
in vec2 texcoord0;
in vec2 texcoord1;

out vec2 texcoordOut0;
out vec2 texcoordOut1;

out vec3 vColorOut;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
   gl_Position = projection * view * model * vec4(position, 1.0);
   texcoordOut0 = texcoord0;
   texcoordOut1 = texcoord1;
}
