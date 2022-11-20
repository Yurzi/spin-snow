#version 330 core
in vec2 texcoordOut0;
in vec3 worldPos;
in vec3 normalOut;

out vec4 fColor;

uniform sampler2D texture_diffuse0;
uniform vec3 lightPos;
uniform vec3 cameraPos;

uniform vec3 objectColor;
uniform vec3 lightColor;

void main() {
  float ambientStrength = 0.1;
  vec3 ambient = ambientStrength * lightColor;

  vec3 lightDir = normalize(lightPos - worldPos);
  float diff = max(dot(normalOut, lightDir), 0.0);
  vec3 diffuse = diff * lightColor;

  float specularStrength = 0.5;
  vec3 viewDir =normalize(cameraPos - worldPos);
  vec3 reflectDir = reflect(-lightDir, normalOut);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  vec3 specular = specularStrength * spec * lightColor;

  vec3 result = (ambient + diffuse + specular) * objectColor;
  fColor = vec4(result, 1.0);
}
