#include "SkyDome.h"
#include "Camera.h"
#include "GameBase.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"

SkyDome::SkyDome() {
	ModelManager::GetInstance()->LoadModel("Resources/3d", "skyDome");
	skyDomeObject_ = std::make_unique<Object3d>();
}

void SkyDome::Initialize(Camera* camera) {

	skyDomeObject_->Initialize();
	skyDomeObject_->SetModel("skyDome");
	camera_ = camera;
	skyDomeObject_->SetCamera(camera_);
	transform_.scale = {100.0f, 100.0f, 100.0f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = {0.0f, 2.50f, 0.0f};
	skyDomeObject_->SetTransform(transform_);
	skyDomeObject_->Update();
}
void SkyDome::Update() {
	skyDomeObject_->SetCamera(camera_);
	skyDomeObject_->SetEnableLighting(false);
	skyDomeObject_->SetShininess(0.0f);
	skyDomeObject_->Update();
}
void SkyDome::Draw() { 
	Object3dCommon::GetInstance()->DrawCommon();
	
	skyDomeObject_->Draw(); 
}