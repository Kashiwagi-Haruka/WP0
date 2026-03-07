#pragma once
#include "Camera.h"
#include "Object3d/Object3d.h"
#include "ParticleEmitter.h"
#include "Transform.h"
#include <memory>
#include <vector>
class PlayerSkill {

private:
	std::unique_ptr<Object3d> debugBox_;
	std::unique_ptr<Object3d> debugDamageBox1_;
	std::unique_ptr<Object3d> debugDamageBox2_;
	std::unique_ptr<Object3d> skillUpObject_;
	std::unique_ptr<Object3d> skillUnderObject_;
	std::unique_ptr<ParticleEmitter> skillEmitter_;
	Transform particle_;
	Transform transform_;
	Transform damageTransform1_;
	Transform damageTransform2_;
	Transform specialTransform_;
	Camera* camera_ = nullptr;

	float downstartposY = 5;

	bool isSkillEnd = false;
	bool isSpecialEnd_ = true;
	int skillTime = 0;
	int skillTimeMax = 60;
	float upTime = 0;
	float middleTime = 0;
	float downTime = 0;
	float damageTime = 0;
	float endTime = 0;
	int skillDamageId_ = 0;
	std::unique_ptr<std::vector<Object3d>> iceFlowers_;
	std::vector<Transform> iceFlowerTransforms_;
	std::unique_ptr<Object3d> specialDebugBox_;
	float specialRadius_ = 3.0f;
	float specialFallSpeed_ = 0.25f;
	float specialStartHeight_ = 6.0f;
	int specialTime_ = 0;
	int specialTimeMax_ = 60;

	void EnsureIceFlowerCount(int count);

	enum State {
		up,
		middle,
		down,
		damage,
	};
	State state;

public:
	PlayerSkill();
	void Initialize();
	void Update();
	void Draw();
	void SetCamera(Camera* camera) { camera_ = camera; }
	void StartAttack(const Transform& playerTransform);
	void StartSpecialAttack(const Transform& playerTransform, int iceCount);
	void UpdateSpecialAttack(const Transform& playerTransform);
	void DrawSpecialAttack();
	bool IsSkillEnd() { return isSkillEnd; }
	bool IsDamaging() const { return state == State::damage && !isSkillEnd; }
	bool IsSpecialEnd() const { return isSpecialEnd_; }
	bool IsSpecialDamaging() const { return !isSpecialEnd_; }
	Vector3 GetDamagePosition() const { return damageTransform2_.translate; }
	Vector3 GetDamageScale() const { return damageTransform2_.scale; }
	const std::vector<Transform>& GetSpecialIceFlowerTransforms() const { return iceFlowerTransforms_; }
	int GetSkillDamageId() const { return skillDamageId_; }
};