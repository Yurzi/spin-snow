#pragma once

#include"MoveControler.h"

class CammerMoveControler : public MoveControler{
public:
    void virtual move_right(float sen, float myDeltaTime, Camera::Ptr camera, Model::Ptr model, Model::Ptr firstPersonal);
    void virtual move_left(float sen, float myDeltaTime, Camera::Ptr camera, Model::Ptr model, Model::Ptr firstPersonal);
    void virtual move_ahead(float sen, float myDeltaTime, Camera::Ptr camera, Model::Ptr model, Model::Ptr firstPersonal);
    void virtual move_back(float sen, float myDeltaTime, Camera::Ptr camera, Model::Ptr model, Model::Ptr firstPersonal);
};