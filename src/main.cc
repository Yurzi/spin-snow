// opengl
#include <glad/glad.h>
// gldw
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
// cpp std lib
#include <iostream>
#include <memory>
#include <random>
#include <vector>

// c std lib
#include <cstdint>
// project header
#include "camera.h"
#include "light.h"
#include "model.h"
#include "shader.h"
#include "utils.h"
#include "MoveControler.h"
#include "CammerMoveControler.h"
#include "SnowmanMoveControler.h"
#include "FirstPersonalMoveControler.h"


/* global */
bool keyboardState[1024];
ShaderProgram::Ptr default_prog;
ShaderProgram::Ptr shadow_prog;
ShaderProgram::Ptr debug;
ShaderProgram::Ptr dot_light_prog;
ShaderProgram::Ptr skybox_prog;
ShaderProgram::Ptr transparency_prog;

static const int64_t SNOWFLAKES_COUNT = 64;
std::random_device rd;
std::ranlux48 random_engine(rd());

Model::Ptr model;
Model::Ptr cube_light;
Model::Ptr skybox;
std::vector<Model::Ptr> snowflakes;
Model::Ptr snowman_firstpersonal;
Model::Ptr person;
Model::Ptr mc_model;

Mesh::Ptr ground;
Mesh::Ptr screen;
Mesh::Ptr grass;

Light light;
Texture skybox_tex(Texture::unknown);

int32_t windowWidth = 1024;
int32_t windowHeight = 720;

Camera::Ptr camera;
Camera::Ptr shadow_camera;

int32_t shadowMapResolution = 16384;
GLuint shadowMapFBO;
Texture::Ptr shadowTexture;

CammerMoveControler cammerMoveControler;
SnowmanMoveControler snowmanMoveControler;
FirstPersonalMoveControler firstPersonalMoveControler;
MoveControler*moveControler = &snowmanMoveControler;


float deltaTime = 0;
bool first_personal = false;
glm::vec3 history_location(0, 0, 0);
extern glm::vec3 first_personal_camera_y;


/* functions */
// callback function for window size changed
void frambuffer_size_callback(GLFWwindow *window, int32_t width, int32_t height);
void mouse_move_callback(GLFWwindow *window, double x, double y);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void keyboard_callback(GLFWwindow *window, int32_t key, int32_t scancode, int32_t action, int32_t mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void rotate_cammer();
bool rotate_cammer_state(bool is_change);
float get_time_delta();
// process user input
void processInput(GLFWwindow *window);
Model::Ptr genSnowflakes() {
  // gen plain snowflakes
  static Model snowflakes_template("assets/snowflakes.obj");
  Model::Ptr snowflakes = std::make_shared<Model>(snowflakes_template);

  // setting origin place
  std::uniform_real_distribution<float> dist(-50, 50);
  std::uniform_real_distribution<float> height(0, 16);

  snowflakes->translate = glm::vec3(dist(random_engine), 50 + height(random_engine), dist(random_engine));
  snowflakes->rotate = glm::vec3(dist(random_engine), dist(random_engine), dist(random_engine));
  snowflakes->scale = glm::vec3(8, 8, 8);

  return snowflakes;
}

void anmineSnowflakes() {
  std::uniform_real_distribution<float> dist(0, 1.1);
  std::uniform_real_distribution<float> height(0, 16);
  for (auto item : snowflakes) {
    // 进行下落
    if (item->translate.y <= -10) {
      item->translate.y = 50 + height(random_engine);
    }
    item->translate.y -= dist(random_engine);

    item->rotate.z += 10 * dist(random_engine);
    if (item->rotate.z >= 360) {
      item->rotate.z -= 360;
    }
    item->rotate.y += 10 * dist(random_engine);
    if (item->rotate.y >= 360) {
      item->rotate.y -= 360;
    }
    item->rotate.x += 10 * dist(random_engine);
    if (item->rotate.x >= 360) {
      item->rotate.x -= 360;
    }
  }
}
// init function
void init() {
  // init shader
  default_prog = std::make_shared<ShaderProgram>("shaders/default.vert", "shaders/default.frag");
  dot_light_prog = std::make_shared<ShaderProgram>("shaders/default.vert", "shaders/dot_light.frag");
  shadow_prog = std::make_shared<ShaderProgram>("shaders/shadow.vert", "shaders/shadow.frag");
  debug = std::make_shared<ShaderProgram>("shaders/debug.vert", "shaders/debug.frag");
  skybox_prog = std::make_shared<ShaderProgram>("shaders/skybox.vert", "shaders/skybox.frag");
  transparency_prog = std::make_shared<ShaderProgram>("shaders/default.vert","shaders/transparency.frag");

  // init camera
  camera = std::make_shared<Camera>();
  camera->aspect = (float)windowWidth / windowHeight;
  camera->position = {0, 30, 50};

  // init light
  light.position = glm::vec3(3.0f, 30.0f, 80.0f);
  light.type = Light::SunLight;
  light.ambient = {0.45, 0.45, 0.45};
  light.diffuse = {(float)218 / 255, (float)218 / 255, (float)192 / 255};

  // init objects;
  model = std::make_shared<Model>("assets/snowman.obj");
  snowman_firstpersonal = std::make_shared<Model>("assets/snowmanfirstperson.obj");
  person = std::make_shared<Model>("assets/sl/神里绫华.pmx");
  mc_model = std::make_shared<Model>("assets/icehouse/icehouse.obj");
  for (uint16_t i = 0; i < SNOWFLAKES_COUNT; ++i) {
    snowflakes.push_back(genSnowflakes());
  }
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
  ground->add_texture(std::make_shared<Texture>("assets/wall.jpg", Texture::diffuse));
  ground->add_texture(std::make_shared<Texture>(Texture::specular, Texture2DFromUChar(nullptr)));

  std::vector<std::string> files = {
    "assets/skybox/right.jpg",
    "assets/skybox/left.jpg",
    "assets/skybox/top.jpg",
    "assets/skybox/bottom.jpg",
    "assets/skybox/front.jpg",
    "assets/skybox/back.jpg",
  };
  skybox_tex.id = CubeMapFromFile(files);

  grass->add_texture(std::make_shared<Texture>("assets/nya.png", Texture::diffuse, true));


  // shadow
  shadow_camera = std::make_shared<Camera>();
  shadow_camera->mode = Camera::DefaultAngle | Camera::Ortho;
  shadow_camera->left = -100;
  shadow_camera->right = 100;
  shadow_camera->bottom = -100;
  shadow_camera->top = 100;
  shadow_camera->position = light.position;
  //shadow_camera->fovy = 120;

  shadowTexture = std::make_shared<Texture>(Texture::shadow);
  glGenFramebuffers(1, &shadowMapFBO);
  shadowTexture->type = Texture::shadow;
  shadowTexture->id = Texture2DForShadowMap(shadowMapResolution, shadowMapResolution, GL_CLAMP_TO_BORDER);
  glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTexture->id, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // properties setting
  person->translate = glm::vec3(20, 0, 70);
  mc_model->translate = glm::vec3(0, -5, 0);
  mc_model->scale = {5,5,5};
  mc_model->rotate = {0, 180, 0};
  //model->scale = {8, 8, 8};
  ground->scale = glm::vec3(50, 50, 50);
  cube_light->scale = {0.2, 0.2, 0.2};
  cube_light->translate = light.position;

  grass->translate = {10, 1, 0};

  ground->add_texture(shadowTexture);
  model->add_texture(shadowTexture);
  screen->add_texture(shadowTexture);
  grass->add_texture(shadowTexture);
  snowman_firstpersonal->add_texture(shadowTexture);
  person->add_texture(shadowTexture);
  mc_model->add_texture(shadowTexture);
  for (auto item : snowflakes) {
    item->add_texture(shadowTexture);
  }

  default_prog->use();

  glEnable(GL_DEPTH_TEST);
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
  transparency_prog->set_light("light", light);
  transparency_prog->set_uniform("shadow_zNear", shadow_camera->zNear);
  transparency_prog->set_uniform("shadow_zFar", shadow_camera->zFar);

  // 传递相机位置
  default_prog->set_uniform("cameraPos", camera->position);
  transparency_prog->set_uniform("cameraPos", camera->position);


  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  /*-----draw objs-------*/

  // shadow draw
  glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
  glClear(GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, shadowMapResolution, shadowMapResolution);
  for (uint16_t i = 0; i < SNOWFLAKES_COUNT; ++i) {
    snowflakes[i]->draw(shadow_prog, shadow_camera);
  }
  if(first_personal){
    snowman_firstpersonal->draw(shadow_prog, shadow_camera);
  }else{
    model->draw(shadow_prog, shadow_camera);
  }
  
  person->draw(shadow_prog, shadow_camera);
  mc_model->draw(shadow_prog, shadow_camera);
  //ground->draw(shadow_prog, shadow_camera);
  grass->draw(shadow_prog, shadow_camera);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // default draw
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, windowWidth, windowHeight);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  skybox_prog->use();
  glActiveTexture(GL_TEXTURE16);
  glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex.id);
  skybox_prog->set_uniform("skybox", 16);

  skybox->translate = camera->position;
  glDepthMask(GL_FALSE);
  skybox->draw(skybox_prog, camera);
  glDepthMask(GL_TRUE);

  cube_light->translate = light.position;
  cube_light->draw(dot_light_prog, camera);


  default_prog->use();
  default_prog->set_uniform("shadowVP", shadow_camera->getProjectionMatrix() * shadow_camera->getViewMatrix());
  transparency_prog->use();
  transparency_prog->set_uniform("shadowVP", shadow_camera->getProjectionMatrix() * shadow_camera->getViewMatrix());

  for (uint16_t i = 0; i < SNOWFLAKES_COUNT; ++i) {
    snowflakes[i]->draw(default_prog, camera);
  }
  if(first_personal){
    snowman_firstpersonal->draw(default_prog, camera);
  }else{
    model->draw(default_prog, camera);
  }
  person->draw(default_prog, camera);
  //ground->draw(default_prog, camera);
  //grass->draw(default_prog, camera);
  
  transparency_prog->use();
  transparency_prog->set_uniform("shadowVP", shadow_camera->getProjectionMatrix() * shadow_camera->getViewMatrix());
  grass->draw(transparency_prog, camera);
  mc_model->draw(transparency_prog, camera);
  
  debug->use();
  //  glDisable(GL_DEPTH_TEST);
  //  glViewport(0, 0, windowWidth / 3, windowHeight / 3);
  //  screen->draw(debug);
  //  glEnable(GL_DEPTH_TEST);
  //  glDisable(GL_BLEND);

  anmineSnowflakes();
}

// main
int main(int argc, char *argv[]) {
  // init glfw
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // create window
  GLFWwindow *window = glfwCreateWindow(windowWidth, windowHeight, "Snow", nullptr, nullptr);
  if (window == nullptr) {
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
  glfwSetMouseButtonCallback(window, mouse_button_callback);

  init();

  // loop for continuios render and event loop for window
  while (!glfwWindowShouldClose(window)) {
    //delta time
    //-------------------------------------
    deltaTime = get_time_delta();
    // user input
    // ------------------------------------
    processInput(window);
    // rotate cammer
    // ------------------------------------
    rotate_cammer();
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
  float myDeltaTime = deltaTime;
  float sen = 10.0f;
  if (keyboardState[GLFW_KEY_ESCAPE]) {
    glfwSetWindowShouldClose(window, true);
  }
  if (keyboardState[GLFW_KEY_W]) {
    moveControler->move_ahead(sen, myDeltaTime, camera, model, snowman_firstpersonal);
  }
  if (keyboardState[GLFW_KEY_S]) {
    moveControler->move_back(sen, myDeltaTime, camera, model, snowman_firstpersonal);
  }
  if (keyboardState[GLFW_KEY_A]) {
    moveControler->move_left(sen, myDeltaTime, camera, model, snowman_firstpersonal);
  }
  if (keyboardState[GLFW_KEY_D]) {
    moveControler->move_right(sen, myDeltaTime, camera, model, snowman_firstpersonal);
  }

  if (keyboardState[GLFW_KEY_R] && !first_personal)
    camera->position.y += sen * myDeltaTime;
  if (keyboardState[GLFW_KEY_F] && !first_personal)
    camera->position.y -= sen * myDeltaTime;

  if (keyboardState[GLFW_KEY_I]) {
    light.position.z -= 0.05f;
  }
  if (keyboardState[GLFW_KEY_K]) {
    light.position.z += 0.05f;
  }
  if (keyboardState[GLFW_KEY_J]) {
    light.position.x -= 0.05f;
  }
  if (keyboardState[GLFW_KEY_L]) {
    light.position.x += 0.05f;
  }
  if (keyboardState[GLFW_KEY_U])
    light.position.y += 0.05f;
  if (keyboardState[GLFW_KEY_H])
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

  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE && !first_personal) 
  {
    camera->yaw += xoffset;
    camera->pitch += yoffset;
    camera->pitch = glm::clamp(camera->pitch, -89.0f, 89.0f);
  }else{
    if(first_personal){
      snowman_firstpersonal->rotate -= glm::vec3(0, xoffset, 0);
      camera->direction = snowmanMoveControler.get_model_direction(snowman_firstpersonal);
      camera->position = snowman_firstpersonal->translate + first_personal_camera_y + snowmanMoveControler.get_model_direction(snowman_firstpersonal);
    }else{
      model->rotate -= glm::vec3(0, xoffset, 0);
    }
    
  }
  

}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {}

void keyboard_callback(GLFWwindow *window, int32_t key, int32_t scancode, int32_t action, int32_t mods) {
  keyboardState[key] = (action == GLFW_PRESS || action == GLFW_REPEAT) ? true : false;

  if (key == GLFW_KEY_C && action == GLFW_PRESS && !first_personal) {
    rotate_cammer_state(true);
  }
  if (key == GLFW_KEY_V && action == GLFW_PRESS && !first_personal) {
    moveControler = &cammerMoveControler;
  }
  if (key == GLFW_KEY_B && action == GLFW_PRESS && !first_personal) {
    moveControler = &snowmanMoveControler;
  }
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
  if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
    first_personal = !first_personal;
    if(first_personal){
      snowman_firstpersonal->translate = model->translate;
      snowman_firstpersonal->rotate = model->rotate;
      history_location = camera->position;
      camera->mode = camera->mode & ~Camera::EulerAngle;
      camera->position = snowman_firstpersonal->translate + first_personal_camera_y + snowmanMoveControler.get_model_direction(snowman_firstpersonal);
      camera->direction = snowmanMoveControler.get_model_direction(snowman_firstpersonal);

      moveControler = &firstPersonalMoveControler;
    }else{
      model->translate = snowman_firstpersonal->translate;
      model->rotate = snowman_firstpersonal->rotate;
      camera->mode = camera->mode | Camera::EulerAngle;
      camera->position = history_location;

      moveControler = &snowmanMoveControler;

    }
    
    
  }
}
void rotate_cammer() {

  if (rotate_cammer_state(false)) {
    float myDeltaTime = deltaTime;
    float sen = 1;
    float speed = sen * myDeltaTime * 10;
    float pointx = 0.0, pointz = 0.0;
    glm::vec3 old_position = camera->position;
    float old_position_x = old_position.x;
    float old_position_z = old_position.z;

    float vecx = old_position_x - pointx;
    float vecz = old_position_z - pointz;
    // float r = sqrt(pow(old_position_x, 2) + pow(old_position_z, 2));
    float dir_x = 0;
    float dir_z = 0;
    if (old_position_z == pointz) {
      dir_x = 0;
      if (old_position_x > pointx) {
        dir_z = 1;
      } else {
        dir_z = -1;
      }
    } else {
      dir_x = old_position_z > pointz ? -1 : 1;
      dir_z = -1 * dir_x * vecx / vecz;
      // 归一化
      float sum = std::abs(dir_x) + std::abs(dir_z);
      dir_x /= sum;
      dir_z /= sum;
    }

    float delx = dir_x * speed;
    float delz = dir_z * speed;
    float new_position_x = old_position_x + delx;
    float new_position_z = old_position_z + delz;

    camera->position.x = new_position_x;
    camera->position.z = new_position_z;
  }
}
bool rotate_cammer_state(bool is_change) {
  static bool state = false;
  if (is_change) {
    state = !state;
  }
  return state;
}

float get_time_delta(){
  static float last_frame = glfwGetTime();
  float current_frame = glfwGetTime();
  float deltaTime = current_frame - last_frame;
  last_frame = current_frame;
  return deltaTime;
}