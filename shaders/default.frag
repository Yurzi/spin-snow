#version 330 core
in vec2 texcoordOut0;
in vec3 worldPos;
in vec3 normalOut;

out vec4 fColor;

uniform vec3 cameraPos;

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
// texture map
  sampler2D texture_diffuse0;
  sampler2D texture_specular0;
};

struct MaterialInner {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

struct Light {
  int type;
  vec3 position;
  vec3 direction;
  
  float inner_cutoff;
  float outer_cutoff;
  
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

uniform Material material;
uniform Light light;

vec3 blinn_phong(vec3 worldPos, vec3 cameraPos, vec3 normal,
           MaterialInner material, Light light) {
  vec3 N = normalize(normal);
  vec3 L = normalize(light.position - worldPos);
  vec3 R = reflect(L, N);
  vec3 V = normalize(cameraPos - worldPos);
  vec3 H = normalize(L + V);
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  ambient = light.ambient * material.ambient;
  diffuse = max(dot(L, N), 0.0f) * light.diffuse * material.diffuse;
  specular = pow(max(dot(N, H), 0.0f), material.shininess) * light.specular * material.specular;

  return ambient + diffuse + specular;
}

MaterialInner convert_from_raw(Material material, vec2 texcoord, float shininess) {
  MaterialInner res;
  res.ambient = texture(material.texture_diffuse0, texcoord).rgb;
  res.diffuse = texture(material.texture_diffuse0, texcoord).rgb;
  res.specular = texture(material.texture_specular0, texcoord).rgb;
  res.shininess = shininess;
  return res;
}

void main() {
  fColor = texture(material.texture_diffuse0, texcoordOut0);
  fColor.rgb = blinn_phong(worldPos, cameraPos, normalOut, convert_from_raw(material, texcoordOut0, 32), light);
}
