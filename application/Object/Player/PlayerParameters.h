#pragma once

struct Parameters{

	float accelationRate = 0.005f;       // 移動時の加速量（フレームごとに加速する値）
	float accelationMax = 0.05f;       // 移動の最大速度
	float decelerationRate = 0.15f;    // キーを離した時の減速率
	float jumpPower = 0.70f;            // ジャンプの初速（上方向の初速）
	float jumpDuration = 0.5f;         // ジャンプを維持できる入力時間（未使用っぽいが意味はこれ）
	float jumpTimerMax = 0.01f;        // ジャンプ中状態を継続できる最大タイマー（非常に短く設定されている）
	float gravity = 0.98f / 10.0f/2.0f;     // 重力（毎フレーム下に加える値）
	float bulletRadius = 0.01f;        // 弾の当たり判定用の半径（小さい円）
	int hpMax_ = 30;                // プレイヤーの最大HP
	float dashMagnification = 2.0f;    // ダッシュ時の速度倍率（通常速度に掛ける）
	

	int HPUp=0;
	int AttuckUp=0;
	int SpeedUp=0;
	int AllowUp=0;

	int MaxLevel=10;
	int MaxEXP=200;
	int Level=0;
	int EXP=0;

	int EP = 0;
	int EPMax = 100;

	int weakAttack1Damage = 1;
	int weakAttack2Damage = 1;
	int weakAttack3Damage = 2;
	int weakAttack4Damage = 2;
	int strongAttackDamage = 3;
	int skillAttackDamage = 5;
	int specialAttackDamage = 10;
	int fallingAttackDamage = 4;

};

Parameters SetInit();