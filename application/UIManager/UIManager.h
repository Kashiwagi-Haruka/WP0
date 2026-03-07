#pragma once
#include "Object/Player/PlayerParameters.h"
#include "Vector2.h"
#include <cstdint>
#include <memory>
class Sprite;
class GameBase;
class UIManager {

	// 数字表示で使用するインデックス一覧。
	enum Numbers {

		kExp100,        // EXPの百の位。
		kExp10,         // EXPの十の位。
		kEexp1,         // EXPの一の位。
		kExpMax100,     // 最大EXPの百の位。
		kExpMax10,      // 最大EXPの十の位。
		kExpMax1,       // 最大EXPの一の位。
		kLv,            // レベル表示用。
		kAttuck,        // 攻撃アップ表示用。
		kHealth,        // 体力アップ表示用。
		kSpeed,         // 速度アップ表示用。
		kArrow,         // 矢数アップ表示用。
		NumbersCountMax // 数字表示の総数。
	};
	// スプライトに必要なリソースと変換情報。
	struct SpriteData {
		// 描画用スプライト本体。
		std::unique_ptr<Sprite> sprite;
		// 使用するテクスチャハンドル。
		uint32_t handle = 0;
		// スプライトサイズ（スケール）。
		Vector2 size = {100, 100};
		// 回転量（未使用の場合は0）。
		Vector2 rotate = {0, 0};
		// 描画位置。
		Vector2 translate = {0, 0};
	};

	// プレイヤーHPバー用スプライト。
	SpriteData playerHpSPData;
	// プレイヤーHPフレーム用スプライト。
	SpriteData playerHPFlameSPData;
	// プレイヤーHPラベル用スプライト。
	SpriteData playerHPStringSPData;
	// 操作方法表示用スプライト。
	SpriteData HowtoOperateSPData;
	// レベルラベル用スプライト。
	SpriteData LevelSPData;
	// 数字表示用スプライト群。
	SpriteData NumberSPData[NumbersCountMax];
	// MAX表示用スプライト群。
	SpriteData MaxSPData[5];
	// 攻撃アップ表示用スプライト。
	SpriteData AttackUpSPData;
	// 体力アップ表示用スプライト。
	SpriteData HealthUpSPData;
	// 速度アップ表示用スプライト。
	SpriteData SpeedUpSPData;
	// 矢数アップ表示用スプライト。
	SpriteData AllowUpSPData;
	// スキルアイコン表示用スプライト。
	SpriteData SkillIconSPData;

	// スラッシュ表示用スプライト（未使用含む）。
	SpriteData SlashSPData[2];
	// EXPラベル用スプライト。
	SpriteData EXPSPData;
	// EXPバー用スプライト。
	SpriteData expBarSPData;
	// EXPバー背景用スプライト。
	SpriteData expBarBackSPData;
	// 家HP数字表示用スプライト群。
	SpriteData houseHpNumberSPData[3];
	// 家HPの%表示用スプライト。
	SpriteData houseHpPercentSPData;
	// 家HPラベル用スプライト。
	SpriteData houseHpStringSPData;

	// 現在のプレイヤーHP。
	int playerHP;
	// プレイヤーHP最大値。
	int playerHPMax;
	// HPバー最大サイズ。
	Vector2 playerHPMaxSize = {400, 100};
	// HPバーの現在表示幅。
	float playerHPWidth = 1200.0f;
	// HPバーの最大表示幅。
	float playerHPWidthMax = 1200.0f;
	// EXPバー最大サイズ。
	Vector2 expBarMaxSize = {300, 40};
	// 数字テクスチャの1桁あたりサイズ。
	Vector2 numbersTextureSize = {300, 300};
	// 家HP数字テクスチャの1桁あたりサイズ。
	Vector2 houseHpNumbersTextureSize = {400, 400};
	// 家HPの%表示基準位置。
	Vector2 houseHpPercentBasePosition = {980, 520};
	// 家HPラベルのオフセット。
	Vector2 houseHpStringOffset = {0, -40};
	// 家HPの桁表示開始インデックス。
	int houseHpDigitStartIndex = 0;
	// プレイヤーパラメータの保持領域。
	Parameters parameters_;

public:
	// 生成時にUIリソースを読み込む。
	UIManager();
	// 破棄時にリソースを解放する。
	~UIManager();
	// UI表示の初期化を行う。
	void Initialize();
	// UI状態を更新する。
	void Update();
	// UIを描画する。
	void Draw();

	// 現在のプレイヤーHPを設定する。
	void SetPlayerHP(int HP);
	// プレイヤーHP最大値を設定する。
	void SetPlayerHPMax(int HPMax);
	// プレイヤーパラメータを設定する。
	void SetPlayerParameters(Parameters parameters);
};