#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 8) in vec2 texcoord0;

out vec2 texcoordOut0;
out vec3 worldPos;
out vec3 normalOut;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 TinverseModel;

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);
    texcoordOut0 = vec2(texcoord0.x, texcoord0.y);
    worldPos = (model * vec4(position, 1.0)).xyz;
    normalOut = (TinverseModel * vec4(normal, 0.0)).xyz;
}
