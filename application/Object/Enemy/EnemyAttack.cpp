#define NOMINMAX
#include "EnemyAttack.h"
#include "Camera.h"
#include "GameBase.h"
#include "Model/ModelManager.h"

EnemyAttack::EnemyAttack() {
	ModelManager::GetInstance()->LoadModel("Resources/3d","EnemyAttack"); // 鎌モデル
	object_ = std::make_unique<Object3d>();
}

void EnemyAttack::Initialize(Camera* camera) {
	camera_ = camera;
	object_->Initialize();
	object_->SetModel("EnemyAttack");
	object_->SetCamera(camera_);
}

void EnemyAttack::Start(const Transform& enemyTransform) {
	state_ = State::SwingDown;
	timer_ = 0.0f;
	hitActive_ = true;
	hasDealtDamage_ = false;
	// 敵の少し前＆上に出す
	transform_ = enemyTransform;
	transform_.translate.y += 1.0f;
	transform_.translate.x -= 0.5f;

	// 初期は持ち上げた角度
	transform_.rotate.z = -1.2f;

	object_->SetTransform(transform_);
}

void EnemyAttack::Update() {
	if (state_ == State::Idle)
		return;

	timer_ += 1.0f / 60.0f;
	float t = std::min(timer_ / swingTime_, 1.0f);

	// 鎌を振り下ろす回転
	transform_.rotate.z = -1.2f + 2.4f * t;

	object_->SetTransform(transform_);
	object_->Update();

	if (t >= 1.0f) {
		state_ = State::End;
		hitActive_ = false;
	}

	if (state_ == State::End) {
		state_ = State::Idle;
	}
}
void EnemyAttack::Cancel() {
	state_ = State::Idle;
	timer_ = 0.0f;
	hitActive_ = false;
	hasDealtDamage_ = false;
}
void EnemyAttack::Draw() {
	if (state_ != State::Idle) {
		object_->Draw();
	}
}

bool EnemyAttack::ConsumeHit() {
	if (!hitActive_ || hasDealtDamage_) {
		return false;
	}
	hasDealtDamage_ = true;
	return true;
}