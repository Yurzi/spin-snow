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
#include "model.h"
#include "shader.h"


/* global */
bool keyboadState[1024];
std::shared_ptr<ShaderProgram> default_prog;
std::shared_ptr<Model> nanosuit;
// clang-format on
glm::vec3 scaleControl(1, 1, 1);
glm::vec3 rotateControl(0, 0, 0);
glm::vec3 translateControl(0, 0, 0);

int32_t windowWidth = 1024;
int32_t windowHeight = 720;

glm::vec3 cameraPosition(0, 0, 4);
glm::vec3 cameraDirection(0, 0, -1);
glm::vec3 cameraUp(0, 1, 0);

// 世界体
float fovy = 70.0f;
float aspect = (float)windowWidth / windowHeight;
float zNear = 0.1f, zFar = 100;

// euler angle
float pitch = 0.0f;
float yaw = 0.0f;
float roll = 0.0f;

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

  nanosuit = std::make_shared<Model>("assets/JiaRan/JiaRan.pmx");

  default_prog->use();
  glEnable(GL_DEPTH_TEST);
}

void display() {
  // constuct the model matrix
  glm::mat4 unit(1.0f);
  glm::mat4 scale = glm::scale(unit, scaleControl);
  glm::mat4 translate = glm::translate(unit, translateControl);
  glm::mat4 rotate = unit;
  rotate = glm::rotate(rotate, glm::radians(rotateControl.x), glm::vec3(1, 0, 0));
  rotate = glm::rotate(rotate, glm::radians(rotateControl.y), glm::vec3(0, 1, 0));
  rotate = glm::rotate(rotate, glm::radians(rotateControl.z), glm::vec3(0, 0, 1));

  glm::mat4 model = translate * rotate * scale;
  glUniformMatrix4fv(glGetUniformLocation(default_prog->get_id(), "model"), 1, GL_FALSE, glm::value_ptr(model));

  cameraDirection.x = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
  cameraDirection.y = sin(glm::radians(pitch));
  cameraDirection.z = -cos(glm::radians(pitch)) * cos(glm::radians(yaw));
  glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraDirection, cameraUp);
  glUniformMatrix4fv(glGetUniformLocation(default_prog->get_id(), "view"), 1, GL_FALSE, glm::value_ptr(view));

  glm::mat4 projection = glm::perspective(fovy, aspect, zNear, zFar);
  glUniformMatrix4fv(glGetUniformLocation(default_prog->get_id(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));

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
  if (keyboadState[GLFW_KEY_LEFT_CONTROL]) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
      translateControl.y += 0.0025f;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
      translateControl.y -= 0.0025f;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
      translateControl.x -= 0.0025f;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
      translateControl.x += 0.0015f;
    }
  } else {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
      cameraPosition += 0.05f * cameraDirection;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
      cameraPosition -= 0.05f * cameraDirection;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
      cameraPosition -= 0.05f * glm::normalize(glm::cross(cameraDirection, cameraUp));
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
      cameraPosition += 0.05f * glm::normalize(glm::cross(cameraDirection, cameraUp));
    }
  }

  if (keyboadState[GLFW_KEY_R])
    cameraPosition.y += 0.05f;
  if (keyboadState[GLFW_KEY_F])
    cameraPosition.y -= 0.05f;
}

void mouse_move_callback(GLFWwindow *window, double x, double y) {
  static double lastX = x;
  static double lastY = y;

  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
    rotateControl.y = -100 * (x - float(windowWidth) / 2.0) / windowWidth;
    rotateControl.x = -100 * (y - float(windowHeight) / 2.0) / windowHeight;
  } else {
    float xoffset = x - lastX;
    float yoffset = lastY - y;
    lastX = x;
    lastY = y;

    float sensitivity = 0.05;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    pitch = glm::clamp(pitch, -89.0f, 89.0f);
  }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) { scaleControl += 1 * yoffset * 0.1; }

void keyboard_callback(GLFWwindow *window, int32_t key, int32_t scancode, int32_t action, int32_t mods) {
  keyboadState[key] = (action == GLFW_PRESS || action == GLFW_REPEAT) ? true : false;
}
