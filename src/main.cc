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
#include "model.h"
#include "shader.h"


/* global */
bool keyboadState[1024];
std::shared_ptr<ShaderProgram> default_prog;
std::shared_ptr<ShaderProgram> dot_light_prog;

std::shared_ptr<Model> cube;
std::shared_ptr<Model> cube_light;

glm::vec3 lightPos(2.2f, 2.0f, -4.0f);

int32_t windowWidth = 1024;
int32_t windowHeight = 720;

std::shared_ptr<Camera> camera;

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
  default_prog = std::make_shared<ShaderProgram>("shaders/default.vert", "shaders/default.frag");
  dot_light_prog = std::make_shared<ShaderProgram>("shaders/default.vert", "shaders/dot_light.frag");

  camera = std::make_shared<Camera>();
  camera->aspect = (float)windowWidth / windowHeight;
  camera->position = {0, 3, 5};

  // init objects;
  cube = std::make_shared<Model>("assets/cube.obj");
  cube_light = std::make_shared<Model>("assets/cube.obj");

  cube_light->scale = {0.2, 0.2, 0.2};
  cube_light->translate = lightPos;


  default_prog->use();
  default_prog->set_unifom("objectColor", {1.0f, 0.5f, 0.31f});
  default_prog->set_unifom("lightColor", {1.0f, 1.0f, 1.0f});

  glEnable(GL_DEPTH_TEST);
}

void display() {
  // 传递光源位置
  default_prog->set_unifom("lightPos", lightPos);
  // 传递相机位置
  default_prog->set_unifom("cameraPos", camera->position);

  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  /*-----draw objs-------*/
  default_prog->use();
  cube->draw(default_prog, camera);

  dot_light_prog->use();
  cube_light->translate = lightPos;
  cube_light->draw(dot_light_prog, camera);
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
    lightPos.z -= 0.05f;
  }
  if (keyboadState[GLFW_KEY_K]) {
    lightPos.z += 0.05f;
  }
  if (keyboadState[GLFW_KEY_J]) {
    lightPos.x -= 0.05f;
  }
  if (keyboadState[GLFW_KEY_L]) {
    lightPos.x += 0.05f;
  }
  if (keyboadState[GLFW_KEY_U])
    lightPos.y += 0.05f;
  if (keyboadState[GLFW_KEY_H])
    lightPos.y -= 0.05f;
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
