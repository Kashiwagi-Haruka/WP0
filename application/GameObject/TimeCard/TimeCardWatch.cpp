#include "TimeCardWatch.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/YoshidaMath/CollisionManager/Collider.h"
#include<imgui.h>
#include"Camera.h"
#include"Audio.h"

bool TimeCardWatch::canMakePortal_ = false;

namespace {
    float tMin_ = 0.0f;
    float tMax_ = 5.0f;
}

TimeCardWatch::TimeCardWatch()
{
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/timeCard", "timeCard");
    modelObj_ = std::make_unique<Object3d>();
    modelObj_->SetModel("timeCard");
    ring_ = std::make_unique<Primitive>();
}

void TimeCardWatch::Initialize()
{
    canMakePortal_ = false;
    modelObj_->Initialize();
    ring_->Initialize(Primitive::Ring);
    ring_->SetColor({ 1.0f,0.0f,0.0f,1.0f });
    ring_->SetEnableLighting(false);
    transform_ = { .scale = {20.0f,20.0f,20.0f},.rotate = {-Function::kPi,0.0f,2.55f},.translate = {1.5f,-1.2f,1.5f} };
    ringTransform_ = { .scale = {1.0f,1.0f,1.0f},.rotate = {0.0f,0.0f,0.0f},.translate = {0.0f,0.0f,0.0f} };
}

void TimeCardWatch::SetCamera(Camera* camera)
{
    //カメラのセット
    modelObj_->SetCamera(camera);
    ring_->SetCamera(camera);
    camera_ = camera;
    assert(camera_);
}

void TimeCardWatch::Update()
{
    Matrix4x4 child = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    assert(parentTransform_);
    Matrix4x4 parent = camera_->GetWorldMatrix();
    child = Function::Multiply(child, parent);

    if (canMakePortal_) {
        MakeChildMat();
    } else {
        MakeWorldMat();
    }

    ring_->SetWorldMatrix(ringMatWorld_);
    ring_->Update();

    modelObj_->SetWorldMatrix(child);
    modelObj_->Update();


#ifdef USE_IMGUI
    ImGui::Begin("timeCardWatch");
    ImGui::DragFloat3("translate", &transform_.translate.x, 0.3f);
    ImGui::DragFloat3("scale", &transform_.scale.x, 0.3f);
    ImGui::DragFloat3("rotate", &transform_.rotate.x, 0.3f);
    ImGui::End();
#endif
}

void TimeCardWatch::Draw()
{
    modelObj_->UpdateCameraMatrices();
    modelObj_->Draw();   

    if (canMakePortal_) {
        ring_->UpdateCameraMatrices();
        ring_->Draw();
    }
}

bool TimeCardWatch::OnCollisionObjOfMakePortal(const Ray& ray, const AABB& aabb, const Transform& transform)
{
    //ポータル作れるよ
    canMakePortal_ = YoshidaMath::RayIntersectsAABB(ray, aabb, tMin_, tMax_);

    if (canMakePortal_) {
        ring_->SetColor({ 0.0f,0.0f,1.0f,1.0f });
        ringTransform_ = transform;
        Vector3 forward = YoshidaMath::GetForward(camera_->GetWorldMatrix());
        ringTransform_.translate -= forward * 0.125f;
    } else {
     /*   ring_->SetColor({ 1.0f,0.0f,0.0f,1.0f });
        ringTransform_ = { .scale = {1.0f,1.0f,1.0f},.rotate = {0.0f,0.0f,0.0f},.translate = {0.0f,0.0f,tMax_} };*/
    }

    return canMakePortal_;
}

void TimeCardWatch::MakeWorldMat()
{
    Matrix4x4 child = Function::MakeAffineMatrix(ringTransform_.scale, ringTransform_.rotate, ringTransform_.translate);
    child = Function::Multiply(child, camera_->GetWorldMatrix());
    ringMatWorld_ = child;
}

void TimeCardWatch::MakeChildMat()
{
    Matrix4x4 child = Function::MakeAffineMatrix(ringTransform_.scale, ringTransform_.rotate, ringTransform_.translate);
    //Matrix4x4 scaleMatrix = Function::MakeScaleMatrix(ringTransform_.scale);
    //Matrix4x4 translateMatrix = Function::MakeTranslateMatrix(ringTransform_.translate);
    //Matrix4x4 rotateMatrix = Function::Multiply(YoshidaMath::MakeRotateMatrix(ringTransform_.rotate), YoshidaMath::GetBillBordMatrix(camera_));
    //Matrix4x4 worldMatrix = Function::Multiply(Function::Multiply(scaleMatrix, rotateMatrix), translateMatrix);
    ringMatWorld_ = child;

}
