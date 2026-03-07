#include "PlayerParameters.h"
Parameters SetInit() {

	Parameters p;

	p.accelationRate = 0.1f;          // 移動時の加速量（フレームごとに加速する値）
	p.accelationMax = 0.25f;          // 横移動の最大速度
	p.decelerationRate = 0.15f;       // キーを離した時の減速率
	p.jumpPower = 0.70f;              // ジャンプの初速（上方向の初速）
	p.jumpDuration = 0.5f;            // ジャンプを維持できる入力時間（未使用っぽいが意味はこれ）
	p.jumpTimerMax = 0.01f;           // ジャンプ中状態を継続できる最大タイマー（非常に短く設定されている）
	p.gravity = 0.98f / 10.0f / 2.0f; // 重力（毎フレーム下に加える値）
	p.bulletRadius = 0.01f;           // 弾の当たり判定用の半径（小さい円）
	p.hpMax_ = 40;                 // プレイヤーの最大HP
	p.dashMagnification = 2.0f;       // ダッシュ時の速度倍率（通常速度に掛ける）
	

	p.HPUp = 0;
	p.AttuckUp = 0;
	p.SpeedUp = 0;
	p.AllowUp = 0;

	p.MaxLevel = 10;
	p.MaxEXP = 200;
	p.Level = 0;
	p.EXP = 0;

	return p;
}
