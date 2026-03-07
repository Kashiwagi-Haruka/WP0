#include "EnemyStun.h"
#include "Model/ModelManager.h"
#include "GameBase.h"

EnemyStun::EnemyStun() {
	ModelManager::GetInstance()->LoadModel("Resources/3d","EnemyStun");
	object_ = std::make_unique<Object3d>();
	object_->SetModel("EnemyStun");
}

void EnemyStun::SetCamera(Camera* camera) { camera_ = camera; }
void EnemyStun::SetTranslate(Vector3 translate) { transform_.translate = translate; };
void EnemyStun::Initialize() { 
	
	object_->SetCamera(camera_);
	object_->Initialize();
	object_->SetTransform(transform_);
	object_->Update();

}

void EnemyStun::Update() { 
	object_->Update(); 
}

void EnemyStun::Draw() { object_->Draw(); }