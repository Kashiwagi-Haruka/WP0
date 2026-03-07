#include "PlayerModels.h"
#include "Function.h"
#include "GameBase.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include "imgui.h"
#include <cmath>
#include <numbers>

PlayerModels::PlayerModels() : state_(StateM::idle) {};
PlayerModels::~PlayerModels() {};

void PlayerModels::Initialize() { 
	sizuku_ = std::make_unique<Sizuku>();
	sizuku_->Initialize();
}

void PlayerModels::Update() {
	sizuku_->SetTransform(player_);
	switch (state_) {
	case PlayerModels::idle:
		sizuku_->SetAnimation("Idle");
		break;
	case PlayerModels::walk:
		sizuku_->SetAnimation("Walk");
		break;
	case PlayerModels::attack1:
		sizuku_->SetAnimation("Attack1");
		break;
	case PlayerModels::attack2:
		sizuku_->SetAnimation("Attack2");
		break;
	case PlayerModels::attack3:
		sizuku_->SetAnimation("Attack3");
		break;
	case PlayerModels::attack4:
		sizuku_->SetAnimation("Attack4");
		break;
	case PlayerModels::fallingAttack:
		sizuku_->SetAnimation("FallAttack");
		break;
	case PlayerModels::skillAttack:
		sizuku_->SetAnimation("SkillAttack");
		break;
	case PlayerModels::damage:
		sizuku_->SetAnimation("damage");
		break;
	default:
		break;
	}
	
	sizuku_->SetCamera(camera_);

	sizuku_->Update();

#ifdef USE_IMGUI
	if (!ImGui::Begin("Player Parts Adjust")) {
		ImGui::End();
		return;
	}

	if (ImGui::CollapsingHeader("Player Root")) {
		ImGui::DragFloat3("Player Pos", &player_.translate.x, 0.01f);
		ImGui::DragFloat3("Player Rot", &player_.rotate.x, 0.01f);
		ImGui::DragFloat3("Player Scale", &player_.scale.x, 0.01f);
	}

	ImGui::End();
#endif
}

void PlayerModels::Draw() {


	Object3dCommon::GetInstance()->DrawCommonSkinningToon();
	sizuku_->Draw();

}
std::optional<Matrix4x4> PlayerModels::GetJointWorldMatrix(const std::string& jointName) const {

	return sizuku_->GetJointWorldMatrix(jointName);
}