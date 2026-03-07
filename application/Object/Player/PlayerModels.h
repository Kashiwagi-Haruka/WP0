#pragma once
#include "Camera.h"
#include "Transform.h"
#include <memory>
#include <optional>
#include <vector>
#include "Function.h"
#include "Object/Character/Sizuku/Sizuku.h"
class PlayerModels {

public:
	enum StateM {
		idle,
		walk,
		attack1,
		attack2,
		attack3,
		attack4,
		fallingAttack,
		skillAttack,
		damage,

	};

private:
	StateM state_;
	Camera* camera_;
	Transform player_;
	bool animationFinished_;
	std::unique_ptr<Sizuku> sizuku_;

public:
	PlayerModels();
	~PlayerModels();
	void SetCamera(Camera* camera) { camera_ = camera; };
	void SetPlayerTransform(Transform player) { player_ = player; };
	void SetStateM(StateM state) { state_ = state; }
	void Initialize();
	void Update();
	void Draw();
	std::optional<Matrix4x4> GetJointWorldMatrix(const std::string& jointName) const;
	bool IsAttackAnimationFinished() const { return sizuku_ ? sizuku_->IsAnimationFinished() : false; }
};