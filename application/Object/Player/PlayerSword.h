#pragma once
#include "Camera.h"
#include "Matrix4x4.h"
#include "Object3d/Object3d.h"
#include "Transform.h"
#include "playerSwordTrail.h"
#include <memory>
#include <optional>

class PlayerSword {

	std::unique_ptr<Object3d> swordObject_;
	std::unique_ptr<PlayerSwordTrail> swordTrail_;
#ifdef _DEBUG
	std::unique_ptr<Object3d> debugBox_;
#endif // _DEBUG
	Camera* camera = nullptr;

	bool isAttacking_ = false;
	int currentComboStep_ = 0;           // 現在のコンボ段階
	float distanceFromPlayer_ = -1.5f;   // プレイヤーからの距離
	float hitDistanceFromPlayer_ = 1.5f; // 当たり判定の距離
	float playerYaw_ = 0.0f;             // プレイヤーの向き（ヨー角）
	Transform hitTransform_{};

public:
	PlayerSword();

	void Initialize();
	void Update(const Transform& playerTransform, const std::optional<Matrix4x4>& jointWorldMatrix);
	void Draw();

	void StartAttack(int comboStep = 1); // コンボ段階を受け取る (1-4:通常攻撃, 5:落下攻撃, 6:重撃)
	void EndAttack();                    // 攻撃を強制終了
	bool IsAttacking() const { return isAttacking_; }

	Vector3 GetPosition() const;
	float GetHitSize() const { return 2.2f; }
	int GetComboStep() const { return currentComboStep_; }

	void SetCamera(Camera* cam) { camera = cam; }
	void SetPlayerYaw(float yaw) { playerYaw_ = yaw; }
};