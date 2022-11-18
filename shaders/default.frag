#version 330 core
in vec2 texcoordOut0;
in vec3 worldPos;
in vec3 normalOut;

out vec4 fColor;

uniform sampler2D texture_diffuse0;
uniform vec3 lightPos;
uniform vec3 cameraPos;

float phong(vec3 worldPos, vec3 cameraPos, vec3 lightPos, vec3 normal) {
  vec3 N = normalize(normal);
  vec3 V = normalize(worldPos - cameraPos);
  vec3 L = normalize(worldPos - lightPos);
  vec3 R = reflect(L, N);

  float ambient = 0.3;
  float diffuse = max(dot(N, -L), 0) * 0.7;
  float specular = pow(max(dot(-R, V), 0.1), 50.0) * 0.4;

  return ambient + diffuse + specular;
}

void main() {
  fColor = texture(texture_diffuse0, texcoordOut0);
  fColor.rgb *= phong(worldPos, cameraPos, lightPos, normalOut);
}
