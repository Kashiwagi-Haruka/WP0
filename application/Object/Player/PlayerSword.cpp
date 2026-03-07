#include "PlayerSword.h"
#include "Function.h"
#include "GameBase.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
PlayerSword::PlayerSword() {

	ModelManager::GetInstance()->LoadModel("Resources/3d", "playerSword");
	ModelManager::GetInstance()->LoadModel("Resources/3d", "debugBox");
}

void PlayerSword::Initialize() {
	swordObject_ = std::make_unique<Object3d>();
	swordObject_->Initialize();
	swordObject_->SetCamera(camera);
	swordObject_->SetModel("playerSword");
	swordTrail_ = std::make_unique<PlayerSwordTrail>();
	swordTrail_->Initialize();
	swordTrail_->SetCamera(camera);
#ifdef _DEBUG
	debugBox_ = std::make_unique<Object3d>();
	debugBox_->Initialize();
	debugBox_->SetCamera(camera);
	debugBox_->SetModel("debugBox");
	hitTransform_ = {
	    .scale{1, 1, 1},
        .rotate{0, 0, 0},
        .translate{0, 0, 0}
    };
#endif // _DEBUG
}

void PlayerSword::StartAttack(int comboStep) {
	isAttacking_ = true;
	currentComboStep_ = comboStep;
}

void PlayerSword::EndAttack() {
	isAttacking_ = false;
	if (swordTrail_) {
		swordTrail_->Clear();
	}
}

Vector3 PlayerSword::GetPosition() const { return hitTransform_.translate; }

void PlayerSword::Update(const Transform& playerTransform, const std::optional<Matrix4x4>& jointWorldMatrix) {

	const bool useJointAttachment = jointWorldMatrix.has_value();
	Transform swordTransform = playerTransform;
	if (useJointAttachment) {
		swordTransform = {
		    .scale{2.0f, 2.0f, 2.0f},
		    .rotate{0.0f, 0.0f, 0.0f},
		    .translate{0.0f, 0.0f, 0.0f},
		};
	} else {
		Vector3 backDir = {sinf(playerYaw_), 0.0f, cosf(playerYaw_)};
		swordTransform.translate = playerTransform.translate - backDir * distanceFromPlayer_;
	}
	Vector3 forwardDir = {sinf(playerYaw_), 0.0f, cosf(playerYaw_)};
	hitTransform_ = playerTransform;
	hitTransform_.scale = {GetHitSize(), GetHitSize(), GetHitSize()};
	hitTransform_.translate = playerTransform.translate + forwardDir * hitDistanceFromPlayer_;

	if (useJointAttachment) {
		const Matrix4x4 localMatrix = Function::MakeAffineMatrix(swordTransform.scale, swordTransform.rotate, swordTransform.translate);
		const Matrix4x4 worldMatrix = Function::Multiply(localMatrix, *jointWorldMatrix);
		swordObject_->SetWorldMatrix(worldMatrix);
		if (swordTrail_) {
			swordTrail_->SetCamera(camera);
			swordTrail_->Update(worldMatrix, isAttacking_);
		}
	} else {
		swordObject_->SetTransform(swordTransform);
		const Matrix4x4 worldMatrix = Function::MakeAffineMatrix(swordTransform.scale, swordTransform.rotate, swordTransform.translate);
		if (swordTrail_) {
			swordTrail_->SetCamera(camera);
			swordTrail_->Update(worldMatrix, isAttacking_);
		}
	}
	swordObject_->SetCamera(camera);
	swordObject_->Update();

#ifdef _DEBUG
	debugBox_->SetTransform(hitTransform_);
	debugBox_->SetCamera(camera);
	debugBox_->Update();
#endif // _DEBUG
}

void PlayerSword::Draw() {
	if (swordTrail_) {
		Object3dCommon::GetInstance()->DrawCommonEmissive();
		swordTrail_->Draw();
	}
	Object3dCommon::GetInstance()->DrawCommon();
	swordObject_->Draw();
#ifdef _DEBUG
	debugBox_->Draw();
#endif // _DEBUG
}