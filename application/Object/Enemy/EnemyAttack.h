#pragma once
#include "Object3d/Object3d.h"
#include "Transform.h"
#include "Vector3.h"
#include <memory>

class Camera;

class EnemyAttack {
public:
	enum class State { Idle, SwingDown, End };

public:
	EnemyAttack();
	void Initialize(Camera* camera);
	void Start(const Transform& enemyTransform);
	void Update();
	void Draw();
	void Cancel();
	bool ConsumeHit();

	bool IsAttacking() const { return state_ != State::Idle; }
	bool IsHitActive() const { return hitActive_; }

	// 当たり判定用
	Vector3 GetPosition() const { return transform_.translate; }
	float GetHitSize() const { return hitSize_; }

private:
	std::unique_ptr<Object3d> object_;
	Transform transform_;

	Camera* camera_ = nullptr;

	State state_ = State::Idle;

	float timer_ = 0.0f;
	float swingTime_ = 0.3f; // 振り下ろし時間
	float hitSize_ = 1.2f;
	bool hitActive_ = false;
	bool hasDealtDamage_ = false;
};