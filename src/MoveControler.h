#pragma once
// opengl
#include <glad/glad.h>
// gldw
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
// cpp std lib
#include <iostream>
// c std lib

// project header
#include "camera.h"
#include "light.h"
#include "model.h"
#include "shader.h"
#include "utils.h"

class MoveControler{
public:
    void virtual move_right(float sen, float myDeltaTime, Camera::Ptr camera, Model::Ptr model, Model::Ptr firstPersonal)=0;
    void virtual move_left(float sen, float myDeltaTime, Camera::Ptr camera, Model::Ptr model, Model::Ptr firstPersonal)=0;
    void virtual move_ahead(float sen, float myDeltaTime, Camera::Ptr camera, Model::Ptr model, Model::Ptr firstPersonal)=0;
    void virtual move_back(float sen, float myDeltaTime, Camera::Ptr camera, Model::Ptr model, Model::Ptr firstPersonal)=0;
};