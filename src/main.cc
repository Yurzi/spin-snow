// opengl
#include <glad/glad.h>
// gldw
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// cpp std lib
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>
// c std lib
#include <stb_image.h>
#include <stdint.h>
// project header
#include "camera.h"
#include "light.h"
#include "model.h"
#include "shader.h"
#include "utils.h"

/* global */
bool keyboadState[1024];
std::shared_ptr<ShaderProgram> default_prog;
std::shared_ptr<ShaderProgram> shadow_prog;
std::shared_ptr<ShaderProgram> debug;
std::shared_ptr<ShaderProgram> dot_light_prog;
std::shared_ptr<ShaderProgram> skybox_prog;

std::shared_ptr<Model> model;
std::shared_ptr<Model> cube_light;
std::shared_ptr<Model> skybox;

std::shared_ptr<Mesh> ground;
std::shared_ptr<Mesh> screen;
std::shared_ptr<Mesh> grass;

Light light;
Texture skybox_tex;

int32_t windowWidth = 1024;
int32_t windowHeight = 720;

std::shared_ptr<Camera> camera;
std::shared_ptr<Camera> shadow_camera;

int32_t shadowMapResolution = 4096;
GLuint shadowMapFBO;
Texture shadowTexture;

/* functions */
// callback function for window size changed
void frambuffer_size_callback(GLFWwindow *window, int32_t width, int32_t height);
void mouse_move_callback(GLFWwindow *window, double x, double y);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void keyboard_callback(GLFWwindow *window, int32_t key, int32_t scancode, int32_t action, int32_t mods);
// process user input
void processInput(GLFWwindow *window);
// init function
void init() {
  // init shader
  default_prog = std::make_shared<ShaderProgram>("shaders/default.vert", "shaders/default.frag");
  dot_light_prog = std::make_shared<ShaderProgram>("shaders/default.vert", "shaders/dot_light.frag");
  shadow_prog = std::make_shared<ShaderProgram>("shaders/shadow.vert", "shaders/shadow.frag");
  debug = std::make_shared<ShaderProgram>("shaders/debug.vert", "shaders/debug.frag");
  skybox_prog = std::make_shared<ShaderProgram>("shaders/skybox.vert", "shaders/skybox.frag");

  // init camera
  camera = std::make_shared<Camera>();
  camera->aspect = (float)windowWidth / windowHeight;
  camera->position = {0, 3, 5};

  // init light
  light.position = glm::vec3(3.0f, 20.0f, 10.0f);
  light.type = Light::PointLight;
  light.ambient = {0.45, 0.45, 0.45};
  light.diffuse = {(float)218 / 255, (float)218 / 255, (float)192 / 255};

  // init objects;
  model = std::make_shared<Model>("assets/naheida/纳西妲.pmx");
  cube_light = std::make_shared<Model>("assets/cube.obj");
  skybox = std::make_shared<Model>("assets/cube.obj");
  ground = std::make_shared<Mesh>();
  screen = std::make_shared<Mesh>();
  screen->vertices = {
    {{-1, -1, 0}, {0, 1, 0}, {0, 0}},
    { {1, -1, 0}, {0, 1, 0}, {1, 0}},
    { {-1, 1, 0}, {0, 1, 0}, {0, 1}},
    {  {1, 1, 0}, {0, 1, 0}, {1, 1}}
  };
  ground->vertices = {
    {{-1, 0, -1}, {0, 1, 0}, {0, 0}},
    { {1, 0, -1}, {0, 1, 0}, {1, 0}},
    { {-1, 0, 1}, {0, 1, 0}, {0, 1}},
    {  {1, 0, 1}, {0, 1, 0}, {1, 1}}
  };
  ground->indices = {0, 1, 2, 2, 1, 3};
  screen->indices = {0, 1, 2, 2, 1, 3};
  grass = std::make_shared<Mesh>(*screen);
  ground->setup();
  screen->setup();

  // texture init
  Texture texture;
  texture.type = Texture::diffuse;
  texture.path = "assets/wall.jpg";
  texture.id = Texture2DFromFile(texture.path);
  ground->add_texture(texture);
  texture.type = Texture::specular;
  texture.id = Texture2DFromUChar(nullptr);
  ground->add_texture(texture);

  std::vector<std::string> files = {
    "assets/skybox/right.jpg",
    "assets/skybox/left.jpg",
    "assets/skybox/top.jpg",
    "assets/skybox/bottom.jpg",
    "assets/skybox/front.jpg",
    "assets/skybox/back.jpg",
  };
  skybox_tex.id = CubeMapFromFile(files);

  texture.type = Texture::diffuse;
  texture.path = "assets/nya.png";
  stbi_set_flip_vertically_on_load(true);
  texture.id = Texture2DFromFile(texture.path);
  stbi_set_flip_vertically_on_load(false);
  grass->add_texture(texture);


  // shadow
  shadow_camera = std::make_shared<Camera>();
  shadow_camera->mode = Camera::DefaultAngle | Camera::Ortho;
  shadow_camera->left = -25;
  shadow_camera->right = 25;
  shadow_camera->bottom = -25;
  shadow_camera->top = 25;
  shadow_camera->position = light.position;
  shadow_camera->fovy = 120;

  glGenFramebuffers(1, &shadowMapFBO);
  shadowTexture.type = Texture::shadow;
  shadowTexture.id = Texture2DForShadowMap(shadowMapResolution, shadowMapResolution, GL_CLAMP_TO_BORDER);
  glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTexture.id, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // properties setting
  ground->scale = glm::vec3(50, 50, 50);
  cube_light->scale = {0.2, 0.2, 0.2};
  cube_light->translate = light.position;

  grass->translate = {10, 1, 0};

  ground->add_texture(shadowTexture);
  model->add_texture(shadowTexture);
  screen->add_texture(shadowTexture);
  grass->add_texture(shadowTexture);

  default_prog->use();

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void display() {
  // 传递光源位置
  shadow_camera->position = light.position;
  shadow_camera->direction = glm::normalize(glm::vec3(0, 0, 0) - shadow_camera->position);
  shadow_camera->aspect = (float)windowWidth / windowHeight;
  default_prog->set_light("light", light);
  default_prog->set_uniform("shadow_zNear", shadow_camera->zNear);
  default_prog->set_uniform("shadow_zFar", shadow_camera->zFar);
  dot_light_prog->set_light("light", light);

  // 传递相机位置
  default_prog->set_uniform("cameraPos", camera->position);

  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  /*-----draw objs-------*/

  // shadow draw
  shadow_prog->use();
  glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
  glClear(GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, shadowMapResolution, shadowMapResolution);
  model->draw(shadow_prog, shadow_camera);
  ground->draw(shadow_prog, shadow_camera);
  grass->draw(shadow_prog, shadow_camera);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // default draw
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, windowWidth, windowHeight);

  skybox_prog->use();
  glActiveTexture(GL_TEXTURE16);
  glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex.id);
  skybox_prog->set_uniform("skybox", 16);

  skybox->translate = camera->position;
  glDepthMask(GL_FALSE);
  skybox->draw(skybox_prog, camera);
  glDepthMask(GL_TRUE);

  dot_light_prog->use();
  cube_light->translate = light.position;
  cube_light->draw(dot_light_prog, camera);

  default_prog->use();
  default_prog->set_uniform("shadowVP", shadow_camera->getProjectionMatrix() * shadow_camera->getViewMatrix());

  model->draw(default_prog, camera);
  ground->draw(default_prog, camera);
  grass->draw(default_prog, camera);
  /*
    debug->use();
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, windowWidth / 3, windowHeight / 3);
    screen->draw(debug);
    glEnable(GL_DEPTH_TEST);
      */
}

// main
int main(int argc, char *argv[]) {
  // init glfw
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // create window
  GLFWwindow *window = glfwCreateWindow(windowWidth, windowHeight, "Snow", NULL, NULL);
  if (window == NULL) {
    // check is success
    std::cout << "Failed to initialize GLFW Window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  // init glad
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  // set viewport
  glViewport(0, 0, windowWidth, windowHeight);
  // set callback function for window size change
  glfwSetFramebufferSizeCallback(window, frambuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_move_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetKeyCallback(window, keyboard_callback);

  init();

  // loop for continuios render and event loop for window
  while (!glfwWindowShouldClose(window)) {
    // user input
    // ------------------------------------
    processInput(window);
    // render
    // ------------------------------------
    display();
    // event dispatch and swap buffer
    // -----------------------------------
    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  // for exit
  glfwTerminate();
  return 0;
}

// callback function for window size changed
void frambuffer_size_callback(GLFWwindow *window, int32_t width, int32_t height) {
  windowWidth = width;
  windowHeight = height;
  glViewport(0, 0, width, height);
  return;
}
// process user input
void processInput(GLFWwindow *window) {
  if (keyboadState[GLFW_KEY_ESCAPE]) {
    glfwSetWindowShouldClose(window, true);
  }
  if (keyboadState[GLFW_KEY_W]) {
    camera->position += 0.05f * camera->direction;
  }
  if (keyboadState[GLFW_KEY_S]) {
    camera->position -= 0.05f * camera->direction;
  }
  if (keyboadState[GLFW_KEY_A]) {
    camera->position -= 0.05f * glm::normalize(glm::cross(camera->direction, camera->up));
  }
  if (keyboadState[GLFW_KEY_D]) {
    camera->position += 0.05f * glm::normalize(glm::cross(camera->direction, camera->up));
  }

  if (keyboadState[GLFW_KEY_R])
    camera->position.y += 0.05f;
  if (keyboadState[GLFW_KEY_F])
    camera->position.y -= 0.05f;

  if (keyboadState[GLFW_KEY_I]) {
    light.position.z -= 0.05f;
  }
  if (keyboadState[GLFW_KEY_K]) {
    light.position.z += 0.05f;
  }
  if (keyboadState[GLFW_KEY_J]) {
    light.position.x -= 0.05f;
  }
  if (keyboadState[GLFW_KEY_L]) {
    light.position.x += 0.05f;
  }
  if (keyboadState[GLFW_KEY_U])
    light.position.y += 0.05f;
  if (keyboadState[GLFW_KEY_H])
    light.position.y -= 0.05f;
}

void mouse_move_callback(GLFWwindow *window, double x, double y) {
  static double lastX = x;
  static double lastY = y;

  float xoffset = x - lastX;
  float yoffset = lastY - y;
  lastX = x;
  lastY = y;

  float sensitivity = 0.05;
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  camera->yaw += xoffset;
  camera->pitch += yoffset;

  camera->pitch = glm::clamp(camera->pitch, -89.0f, 89.0f);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {}

void keyboard_callback(GLFWwindow *window, int32_t key, int32_t scancode, int32_t action, int32_t mods) {
  keyboadState[key] = (action == GLFW_PRESS || action == GLFW_REPEAT) ? true : false;
}
