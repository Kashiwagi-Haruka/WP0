#include "CharacterDisplay.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include <numbers>
#include <imgui.h>
#include "Input.h"
void CharacterDisplay::Initialize() {
	if (!camera_) {
	camera_ = std::make_unique<Camera>();
	}
	camera_->SetTransform(cameraTransform_);
	camera_->Update();

	if (!sizukuObject_) {
	sizukuObject_ = std::make_unique<Sizuku>();
	sizukuObject_->Initialize();
	}
	
	sizukuObject_->SetCamera(camera_.get());
	sizukuObject_->SetAnimation("InitIdle");
	sizukuObject_->SetTransform(characterTransform_);
	sizukuObject_->Update();

	if (!skyDome_) {
	skyDome_ = std::make_unique<CharacterDisplaySkyDome>();
		skyDome_->Initialize(camera_.get());
	}
	
}

void CharacterDisplay::Update() {
	if (!isActive_ || !sizukuObject_ || !camera_) {
		Input::GetInstance()->SetIsCursorStability(true);
		Input::GetInstance()->SetIsCursorVisible(false);
		return;
	}
	Object3dCommon::GetInstance()->SetDirectionalLight(directionalLight);
	Input::GetInstance()->SetIsCursorStability(false);
	Input::GetInstance()->SetIsCursorVisible(true);
	camera_->SetTransform(cameraTransform_);
	camera_->Update();
	skyDome_->Update();
#ifdef USE_IMGUI
	if (ImGui::Begin("CharacterDisplay")) {
		if (ImGui::TreeNode("Character Transform")) {
			ImGui::DragFloat3("Character Scale", &characterTransform_.scale.x, 0.01f);
			ImGui::DragFloat3("Character Rotate", &characterTransform_.rotate.x, 0.01f);
			ImGui::DragFloat3("Character Translate", &characterTransform_.translate.x, 0.01f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Display Camera Transform")) {
			ImGui::DragFloat3("Camera Scale", &cameraTransform_.scale.x, 0.01f);
			ImGui::DragFloat3("Camera Rotate", &cameraTransform_.rotate.x, 0.01f);
			ImGui::DragFloat3("Camera Translate", &cameraTransform_.translate.x, 0.01f);
			ImGui::TreePop();
		}
	}
	ImGui::End();
#endif // USE_IMGUI
	sizukuObject_->SetCamera(camera_.get());
	sizukuObject_->SetTransform(characterTransform_);
	sizukuObject_->Update();
}

void CharacterDisplay::Draw() {
	if (!isActive_ || !sizukuObject_) {
		return;
	}
	Object3dCommon::GetInstance()->DrawCommon();
	skyDome_->Draw();
	Object3dCommon::GetInstance()->DrawCommonSkinningToon();
	sizukuObject_->Draw();
}