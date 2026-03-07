#pragma once
#include "Input.h"
#include "Object3d/Object3d.h"
#include "PlayerParameters.h"
#include "PlayerSword.h"
#include "PlayerSkill.h"
#include "Transform.h"
#include "Vector3.h"
#include <memory>
#include "Audio.h"
#include "PlayerModels.h"
class Camera;
class PlayerBullet;
class MapchipField;

class Player {

	enum class State {
		kIdle,
		kRunning,
		kJumping,
		kFalling,
		kAttacking

	};
	State state_;
	enum class AttackState {
		kNone,          // 攻撃していない
		kWeakAttack1,   // 弱攻撃1
		kWeakAttack2,   // 弱攻撃2
		kWeakAttack3,   // 弱攻撃3
		kWeakAttack4,   // 弱攻撃4
		kStrongAttack,  // 重撃
		kSkillAttack,   // スキル技
		kSpecialAttack, // 必殺技
		kFallingAttack  // 落下攻撃
	};
	AttackState attackState_;

	Parameters parameters_;
	std::unique_ptr<PlayerModels> models_;

	struct Select {};

	float jumpTimer = 0.0f;
	int hp_; // プレイヤーHP

	bool isAlive;
	bool isInvincible_ = false;
	float invincibleTimer_ = 0.0f;
	bool damageTrigger_ = false;


	// コンボ攻撃用
	int comboStep_ = 0;        // 現在のコンボ段階 (0〜4)
	float comboTimer_ = 0.0f;  // コンボ受付タイマー
	float comboWindow_ = 0.8f; // コンボ受付時間（秒）
	bool isAttacking_ = false; // 攻撃中フラグ
	bool canCombo_ = false;    // 次のコンボ入力可能フラグ

	// 重撃・落下攻撃用
	float attackHoldTimer_ = 0.0f;      // 長押し時間
	float heavyAttackThreshold_ = 0.3f; // 重撃判定時間（秒）
	bool isFallingAttack_ = false;      // 落下攻撃中フラグ

	//スキル攻撃用
	bool isSkillAttack = false;
	bool isSpecialAttack = false;

	bool isDash = false;
	bool isJump = false;
	bool isfalling = false;
	bool isAttack = false;
	Vector3 velocity_;
	Vector3 bulletVelocity_;

	Transform transform_;


	std::unique_ptr<PlayerSword> sword_;
	std::unique_ptr<PlayerSkill> skill_;


	Camera* camera_;

	MapchipField* map_ = nullptr;

	bool isSelect_;
	bool isLevelUP;
	float rotateTimer = 0.0f;

	bool usedAirAttack = false;

	//SE
	bool isAttackSE = false;
	bool isAttackEndSE = false;

	SoundData attackSE;
	SoundData attackEndSE;
	SoundData skillAttackSE;

	Vector3 movementLimitCenter_{0.0f,2.5f,0.0f};
	float movementLimitRadius_ = 50.0f;

public:
	Player();
	~Player();
	void Initialize(Camera* camera);
	void Move();
	void Attack();
	void Update();
	void Draw();
	void Jump();
	void Falling();
	PlayerSkill* GetSkill() { return skill_.get(); }


	void SetCamera(Camera* camera) { camera_ = camera; }
	void SetMap(MapchipField* map) { map_ = map; }
	Vector3 GetPosition() { return transform_.translate; }
	Vector3 GetVelocity() { return velocity_; }
	Vector3 GetBulletPosition();
	bool GetIsAlive() { return isAlive; }
	bool GetIsSkillAttack() { return isSkillAttack; }
	Vector3 GetSkillPosition() { return skill_->GetDamagePosition(); }
	Parameters GetParameters() { return parameters_; }
	void SetParameters(const Parameters& p) { parameters_ = p; }
	Vector3 GetRotate() { return transform_.rotate; }
	Vector3 GetScale() { return transform_.scale; }
	void Damage(int amount) {
		if (!isInvincible_) {
			hp_ -= amount;
			isInvincible_ = true;
			invincibleTimer_ = 1.0f; // 1秒無敵
			damageTrigger_ = true;
		}
	}
	bool ConsumeDamageTrigger() {
		const bool triggered = damageTrigger_;
		damageTrigger_ = false;
		return triggered;
	}
	bool GetSelect() { return isSelect_; };
	int GetHP() const { return hp_; }
	int GetHPMax() const { return parameters_.hpMax_; }
	Vector3 GetMovementLimitCenter() const { return movementLimitCenter_; }
	float GetMovementLimitRadius() const { return movementLimitRadius_; }
	void IsLevelUp(bool lv) { isLevelUP = lv; }
	bool GetLv() { return isLevelUP; }
	void EXPMath();
	PlayerSword* GetSword() { return sword_.get(); }
	int GetComboStep() const { return comboStep_; }           // コンボ段階取得用
	bool IsFallingAttack() const { return isFallingAttack_; } // 落下攻撃中か
};