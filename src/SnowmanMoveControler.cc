#include"SnowmanMoveControler.h"
#include<iostream>

glm::vec3 SnowmanMoveControler::get_model_direction(Model::Ptr model){
    glm::mat4 unit(1.0f);
    glm::mat4 rotate = unit; 
    rotate = glm::rotate(rotate, glm::radians(model->rotate.x), glm::vec3(1, 0, 0));
    rotate = glm::rotate(rotate, glm::radians(model->rotate.y), glm::vec3(0, 1, 0));
    rotate = glm::rotate(rotate, glm::radians(model->rotate.z), glm::vec3(0, 0, 1));

    glm::vec4 direction4 = rotate * glm::vec4(0, 0, 1, 1);
    glm::vec3 direction(direction4.x / direction4.w, direction4.y / direction4.w, direction4.z / direction4.w);

    return direction;
}

void SnowmanMoveControler::move_ahead(float sen, float myDeltaTime, Camera::Ptr camera, Model::Ptr model, Model::Ptr firstPersonal){
    glm::vec3 direction = get_model_direction(model);
    model->translate += sen * myDeltaTime * direction;
}
void SnowmanMoveControler::move_back(float sen, float myDeltaTime, Camera::Ptr camera, Model::Ptr model, Model::Ptr firstPersonal){
    glm::vec3 direction = get_model_direction(model);
    model->translate -= sen * myDeltaTime * direction;
}
void SnowmanMoveControler::move_left(float sen, float myDeltaTime, Camera::Ptr camera, Model::Ptr model, Model::Ptr firstPersonal){
    glm::vec3 direction = get_model_direction(model);
    model->translate -= sen * myDeltaTime * glm::normalize(glm::cross(direction, glm::vec3(0, 1, 0)));
}
void SnowmanMoveControler::move_right(float sen, float myDeltaTime, Camera::Ptr camera, Model::Ptr model, Model::Ptr firstPersonal){
    glm::vec3 direction = get_model_direction(model);
    model->translate += sen * myDeltaTime * glm::normalize(glm::cross(direction, glm::vec3(0, 1, 0)));
}