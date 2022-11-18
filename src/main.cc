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
std::shared_ptr<Model> nanosuit;
// clang-format on

glm::vec3 lightPos(0, 0, 0);

int32_t windowWidth = 1024;
int32_t windowHeight = 720;

Camera camera;

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
  camera.aspect = (float)windowWidth / windowHeight;

  nanosuit = std::make_shared<Model>("assets/nanosuit/nanosuit.obj");

  default_prog->use();
  glEnable(GL_DEPTH_TEST);
}

void display() {
  glUniformMatrix4fv(glGetUniformLocation(default_prog->get_id(), "view"), 1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
  glUniformMatrix4fv(
    glGetUniformLocation(default_prog->get_id(), "projection"), 1, GL_FALSE, glm::value_ptr(camera.getProjectionMatrix()));

  // 传递光源位置
  glUniform3fv(glGetUniformLocation(default_prog->get_id(), "lightPos"), 1, glm::value_ptr(lightPos));
  // 传递相机位置
  glUniform3fv(glGetUniformLocation(default_prog->get_id(), "cameraPos"), 1, glm::value_ptr(camera.position));

  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // nanosuit->draw(default_prog);
  nanosuit->draw(default_prog);
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
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    camera.position += 0.05f * camera.direction;
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    camera.position -= 0.05f * camera.direction;
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    camera.position -= 0.05f * glm::normalize(glm::cross(camera.direction, camera.up));
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    camera.position += 0.05f * glm::normalize(glm::cross(camera.direction, camera.up));
  }

  if (keyboadState[GLFW_KEY_R])
    camera.position.y += 0.05f;
  if (keyboadState[GLFW_KEY_F])
    camera.position.y -= 0.05f;

  if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
    lightPos.z -= 0.05f;
  }
  if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
    lightPos.z += 0.05f;
  }
  if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
    lightPos.x -= 0.05f;
  }
  if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
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

  camera.yaw += xoffset;
  camera.pitch += yoffset;

  camera.pitch = glm::clamp(camera.pitch, -89.0f, 89.0f);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {}

void keyboard_callback(GLFWwindow *window, int32_t key, int32_t scancode, int32_t action, int32_t mods) {
  keyboadState[key] = (action == GLFW_PRESS || action == GLFW_REPEAT) ? true : false;
}
