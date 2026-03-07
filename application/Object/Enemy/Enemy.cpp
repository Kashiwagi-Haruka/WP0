#define NOMINMAX
#include "Enemy.h"
#include "Camera.h"
#include "GameBase.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3d.h"
#include "Vector4.h"
#include <algorithm>

namespace {
const Vector4 kDamageInvincibleColor = {1.0f, 0.0f, 0.0f, 1.0f};
const Vector4 kDefaultColor = {1.0f, 1.0f, 1.0f, 1.0f};
const Vector4 kDeathColor = {1.0f, 0.2f, 0.2f, 1.0f};
} // namespace

Enemy::Enemy() {

	object_ = std::make_unique<Object3d>();
	enemyStun = std::make_unique<EnemyStun>();
}
void Enemy::Initialize(Camera* camera, Vector3 translates) {
	isAlive = true;
	isStun_ = false;
	HP = 3;
	stunTime = 0;
	attackTimer_ = 0.0f;
	damageInvincibleTimer_ = 0.0f;
	lastSkillDamageId_ = -1;
	isDying_ = false;
	deathTimer_ = 0.0f;

	object_->Initialize();
	object_->SetModel("Enemy");
	camera_ = camera;
	transform_ = {
	    .scale{1.0f, 1.0f, 1.0f},
        .rotate{0.0f, 0.0f, 0.0f},
        .translate = translates
    };
	object_->SetTransform(transform_);
	object_->SetCamera(camera_);
	object_->Update();

	enemyStun->Initialize();
	enemyAttack_ = std::make_unique<EnemyAttack>();
	enemyAttack_->Initialize(camera_);
	object_->SetColor(kDefaultColor);
}

void Enemy::Update(const Vector3& housePos, const Vector3& houseScale, const Vector3& playerPos, bool isPlayerAlive) {
	const float deltaTime = 1.0f / 60.0f;
	if (isDying_) {
		deathTimer_ += deltaTime;
		float progress = std::clamp(deathTimer_ / deathDuration_, 0.0f, 1.0f);
		transform_.rotate.z = deathTargetRotateZ_ * progress;
		object_->SetColor(kDeathColor);
		if (enemyAttack_) {
			enemyAttack_->Cancel();
			enemyAttack_->Update();
		}
		object_->SetCamera(camera_);
		object_->SetTransform(transform_);
		object_->Update();
		if (progress >= 1.0f) {
			isAlive = false;
			isDying_ = false;
		}
		return;
	}
	if (!enemyAttack_ || !enemyAttack_->IsAttacking()) {
		attackTimer_ += deltaTime;
	}
	if (damageInvincibleTimer_ > 0.0f) {
		damageInvincibleTimer_ -= deltaTime;
		if (damageInvincibleTimer_ < 0.0f) {
			damageInvincibleTimer_ = 0.0f;
		}
	}
	object_->SetColor(damageInvincibleTimer_ > 0.0f ? kDamageInvincibleColor : kDefaultColor);
	Vector3 targetPosition = housePos;
	if (isPlayerAlive) {
		Vector3 toPlayer = playerPos - transform_.translate;
		if (LengthSquared(toPlayer) <= playerChaseRange_ * playerChaseRange_) {
			targetPosition = playerPos;
		}
	}

	// 敵の更新処理
	if (!isStun_) {
		Vector3 toTarget = targetPosition - transform_.translate;
		toTarget.y = 0.0f;
		if (LengthSquared(toTarget) > 0.0001f) {
			Vector3 direction = Function::Normalize(toTarget);
			velocity_ = direction * maxSpeed_;
		} else {
			velocity_ = {0.0f, 0.0f, 0.0f};
		}
		if (enemyAttack_ && enemyAttack_->IsAttacking()) {
			velocity_ = {0.0f, 0.0f, 0.0f};
		}
	}

	if (isStun_) {
		velocity_ = {0.0f, 0.0f, 0.0f};
		stunTime++;
		enemyStun->SetCamera(camera_);
		enemyStun->SetTranslate(transform_.translate);
		enemyStun->Update();
		if (stunTime >= stunTimeMax) {
			isStun_ = false;
		}
	}
	transform_.translate += velocity_;
	object_->SetCamera(camera_);
	object_->SetTransform(transform_);
	object_->Update();

	// 攻撃開始条件
	bool inAttackRange = false;
	if (!isStun_) {
		Vector3 toTarget = targetPosition - transform_.translate;
		toTarget.y = 0.0f;
		Vector3 toHouse = housePos - transform_.translate;
		toHouse.y = 0.0f;
		float houseRadius = std::max({houseScale.x, houseScale.y, houseScale.z});
		float enemyRadius = std::max({transform_.scale.x, transform_.scale.y, transform_.scale.z});
		float houseAttackRange = attackRange_ + houseRadius + enemyRadius;
		inAttackRange = LengthSquared(toTarget) <= attackRange_ * attackRange_ || LengthSquared(toHouse) <= houseAttackRange * houseAttackRange;
	}
	if (!isStun_ && !IsAttacking() && inAttackRange && IsAttackReady()) {
		enemyAttack_->Start(transform_);
		ResetAttackTimer();
	}

	enemyAttack_->Update();

	if (HP <= 0 && !isDying_) {
		StartDeathAnimation();
	}
}
void Enemy::Stun() {
	if (isDying_) {
		return;
	}
	isStun_ = true;
	stunTime = 0;
	if (enemyAttack_) {
		enemyAttack_->Cancel();
	}
	if (HP <= 0 && !isDying_) {
		StartDeathAnimation();
	}
}

void Enemy::Draw() {
	// 敵の描画処理
	object_->Draw();

	if (!isDying_ && enemyAttack_) {
		enemyAttack_->Draw();
	}

	if (!isDying_ && isStun_) {
		enemyStun->Draw();
	}
}
void Enemy::StartDeathAnimation() {
	if (isDying_) {
		return;
	}
	isDying_ = true;
	deathTimer_ = 0.0f;
	isStun_ = false;
	velocity_ = {0.0f, 0.0f, 0.0f};
	if (enemyAttack_) {
		enemyAttack_->Cancel();
	}
	object_->SetColor(kDeathColor);
}
void Enemy::BulletCollision() {}

void Enemy::SetIsStun(bool IsStun) { isStun_ = IsStun; }

float Enemy::GetAttackHitSize() const {
	if (enemyAttack_) {
		return enemyAttack_->GetHitSize();
	}
	return attackHitSize_;
}

Vector3 Enemy::GetAttackPosition() const {
	if (enemyAttack_) {
		return enemyAttack_->GetPosition();
	}
	return transform_.translate;
}

bool Enemy::IsAttackHitActive() const { return enemyAttack_ && enemyAttack_->IsHitActive(); }

bool Enemy::ConsumeAttackHit() {
	if (!enemyAttack_) {
		return false;
	}
	return enemyAttack_->ConsumeHit();
}
