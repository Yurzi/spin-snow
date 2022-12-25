#include"CammerMoveControler.h"

void CammerMoveControler::move_right(float sen, float myDeltaTime, Camera::Ptr camera, Model::Ptr model, Model::Ptr firstPersonal){
    camera->position += sen * myDeltaTime * glm::normalize(glm::cross(camera->direction, camera->up));
}
void CammerMoveControler::move_left(float sen, float myDeltaTime, Camera::Ptr camera, Model::Ptr model, Model::Ptr firstPersonal){
    camera->position -= sen * myDeltaTime * glm::normalize(glm::cross(camera->direction, camera->up));
}
void CammerMoveControler::move_ahead(float sen, float myDeltaTime, Camera::Ptr camera, Model::Ptr model, Model::Ptr firstPersonal){
    camera->position += sen * myDeltaTime * camera->direction;
}
void CammerMoveControler::move_back(float sen, float myDeltaTime, Camera::Ptr camera, Model::Ptr model, Model::Ptr firstPersonal){
    camera->position -= sen * myDeltaTime * camera->direction;
}