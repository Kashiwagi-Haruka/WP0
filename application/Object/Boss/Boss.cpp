#define NOMINMAX
#include "Boss.h"
#include "Camera.h"
#include "Function.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3d.h"
#include "Object3d/Object3dCommon.h"
#include "Vector4.h"
#include <algorithm>
#include <cmath>
#include <numbers>
#include "DirectXCommon.h"

namespace {
const Vector4 kDamageInvincibleColor = {1.0f, 0.25f, 0.25f, 1.0f};
const Vector4 kChargeColor = {1.0f, 0.95f, 0.6f, 1.0f};
const Vector4 kDefaultColor = {1.0f, 1.0f, 1.0f, 1.0f};
} // namespace

Boss::Boss() {
	object_ = std::make_unique<Object3d>();
#ifdef _DEBUG
	ModelManager::GetInstance()->LoadModel("Resources/3d", "debugBox");
#endif // _DEBUG
}

void Boss::Initialize(Camera* camera, const Vector3& position) {
	hp_ = maxHp_;
	isAlive_ = true;
	damageInvincibleTimer_ = 0.0f;
	lastSkillDamageId_ = -1;
	animationTimer_ = 0.0f;
	animationTime_ = 0.0f;
	appearTimer_ = 0.0f;
	attackTimer_ = 0.0f;
	attackActiveTimer_ = 0.0f;
	attackHitConsumed_ = false;
	chargeTimer_ = 0.0f;
	attackAnimationTimer_ = 0.0f;
	chargeSpinTimer_ = 0.0f;
	isChargeAttack_ = false;
	actionState_ = ActionState::Idle;
	animationLoop_ = true;
	camera_ = camera;
	object_->Initialize();
	if (!ModelManager::GetInstance()->FindModel("WaterBoss")) {
		ModelManager::GetInstance()->LoadGltfModel("Resources/3d", "WaterBoss");
	}
	object_->SetModel("WaterBoss");

	animationClips_ = Animation::LoadAnimationClips("Resources/3d", "WaterBoss");
	if (!animationClips_.empty()) {
		currentAnimationIndex_ = 0;
		for (size_t i = 0; i < animationClips_.size(); ++i) {
			if (animationClips_[i].name == "Walk") {
				currentAnimationIndex_ = i;
				break;
			}
		}
		animationLoop_ = true;
		object_->SetAnimation(&animationClips_[currentAnimationIndex_], animationLoop_);
		object_->ResetAnimationTime();
		for (const auto& clip : animationClips_) {
			if (clip.name == "Charge" && clip.duration > 0.0f) {
				chargeDuration_ = clip.duration;
			}
			if (clip.name == "Attack" && clip.duration > 0.0f) {
				attackAnimationDuration_ = clip.duration;
			}
		}
	}
	if (Model* bossModel = ModelManager::GetInstance()->FindModel("WaterBoss")) {
		skeleton_ = std::make_unique<Skeleton>(Skeleton().Create(bossModel->GetModelData().rootnode));
		skinCluster_ = CreateSkinCluster(*skeleton_, *bossModel);
		if (!skinCluster_.mappedPalette.empty()) {
			object_->SetSkinCluster(&skinCluster_);
		}
	}
	basePosition_ = position;
	baseScale_ = {1.0f, 1.0f, 1.0f};
	transform_ = {
	    .scale{1.0f, 1.0f, 1.0f},
	    .rotate{0.0f, 0.0f, 0.0f},
	    .translate = position,
	};
	object_->SetTransform(transform_);
	object_->SetCamera(camera_);
	object_->SetColor(kDefaultColor);
	object_->Update();
#ifdef _DEBUG
	debugBox_ = std::make_unique<Object3d>();
	debugBox_->Initialize();
	debugBox_->SetCamera(camera_);
	debugBox_->SetModel("debugBox");
	debugBox_->SetColor({0.25f, 0.9f, 0.3f, 0.4f});
	debugBox_->SetTransform(transform_);
	debugBox_->Update();
#endif // _DEBUG
}

void Boss::Update(const Vector3& housePos, const Vector3& playerPos, bool isPlayerAlive) {
	if (!isAlive_) {
		return;
	}
	const float deltaTime = 1.0f / 60.0f;
	const float animationDeltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
	animationTimer_ += deltaTime;
	if (attackActiveTimer_ > 0.0f) {
		attackActiveTimer_ -= deltaTime;
		if (attackActiveTimer_ <= 0.0f) {
			attackActiveTimer_ = 0.0f;
			attackHitConsumed_ = false;
		}
	} else if (actionState_ == ActionState::Idle) {
		attackTimer_ += deltaTime;
	}
	if (actionState_ == ActionState::Charging) {
		chargeTimer_ -= deltaTime;
		if (chargeTimer_ <= 0.0f) {
			actionState_ = ActionState::Attacking;
			attackActiveTimer_ = attackActiveDuration_;
			attackHitConsumed_ = false;
			attackAnimationTimer_ = attackAnimationDuration_ > 0.0f ? attackAnimationDuration_ : attackActiveDuration_;
			chargeSpinTimer_ = chargeSpinDuration_;
			baseYaw_ = transform_.rotate.y;
		}
	} else if (actionState_ == ActionState::Attacking) {
		attackAnimationTimer_ -= deltaTime;
		if (attackAnimationTimer_ <= 0.0f) {
			actionState_ = ActionState::Idle;
		}
	}
	if (damageInvincibleTimer_ > 0.0f) {
		damageInvincibleTimer_ -= deltaTime;
		if (damageInvincibleTimer_ < 0.0f) {
			damageInvincibleTimer_ = 0.0f;
		}
	}

	if (appearTimer_ < appearDuration_) {
		appearTimer_ += deltaTime;
	}


	Vector3 targetPosition = housePos;
	if (isPlayerAlive) {
		Vector3 toPlayer = playerPos - basePosition_;
		if (LengthSquared(toPlayer) <= playerChaseRange_ * playerChaseRange_) {
			targetPosition = playerPos;
		}
	}
	Vector3 toTarget = targetPosition - basePosition_;
	toTarget.y = 0.0f;
	if (LengthSquared(toTarget) > 0.0001f) {
		Vector3 direction = Function::Normalize(toTarget);
		velocity_ = direction * maxSpeed_;
	} else {
		velocity_ = {0.0f, 0.0f, 0.0f};
	}
	if (actionState_ == ActionState::Charging) {
		velocity_ = {0.0f, 0.0f, 0.0f};
	}
	basePosition_ += velocity_;

bool inAttackRange = false;
	Vector3 toHouse = housePos - basePosition_;
	toHouse.y = 0.0f;
	if (LengthSquared(toHouse) <= attackRange_ * attackRange_) {
		inAttackRange = true;
	}
	if (!inAttackRange && isPlayerAlive) {
		Vector3 toPlayer = playerPos - basePosition_;
		toPlayer.y = 0.0f;
		inAttackRange = LengthSquared(toPlayer) <= attackRange_ * attackRange_;
	}
	if (actionState_ == ActionState::Idle && attackTimer_ >= attackCooldown_ && inAttackRange) {
		actionState_ = ActionState::Charging;
		chargeTimer_ = chargeDuration_;
		attackTimer_ = 0.0f;
		isChargeAttack_ = true;
	}
	if (actionState_ == ActionState::Idle) {
		isChargeAttack_ = false;
	}
	float appearT = std::clamp(appearTimer_ / appearDuration_, 0.0f, 1.0f);
	float smoothT = appearT * appearT * (3.0f - 2.0f * appearT);
	float scalePulse = 1.0f + std::sin(animationTimer_ * std::numbers::pi_v<float>) * 0.05f;
	transform_.scale = baseScale_ * (smoothT * scalePulse);

	float bob = std::sin(animationTimer_ * std::numbers::pi_v<float> * 2.0f) * 0.5f;
	transform_.translate = basePosition_;
	transform_.translate.y += bob;
	transform_.translate.x += std::sin(animationTimer_ * std::numbers::pi_v<float> * 0.5f) * 0.3f;
	if (chargeSpinTimer_ > 0.0f) {
		chargeSpinTimer_ = std::max(0.0f, chargeSpinTimer_ - deltaTime);
		transform_.rotate.y += deltaTime * chargeSpinSpeed_;
	} else {
		transform_.rotate.y = baseYaw_;
	}
	transform_.rotate.z = std::sin(animationTimer_ * std::numbers::pi_v<float>) * 0.05f;

	if (!animationClips_.empty()) {
		const char* desiredAnimationName = "Walk";
		bool loopAnimation = true;
		switch (actionState_) {
		case ActionState::Idle:
			desiredAnimationName = "Walk";
			loopAnimation = true;
			break;
		case ActionState::Charging:
			desiredAnimationName = "Charge";
			loopAnimation = false;
			break;
		case ActionState::Attacking:
			desiredAnimationName = "Attack";
			loopAnimation = false;
			break;
		default:
			break;
		}

		size_t desiredIndex = currentAnimationIndex_;
		for (size_t i = 0; i < animationClips_.size(); ++i) {
			if (animationClips_[i].name == desiredAnimationName) {
				desiredIndex = i;
				break;
			}
		}
		if (desiredIndex != currentAnimationIndex_ || loopAnimation != animationLoop_) {
			currentAnimationIndex_ = desiredIndex;
			animationLoop_ = loopAnimation;
			animationTime_ = 0.0f;
			object_->SetAnimation(&animationClips_[currentAnimationIndex_], animationLoop_);
		}
	}

	if (skeleton_ && !animationClips_.empty()) {
		const auto& currentAnimation = animationClips_[currentAnimationIndex_];
		animationTime_ = Animation::AdvanceTime(currentAnimation, animationTime_, animationDeltaTime, animationLoop_);
		skeleton_->ApplyAnimation(currentAnimation, animationTime_);
		skeleton_->Update();
		if (!skinCluster_.mappedPalette.empty()) {
			UpdateSkinCluster(skinCluster_, *skeleton_);
		}
	}

	object_->SetCamera(camera_);
	object_->SetTransform(transform_);
	Vector4 nextColor = kDefaultColor;
	if (actionState_ == ActionState::Charging) {
		nextColor = kChargeColor;
	}
	if (damageInvincibleTimer_ > 0.0f) {
		nextColor = kDamageInvincibleColor;
	}
	object_->SetColor(nextColor);
	object_->Update();
	if (skeleton_) {
		skeleton_->SetObjectMatrix(object_->GetWorldMatrix());
	}
#ifdef _DEBUG
	skeleton_->DrawBones(camera_, {1, 1, 1, 1}, {1, 1, 0, 1});
	if (debugBox_) {
		debugBox_->SetCamera(camera_);
		debugBox_->SetTransform(transform_);
		debugBox_->Update();
	}
#endif // _DEBUG
}
Vector3 Boss::GetAttackPosition() const {
	Vector3 forwardDir = {std::sinf(transform_.rotate.y), 0.0f, std::cosf(transform_.rotate.y)};
	if (isChargeAttack_) {
		return transform_.translate + forwardDir * (attackHitForwardOffset_ * 0.6f);
	}
	return transform_.translate + forwardDir * attackHitForwardOffset_;
}

void Boss::Draw() {
	if (!isAlive_) {
		return;
	}
	if (skinCluster_.mappedPalette.empty()) {
		Object3dCommon::GetInstance()->DrawCommon();
	} else {
		Object3dCommon::GetInstance()->DrawCommonSkinning();
	}
	object_->Draw();
#ifdef _DEBUG
	if (debugBox_) {
		Object3dCommon::GetInstance()->DrawCommonNoCullDepth();
		debugBox_->Draw();
	}
#endif // _DEBUG
}

void Boss::SetHPSubtract(int hp) {
	hp_ -= hp;
	if (hp_ <= 0) {
		isAlive_ = false;
	}
}
bool Boss::ConsumeAttackHit() {
	if (attackHitConsumed_ || attackActiveTimer_ <= 0.0f) {
		return false;
	}
	attackHitConsumed_ = true;
	return true;
}