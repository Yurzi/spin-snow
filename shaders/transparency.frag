#version 330 core
in vec2 texcoordOut0;
in vec3 worldPos;
in vec3 normalOut;

out vec4 fColor;

uniform vec3 cameraPos;

struct Texture {
// texture map
  sampler2D diffuse0;
  sampler2D specular0;
  sampler2D shadow0;
  sampler2D alpha0;
};

struct Material {
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
uniform Texture textures;
uniform Light light;

uniform mat4 shadowVP;

uniform float shadow_zNear;
uniform float shadow_zFar;

float linearize_depth(float depth, float near_plane, float far_plane) {
  float z = depth * 2.0 - 1.0;
  return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

vec3 blinn_phong(vec3 worldPos, vec3 cameraPos, vec3 normal,
           Material material, Light light, float shadow) {
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

  return ambient + (1 -shadow) * (diffuse + specular);
}

Material convert_from_texture(Texture textures, vec2 texcoord, float shininess) {
  Material res;
  res.ambient = texture(textures.diffuse0, texcoord).rgb;
  res.diffuse = texture(textures.diffuse0, texcoord).rgb;
  res.specular = texture(textures.specular0, texcoord).rgb;
  res.shininess = shininess;
  return res;
}

float shadowMapping(sampler2D tex, mat4 shadowVP, vec4 worldPos, sampler2D alphaTex) {
  vec4 light_view_pos = shadowVP * worldPos;
  light_view_pos = vec4(light_view_pos.xyz/light_view_pos.w, 1.0f);
  light_view_pos = light_view_pos * 0.5 + 0.5;

  float closetDepth = texture(tex, light_view_pos.xy).r;
  float currentDepth = light_view_pos.z;
  vec3 lightDir = light.position - worldPos.xyz;
  float bias = max(0.05 * (1.0 - dot(normalOut, lightDir)), 0.005);


  float shadow = 0;
  vec2 texelSize = 1.0 / textureSize(tex, 0);
  
  for(int x = -1; x <= 1; ++x)
    {
      for(int y = -1; y <= 1; ++y)
        {
          float pcfDepth = texture(tex, light_view_pos.xy + vec2(x, y) * texelSize).r; 
          shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
  shadow /= 9.0; 

  float alpha = texture(alphaTex, light_view_pos.xy).r;
  shadow *= alpha * alpha * alpha * alpha;
  if (light_view_pos.z > 1) {
    shadow = 0.0;
  }
  return shadow;
}

void main() {
  fColor = texture(textures.diffuse0, texcoordOut0);
  if(fColor.a < 0.1){
    discard;
  }
  float shadow = shadowMapping(textures.shadow0, shadowVP, vec4(worldPos, 1.0f), textures.alpha0);
  shadow = min(shadow, 0.75);
  fColor.rgb = blinn_phong(worldPos, cameraPos, normalOut, convert_from_texture(textures, texcoordOut0, 32), light, shadow);
}