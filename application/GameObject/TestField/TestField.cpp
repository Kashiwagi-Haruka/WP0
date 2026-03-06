#include "TestField.h"
#include"Function.h"
#include"Light/SpotLight.h"


TestField::TestField()
{
    plane_ = std::make_unique<Primitive>();
    AABB aabb = { .min = {-50.0f,-1.0f,-50.0f},.max = {50.0f,0.0f,50.0f} };
    SetAABB(aabb);
    SetCollisionAttribute(kCollisionFloor);
    SetCollisionMask(kCollisionPlayer | kCollisionPortal);
}

void TestField::Initialize()
{
    transform_ = { .scale = {100.0f,100.0f,100.0f},.rotate = {Function::kPi * 0.5f,0.0f,0.0f},.translate = {0.0f,0.0f,0.0f} };
    plane_->Initialize(Primitive::Plane, "Resources/TD3_3102/2d/floor.png");
    plane_->SetTransform(transform_);
    plane_->SetColor({ 1.0f,1.0f,1.0f,1.0f });
}

void TestField::Update()
{
    plane_->SetTransform(transform_);
    plane_->SetUvTransform({ 100.0f,100.0f,100.0f }, {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f});
    plane_->Update();
}

void TestField::Draw()
{
    plane_->UpdateCameraMatrices();
    plane_->Draw();
}

void TestField::SetCamera(Camera* camera)
{
    plane_->SetCamera(camera);
}

void TestField::OnCollision(Collider* collider)
{
    if (collider->GetCollisionAttribute() == kCollisionPlayer) {

    }
}

Vector3 TestField::GetWorldPosition() const
{
    return transform_.translate;
}
