#define NOMINMAX
#include "Player.h"
#include "Camera.h"
#include "Function.h"
#include "GameBase.h"
#include "Model/ModelManager.h"
#include "Object/MapchipField.h"
#include <algorithm>
#include <numbers>
#include "Object3d/Object3dCommon.h"
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

Player::Player() {

	ModelManager::GetInstance()->LoadModel("Resources/3d","FallingEffect");
	ModelManager::GetInstance()->LoadModel("Resources/3d","playerSkillUp");
	ModelManager::GetInstance()->LoadModel("Resources/3d","playerSkillUnder");

	sword_ = std::make_unique<PlayerSword>();
	skill_ = std::make_unique<PlayerSkill>();

	attackSE = Audio::GetInstance()->SoundLoadFile("Resources/audio/SE/normalAttack.mp3");
	attackEndSE = Audio::GetInstance()->SoundLoadFile("Resources/audio/SE/endAttack.mp3");
	skillAttackSE = Audio::GetInstance()->SoundLoadFile("Resources/audio/SE/magic.mp3");
	models_ = std::make_unique<PlayerModels>();
	
}

Player::~Player() { Audio::GetInstance()->SoundUnload(&attackSE);
	Audio::GetInstance()->SoundUnload(&attackEndSE);
	Audio::GetInstance()->SoundUnload(&skillAttackSE);
}

void Player::Initialize(Camera* camera) {

	state_ = State::kIdle;
	velocity_ = {0.0f, 0.0f, 0.0f};
	transform_ = {
	    .scale{1.0f, 1.0f, 1.0f},
        .rotate{0.0f, 0.0f, 0.0f},
        .translate{-5.0f, 3.6f, -5.0f}
    };
	camera_ = camera;
	sword_->Initialize();
	sword_->SetCamera(camera_);
	skill_->Initialize();
	skill_->SetCamera(camera_);
	isAlive = true;
	parameters_ = SetInit();
	models_->SetCamera(camera_);
	models_->SetPlayerTransform(transform_);
	models_->Initialize();
	hp_ = parameters_.hpMax_;
	bulletVelocity_ = {0, 0, 0};
	isDash = false;
	isJump = false;
	isfalling = false;
	isLevelUP = false;
	rotateTimer = 0.1f;
	damageTrigger_ = false;

	// コンボ関連の初期化
	comboStep_ = 0;
	comboTimer_ = 0.0f;
	comboWindow_ = 0.8f; // コンボ受付時間（秒）
	isAttacking_ = false;
	canCombo_ = false;

	// 重撃・落下攻撃関連
	attackHoldTimer_ = 0.0f;
	heavyAttackThreshold_ = 0.3f; // 長押し判定時間（秒）
	isFallingAttack_ = false;

	isSkillAttack = false;
	isSpecialAttack = false;
}

void Player::Move() {

	// ★ 落下攻撃中は移動できない
	if (isFallingAttack_) {
		return;
	}

	// 入力方向を記録する変数
	Vector3 inputDirection = {0.0f, 0.0f, 0.0f};
	Vector3 inputAxis = {0.0f, 0.0f, 0.0f};
	bool hasInput = false;

	if (Input::GetInstance()->PushKey(DIK_A) || Input::GetInstance()->PushKey(DIK_D)) {

		if (!Input::GetInstance()->PushKey(DIK_D)) {

			if (Input::GetInstance()->PushKey(DIK_A)) {

				inputAxis.x = -1.0f;
				hasInput = true;
				if (!isAttacking_) {
					models_->SetStateM(PlayerModels::StateM::walk);
				}
			}
		}

		if (!Input::GetInstance()->PushKey(DIK_A)) {

			if (Input::GetInstance()->PushKey(DIK_D)) {
				inputAxis.x = 1.0f;
				hasInput = true;
				if (!isAttacking_) {
					models_->SetStateM(PlayerModels::StateM::walk);
				}
			}
		}
	}
	if (Input::GetInstance()->PushKey(DIK_W) || Input::GetInstance()->PushKey(DIK_S)) {

		if (!Input::GetInstance()->PushKey(DIK_W)) {

			if (Input::GetInstance()->PushKey(DIK_S)) {
				if (!isAttacking_) {
					models_->SetStateM(PlayerModels::StateM::walk);
				}
				inputAxis.z = -1.0f;
				hasInput = true;
			}
		}

		if (!Input::GetInstance()->PushKey(DIK_S)) {

			if (Input::GetInstance()->PushKey(DIK_W)) {
				inputAxis.z = 1.0f;
				hasInput = true;
				if (!isAttacking_) {
					models_->SetStateM(PlayerModels::StateM::walk);
				}
			}
		}
	}

	// 入力がある場合、その方向に向きを回転
	if (hasInput) {
		const float inputLength = std::sqrt(inputAxis.x * inputAxis.x + inputAxis.z * inputAxis.z);
		if (inputLength > 0.0f) {
			inputAxis.x /= inputLength;
			inputAxis.z /= inputLength;
		}

		const float cameraYaw = camera_ ? camera_->GetRotate().y : 0.0f;
		const Vector3 forward = {std::sinf(cameraYaw), 0.0f, std::cosf(cameraYaw)};
		const Vector3 right = {std::cosf(cameraYaw), 0.0f, -std::sinf(cameraYaw)};
		inputDirection = forward * inputAxis.z + right * inputAxis.x;

		const float accel = parameters_.accelationRate * (parameters_.SpeedUp + 1);
		float currentSpeed = velocity_.x * inputDirection.x + velocity_.z * inputDirection.z;
		if (currentSpeed < 0.0f) {
			currentSpeed = 0.0f;
		}
		const float nextSpeed = std::min(parameters_.accelationMax, currentSpeed + accel);
		velocity_.x = inputDirection.x * nextSpeed;
		velocity_.z = inputDirection.z * nextSpeed;
		// 入力方向から目標角度を計算
		float targetAngle = std::atan2(inputDirection.x, inputDirection.z);

		// 現在の角度と目標角度の差を計算
		float angleDiff = targetAngle - transform_.rotate.y;

		// 角度を-π〜πの範囲に正規化
		while (angleDiff > std::numbers::pi_v<float>) {
			angleDiff -= 2.0f * std::numbers::pi_v<float>;
		}
		while (angleDiff < -std::numbers::pi_v<float>) {
			angleDiff += 2.0f * std::numbers::pi_v<float>;
		}

		// 滑らかに回転
		transform_.rotate.y = Function::Lerp(transform_.rotate.y, transform_.rotate.y + angleDiff, rotateTimer);
	}

	if (!Input::GetInstance()->PushKey(DIK_A) && !Input::GetInstance()->PushKey(DIK_D) && !Input::GetInstance()->PushKey(DIK_W) && !Input::GetInstance()->PushKey(DIK_S)) {
		isDash = false;
		if (!isAttacking_&&!isSkillAttack) {
		models_->SetStateM(PlayerModels::StateM::idle);
		}
	} else {
		isDash = Input::GetInstance()->PushKey(DIK_LSHIFT) || Input::GetInstance()->PushKey(DIK_RSHIFT);
	}

	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		if (!isfalling && !isJump) {
			isJump = true;
		}
	}

	if (!Input::GetInstance()->PushKey(DIK_A) && !Input::GetInstance()->PushKey(DIK_D)) {
		velocity_.x *= (1.0f - parameters_.decelerationRate);
		if (velocity_.x > -0.01f && velocity_.x < 0.01f) {
			velocity_.x = 0.0f;
		}
	}
	if (!Input::GetInstance()->PushKey(DIK_W) && !Input::GetInstance()->PushKey(DIK_S)) {
		velocity_.z *= (1.0f - parameters_.decelerationRate);
		if (velocity_.z > -0.01f && velocity_.z < 0.01f) {
			velocity_.z = 0.0f;
		}
	}

	velocity_.x = std::clamp(velocity_.x, -parameters_.accelationMax, parameters_.accelationMax);
	velocity_.z = std::clamp(velocity_.z, -parameters_.accelationMax, parameters_.accelationMax);
	if (isDash) {
		velocity_.x *= parameters_.dashMagnification;
		velocity_.z *= parameters_.dashMagnification;
	}

	if (Input::GetInstance()->PushKey(DIK_A)) {
		bulletVelocity_.x = -1;
	}
	if (Input::GetInstance()->PushKey(DIK_D)) {
		bulletVelocity_.x = 1;
	}
}

void Player::Jump() {

	if (isJump) {
		velocity_.y = parameters_.jumpPower;

		if (jumpTimer >= parameters_.jumpTimerMax) {
			jumpTimer = 0.0f;
			isJump = false;
			isfalling = true;
		} else {
			jumpTimer += 0.1f * (1 / 60.0f);
		}
	}

	// ★ 落下攻撃を開始したら即座に落下状態に
	if (isFallingAttack_ && isJump) {
		isJump = false;
		isfalling = true;
		jumpTimer = 0.0f;
	}
}

void Player::Falling() {

	if (isfalling) {

		// ★ 落下攻撃中は急降下
		if (isFallingAttack_) {
			velocity_.y -= parameters_.gravity * 3.0f; // 通常の3倍の速度で落下
			models_->SetStateM(PlayerModels::StateM::fallingAttack);
		} else {
			velocity_.y -= parameters_.gravity;
			models_->SetStateM(PlayerModels::StateM::idle);
		}

		if (transform_.translate.y <= 3.6f) {
			transform_.translate.y = 3.6f;
			velocity_.y = 0.0f;
			isfalling = false;

			// ★ 落下攻撃が地面に着いたら終了
			if (isFallingAttack_) {
				isFallingAttack_ = false;
				isAttacking_ = false;
				models_->SetStateM(PlayerModels::StateM::idle);
				sword_->EndAttack(); // 攻撃を終了
			}

			usedAirAttack = false;
		}
	}
}

void Player::Attack() {

	// ★ 落下攻撃中は他の攻撃不可
	if (isFallingAttack_) {
		return;
	}
	if (isSkillAttack) {
		models_->SetStateM(PlayerModels::StateM::skillAttack);
		return;
	}
	if (isSpecialAttack) {
		models_->SetStateM(PlayerModels::StateM::idle);
		return;
	}

	// コンボタイマーの更新
	if (comboTimer_ > 0.0f) {
		comboTimer_ -= 1.0f / 60.0f;
		if (comboTimer_ <= 0.0f) {
			// タイムアウト: コンボリセット
			comboStep_ = 0;
			canCombo_ = false;
		}
	}

	// ===== 左クリック長押し判定 =====
	if (Input::GetInstance()->PushMouseButton(Input::MouseButton::kLeft) || Input::GetInstance()->PushButton(Input::PadButton::kButtonB)) {
		attackHoldTimer_ += 1.0f / 60.0f;
	}

	// ===== 攻撃入力の処理 =====
	if (Input::GetInstance()->TriggerMouseButton(Input::MouseButton::kLeft) || Input::GetInstance()->TriggerButton(Input::PadButton::kButtonB)) {

		// ★ 空中にいる場合は落下攻撃
		if (isfalling || isJump) {
			isFallingAttack_ = true;
			isAttacking_ = true;
			attackState_ = AttackState::kFallingAttack;
			sword_->StartAttack(5); // 5=落下攻撃

			// コンボリセット
			comboStep_ = 0;
			canCombo_ = false;
			comboTimer_ = 0.0f;
			attackHoldTimer_ = 0.0f;
			return;
		}

		// 地上での通常攻撃
		if (!isAttacking_ || canCombo_) {

			// 次のコンボ段階に進む
			comboStep_++;
			if (comboStep_ > 4) {
				comboStep_ = 1; // 4段階目の後は最初に戻る
			}

			// 攻撃状態の設定
			isAttacking_ = true;
			canCombo_ = false;          // 連打防止: 一旦コンボ不可に
			comboTimer_ = comboWindow_; // コンボ受付時間をリセット

			// 攻撃ステートの設定
			switch (comboStep_) {
			case 1:
				attackState_ = AttackState::kWeakAttack1;
				sword_->StartAttack(1); // 1段階目
				Audio::GetInstance()->SoundPlayWave(attackSE, false);
				models_->SetStateM(PlayerModels::StateM::attack1);
				break;
			case 2:
				attackState_ = AttackState::kWeakAttack2;
				sword_->StartAttack(2); // 2段階目
				Audio::GetInstance()->SoundPlayWave(attackSE, false);
				models_->SetStateM(PlayerModels::StateM::attack2);
				break;
			case 3:
				attackState_ = AttackState::kWeakAttack3;
				sword_->StartAttack(3); // 3段階目
				Audio::GetInstance()->SoundPlayWave(attackSE, false);
				models_->SetStateM(PlayerModels::StateM::attack3);
				break;
			case 4:
				attackState_ = AttackState::kWeakAttack4;
				sword_->StartAttack(4); // 4段階目（フィニッシュ）
				Audio::GetInstance()->SoundPlayWave(attackEndSE, false);
				models_->SetStateM(PlayerModels::StateM::attack4);
				break;
			}
		}
	}

		// ===== 左クリックを離したとき =====
	if (Input::GetInstance()->ReleaseMouseButton(Input::MouseButton::kLeft) || Input::GetInstance()->ReleaseButton(Input::PadButton::kButtonB)) {

		// ★ 長押ししていた場合は重撃に変更
		if (attackHoldTimer_ >= heavyAttackThreshold_ && isAttacking_) {
			attackState_ = AttackState::kStrongAttack;
			sword_->StartAttack(6); // 6=重撃
			models_->SetStateM(PlayerModels::StateM::attack4);

			// コンボリセット
			comboStep_ = 0;
			canCombo_ = false;
			comboTimer_ = 0.0f;
		}

		attackHoldTimer_ = 0.0f; // タイマーリセット
	}

	// 攻撃モーションが終了したか確認
	if (isAttacking_ && !isFallingAttack_ && models_->IsAttackAnimationFinished()) {
		isAttacking_ = false;
		canCombo_ = true; // 次のコンボ入力を受け付ける
		sword_->EndAttack();

		// 4段階目が終わったらコンボリセット
		if (comboStep_ >= 4) {
			comboStep_ = 0;
			canCombo_ = false;
			comboTimer_ = 0.0f;
		}

		// 重撃が終わったらリセット
		if (attackState_ == AttackState::kStrongAttack) {
			comboStep_ = 0;
			canCombo_ = false;
			comboTimer_ = 0.0f;
		}
	}
	if (Input::GetInstance()->TriggerKey(DIK_E) || Input::GetInstance()->TriggerButton(Input::PadButton::kButtonY)) {
		// スキル攻撃
		if (!isSkillAttack) {
			isSkillAttack = true;
			attackState_ = AttackState::kSkillAttack;
			skill_->StartAttack(transform_);
			Audio::GetInstance()->SoundPlayWave(skillAttackSE, false);
			if (parameters_.AllowUp > 0) {
				isSpecialAttack = true;
				attackState_ = AttackState::kSpecialAttack;
				skill_->StartSpecialAttack(transform_, parameters_.AllowUp);
			}
			// コンボリセット
			comboStep_ = 0;
			canCombo_ = false;
			comboTimer_ = 0.0f;
		}
	}
	if (Input::GetInstance()->TriggerKey(DIK_Q) || Input::GetInstance()->TriggerButton(Input::PadButton::kButtonX)) {
		//必殺技
		if (!isSpecialAttack) {
			isSpecialAttack = true;
			attackState_ = AttackState::kSpecialAttack;

			// コンボリセット
			comboStep_ = 0;
			canCombo_ = false;
			comboTimer_ = 0.0f;
		}

	}
}

void Player::Update() {

	Attack();
	Move();
	Jump();
	Falling();

	// --- レベルアップ処理 ---
	if (parameters_.Level < parameters_.MaxLevel) {

		if (parameters_.EXP >= parameters_.MaxEXP) {

			parameters_.EXP -= parameters_.MaxEXP;
			parameters_.Level++;

			isLevelUP = true;
		}
	} else {
		if (parameters_.EXP > parameters_.MaxEXP) {
			parameters_.EXP = parameters_.MaxEXP;
		}
	}


	transform_.translate += velocity_;
	{
		const float radius = movementLimitRadius_;
		const float dx = transform_.translate.x - movementLimitCenter_.x;
		const float dz = transform_.translate.z - movementLimitCenter_.z;
		const float distSquared = dx * dx + dz * dz;
		if (distSquared > radius * radius) {
			const float dist = std::sqrt(distSquared);
			const float scale = radius / dist;
			transform_.translate.x = movementLimitCenter_.x + dx * scale;
			transform_.translate.z = movementLimitCenter_.z + dz * scale;
		}
	}


	models_->SetCamera(camera_);
	models_->SetPlayerTransform(transform_);
	models_->Update();
	const auto swordJointMatrix = models_->GetJointWorldMatrix("剣");
	sword_->SetCamera(camera_);
	sword_->SetPlayerYaw(transform_.rotate.y);
	sword_->Update(transform_, swordJointMatrix);
	
	if (isSkillAttack) {
	skill_->SetCamera(camera_);
	skill_->Update();
		if (skill_->IsSkillEnd()) {
		isSkillAttack = false;
		}
	}

	if (isSpecialAttack) {
	
	
	}

#ifdef USE_IMGUI

	if (ImGui::Begin("Player")) {

		ImGui::DragInt("HP", &hp_);
		ImGui::DragFloat3("plPos", &transform_.translate.x);
		ImGui::DragFloat3("plRot", &transform_.rotate.x);
		ImGui::DragFloat("rotateSpeed", &rotateTimer, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("comboWindow", &comboWindow_, 0.01f, 0.1f, 2.0f);
		ImGui::DragFloat("heavyAttackThreshold", &heavyAttackThreshold_, 0.01f, 0.1f, 2.0f);
		ImGui::Text("LMB = FIRE , WASD = MOVE , SPACE = JUMP");
		ImGui::Text("isDash: %d", isDash);
		ImGui::Text("Combo Step: %d / 4", comboStep_);
		ImGui::Text("Combo Timer: %.2f", comboTimer_);
		ImGui::Text("Can Combo: %s", canCombo_ ? "YES" : "NO");
		ImGui::Text("Hold Timer: %.2f", attackHoldTimer_);
		ImGui::Text("Falling Attack: %s", isFallingAttack_ ? "YES" : "NO");
	}
	ImGui::End();

#endif //

	parameters_.hpMax_ = parameters_.hpMax_ * (1 + parameters_.HPUp);

	// 死
	if (hp_ <= 0) {
		isAlive = false;
	}

	//  --- ダメージ後の無敵時間 ---
	if (isInvincible_) {
		invincibleTimer_ -= 1.0f / 60.0f;
		if (invincibleTimer_ <= 0.0f) {
			isInvincible_ = false;
		}
	}
}

void Player::EXPMath() { parameters_.EXP += 15; }

void Player::Draw() {

	models_->Draw();
	Object3dCommon::GetInstance()->DrawCommon();
	if (isFallingAttack_) {

	}
	sword_->Draw();
	if (isSkillAttack) {
	skill_->Draw();
	}
	if (isSpecialAttack) {
	
	}
}