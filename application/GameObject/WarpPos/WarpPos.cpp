#include "WarpPos.h"
#include"Model/ModelManager.h"
#include<cmath>
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"

WarpPos::WarpPos()
{
    camera_ = std::make_unique<Camera>();
    object3d_ = std::make_unique<Object3d>();
    transform_= { .scale = {0.1f,0.1f,0.1f},.rotate = { 0.0f,0.0f,0.0f},.translate = { 0.0f,0.0f,0.0f}};
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/camera", "camera");
    object3d_->SetModel("camera");
    object3d_->SetTransform(transform_);
}

Vector3 WarpPos::GetWorldPos()
{
    return YoshidaMath::GetWorldPosByMat(object3d_->GetWorldMatrix());
}

void WarpPos::Initialize()
{
   
    object3d_->Initialize();
    sinTheta_ = 0.0f;
}

void WarpPos::SetCamera(Camera* camera)
{
    object3d_->SetCamera(camera);
}

void WarpPos::Update()
{

    Matrix4x4 child = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

    if (parentTransform_) {
        Matrix4x4 parent = Function::MakeAffineMatrix(parentTransform_->scale, parentTransform_->rotate, parentTransform_->translate);
        child = Function::Multiply( child, parent);
    } 

    sinTheta_ += Function::kPi * YoshidaMath::kDeltaTime;
   
    if (sinTheta_ >= Function::kPi*2.0f) {
        sinTheta_ = 0.0f;
    }

    transform_.translate.y += std::sinf(sinTheta_)*0.0625f;

    camera_->SetWorldMatrix(child);
    object3d_->SetWorldMatrix(child);

    camera_->Update();
    object3d_->Update();
}

void WarpPos::Draw()
{
    object3d_->UpdateCameraMatrices();
    object3d_->Draw();
}
