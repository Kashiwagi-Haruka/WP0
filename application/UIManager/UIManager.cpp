#define NOMINMAX
#include "UIManager.h"
#include "GameBase.h"
#include "Object/House/HouseHP.h"
#include "Sprite.h"
#include "Sprite/SpriteCommon.h"
#include "TextureManager.h"
#include <algorithm>

UIManager::UIManager() {

	// ===========================
	//      テクスチャ読み込み
	// ===========================

	playerHpSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/playerHP.png");

	playerHPFlameSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/playerHPFlame.png");

	playerHPStringSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/playerHPString.png");

	// WASD / SPACE / ATTACK
	HowtoOperateSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/option.png");

	// Level
	LevelSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/Lv.png");

	// 数字テクスチャ
	for (int i = 0; i < NumbersCountMax; i++) {
		NumberSPData[i].handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/No.png");
	}
	for (int i = 0; i < 5; i++) {
		MaxSPData[i].handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/Max.png");
	}

// ステータスUPアイコン
	AttackUpSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/AttuckUp.png");

	HealthUpSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/HealthUp.png");

	SpeedUpSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/SpeedUp.png");

	AllowUpSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/ArrowUp.png");
	SkillIconSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/skill.png");
	SlashSPData[0].handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/Slash.png");

	EXPSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/Exp.png");
	expBarSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/BossHpBar.png");
	expBarBackSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/BossHpBar.png");
	houseHpPercentSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/houseHPNumbers.png");
	houseHpStringSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/houseHPString.png");

	// ===========================
	//      Sprite の生成
	// ===========================

	playerHpSPData.sprite = std::make_unique<Sprite>();
	playerHPFlameSPData.sprite = std::make_unique<Sprite>();
	playerHPStringSPData.sprite = std::make_unique<Sprite>();

	HowtoOperateSPData.sprite = std::make_unique<Sprite>();

	LevelSPData.sprite = std::make_unique<Sprite>();

	for (int i = 0; i < NumbersCountMax; i++)
		NumberSPData[i].sprite = std::make_unique<Sprite>();
	for (int i = 0; i < 5; i++) {
		MaxSPData[i].sprite = std::make_unique<Sprite>();
	}
	for (int i = 0; i < 2; i++) {
		SlashSPData[i].sprite = std::make_unique<Sprite>();
	}
	AttackUpSPData.sprite = std::make_unique<Sprite>();
	HealthUpSPData.sprite = std::make_unique<Sprite>();
	SpeedUpSPData.sprite = std::make_unique<Sprite>();
	AllowUpSPData.sprite = std::make_unique<Sprite>();
	SkillIconSPData.sprite = std::make_unique<Sprite>();
	expBarSPData.sprite = std::make_unique<Sprite>();
	expBarBackSPData.sprite = std::make_unique<Sprite>();
	houseHpPercentSPData.sprite = std::make_unique<Sprite>();
	for (int i = 0; i < 3; i++) {
		houseHpNumberSPData[i].sprite = std::make_unique<Sprite>();
	}
	houseHpStringSPData.sprite = std::make_unique<Sprite>();
}

UIManager::~UIManager() {}

void UIManager::Initialize() {

	// ------------------ HP Bar ------------------
	playerHpSPData.sprite->Initialize(playerHpSPData.handle);
	playerHpSPData.size = playerHPMaxSize;
	// ★ アンカーポイントを左端に設定（左端固定）
	playerHpSPData.sprite->SetAnchorPoint({0.0f, 0.0f});
	playerHpSPData.sprite->SetScale(playerHpSPData.size);
	// ★ 左端の位置を固定
	playerHpSPData.translate = {640 - playerHPMaxSize.x / 2, 500};
	playerHpSPData.sprite->SetPosition(playerHpSPData.translate);

	// ------------------ HP Flame ------------------
	playerHPFlameSPData.sprite->Initialize(playerHPFlameSPData.handle);
	playerHPFlameSPData.size = playerHPMaxSize;
	playerHPFlameSPData.translate = {640, 500};
	playerHPFlameSPData.sprite->SetAnchorPoint({0.5f, 0.0f});
	playerHPFlameSPData.sprite->SetPosition(playerHPFlameSPData.translate);
	playerHPFlameSPData.sprite->SetScale(playerHPFlameSPData.size);

	// ------------------ HP Label ------------------

	playerHPStringSPData.sprite->Initialize(playerHPStringSPData.handle);
	playerHPStringSPData.sprite->SetAnchorPoint({1.0f, 0.5f});
	playerHPStringSPData.sprite->SetScale({160, 160});
	playerHPStringSPData.translate = {playerHpSPData.translate.x - 10.0f, playerHpSPData.translate.y + playerHPMaxSize.y * 0.5f};
	playerHPStringSPData.sprite->SetPosition(playerHPStringSPData.translate);

	// ------------------ WASD / SPACE / ATTACK ------------------

	HowtoOperateSPData.sprite->Initialize(HowtoOperateSPData.handle);
	HowtoOperateSPData.sprite->SetScale({700, 100});
	HowtoOperateSPData.sprite->SetPosition({10, 600});

	// ------------------ Level ------------------
	LevelSPData.sprite->Initialize(LevelSPData.handle);
	LevelSPData.sprite->SetScale({50, 50});
	LevelSPData.translate = {10, 10};
	LevelSPData.sprite->SetPosition(LevelSPData.translate);

	// ------------------ Number 0～9 ------------------
	for (int i = 0; i < NumbersCountMax; i++) {
		NumberSPData[i].sprite->Initialize(NumberSPData[i].handle);
		NumberSPData[i].sprite->SetTextureRange({0, 0}, numbersTextureSize);
		NumberSPData[i].sprite->SetScale({32, 32});
	}
	for (int i = 0; i < 5; i++) {
		MaxSPData[i].sprite->Initialize(MaxSPData[i].handle);

		MaxSPData[i].sprite->SetScale({48, 48});
	}

	for (int i = 0; i < 2; i++) {
		SlashSPData[i].sprite->Initialize(SlashSPData[0].handle);
	}

	// Up アイコン
	AttackUpSPData.sprite->Initialize(AttackUpSPData.handle);
	AttackUpSPData.sprite->SetScale({48, 48});

	HealthUpSPData.sprite->Initialize(HealthUpSPData.handle);
	HealthUpSPData.sprite->SetScale({48, 48});

	SpeedUpSPData.sprite->Initialize(SpeedUpSPData.handle);
	SpeedUpSPData.sprite->SetScale({48, 48});

	AllowUpSPData.sprite->Initialize(AllowUpSPData.handle);
	AllowUpSPData.sprite->SetScale({48, 48});

	SkillIconSPData.sprite->Initialize(SkillIconSPData.handle);
	SkillIconSPData.sprite->SetAnchorPoint({1.0f, 1.0f});
	SkillIconSPData.sprite->SetScale({64, 64});

	houseHpPercentSPData.sprite->Initialize(houseHpPercentSPData.handle);
	houseHpPercentSPData.sprite->SetTextureRange({houseHpNumbersTextureSize.x * 10.0f, 0}, houseHpNumbersTextureSize);
	houseHpPercentSPData.sprite->SetScale({32, 32});

	houseHpStringSPData.sprite->Initialize(houseHpStringSPData.handle);
	houseHpStringSPData.sprite->SetAnchorPoint({0.5f, 0.5f});
	houseHpStringSPData.sprite->SetScale({160, 90});
	for (int i = 0; i < 3; i++) {
		houseHpNumberSPData[i].sprite->Initialize(houseHpPercentSPData.handle);
		houseHpNumberSPData[i].sprite->SetTextureRange({0, 0}, houseHpNumbersTextureSize);
		houseHpNumberSPData[i].sprite->SetScale({32, 32});
	}

	// ------------------ EXP Bar ------------------
	expBarSPData.sprite->Initialize(expBarSPData.handle);
	expBarSPData.sprite->SetAnchorPoint({0.0f, 0.0f});
	expBarSPData.sprite->SetColor({0.0f, 0.8f, 0.0f, 1.0f});
	expBarSPData.size = expBarMaxSize;
	expBarSPData.sprite->SetScale(expBarSPData.size);
	expBarSPData.translate = {100, 20};
	expBarSPData.sprite->SetPosition(expBarSPData.translate);

	expBarBackSPData.sprite->Initialize(expBarBackSPData.handle);
	expBarBackSPData.sprite->SetAnchorPoint({0.0f, 0.0f});
	expBarBackSPData.sprite->SetColor({0.2f, 0.2f, 0.2f, 1.0f});
	expBarBackSPData.size = expBarMaxSize;
	expBarBackSPData.sprite->SetScale(expBarBackSPData.size);
	expBarBackSPData.translate = expBarSPData.translate;
	expBarBackSPData.sprite->SetPosition(expBarBackSPData.translate);
}

void UIManager::Update() {

	// ---- HP (左端固定、右端から減る) ----
	if (playerHPMax > 0) {
		// HP比率を計算
		float hpRatio = (float)playerHP / (float)playerHPMax;

		// スケールを更新（HPに応じて幅を変更）
		playerHpSPData.size.x = playerHPMaxSize.x * hpRatio;
		playerHpSPData.sprite->SetScale(playerHpSPData.size);

		// テクスチャ範囲を更新（左端0から、HP比率分だけ表示）
		playerHPWidth = playerHPWidthMax * hpRatio;
		playerHpSPData.sprite->SetTextureRange({0, 0}, {playerHPWidth, 300});

		// ★ 左端の位置は常に固定（ずれない）
		playerHpSPData.translate = {640 - playerHPMaxSize.x / 2, 500};
		playerHpSPData.sprite->SetPosition(playerHpSPData.translate);
	}

	// 各スプライト Update
	playerHpSPData.sprite->Update();
	playerHPFlameSPData.sprite->Update();
	playerHPStringSPData.translate = {playerHpSPData.translate.x - 10.0f, playerHpSPData.translate.y + playerHPMaxSize.y * 0.5f};
	playerHPStringSPData.sprite->SetPosition(playerHPStringSPData.translate);
	playerHPStringSPData.sprite->Update();

	HowtoOperateSPData.sprite->Update();

	LevelSPData.sprite->Update();

	AttackUpSPData.translate = {10, 70};
	AttackUpSPData.sprite->SetPosition(AttackUpSPData.translate);
	AttackUpSPData.sprite->Update();
	HealthUpSPData.translate = {10, AttackUpSPData.translate.y + 70};
	HealthUpSPData.sprite->SetPosition(HealthUpSPData.translate);
	HealthUpSPData.sprite->Update();
	SpeedUpSPData.translate = {10, HealthUpSPData.translate.y + 70};
	SpeedUpSPData.sprite->SetPosition(SpeedUpSPData.translate);
	SpeedUpSPData.sprite->Update();
	AllowUpSPData.translate = {10, SpeedUpSPData.translate.y + 70};
	AllowUpSPData.sprite->SetPosition(AllowUpSPData.translate);
	AllowUpSPData.sprite->Update();
	SkillIconSPData.translate = {1260, 700};
	SkillIconSPData.sprite->SetPosition(SkillIconSPData.translate);
	SkillIconSPData.sprite->Update();
	// ===========================
	//     EXPバー表示
	// ===========================
	if (parameters_.MaxEXP > 0) {
		const float expRatio = std::clamp(static_cast<float>(parameters_.EXP) / static_cast<float>(parameters_.MaxEXP), 0.0f, 1.0f);
		expBarSPData.size.x = expBarMaxSize.x * expRatio;
		expBarSPData.sprite->SetScale(expBarSPData.size);
		expBarSPData.sprite->SetPosition(expBarSPData.translate);
	}
	expBarBackSPData.sprite->Update();
	expBarSPData.sprite->Update();

	NumberSPData[kLv].sprite->SetTextureRange({300.0f * parameters_.Level, 0}, numbersTextureSize);
	NumberSPData[kAttuck].sprite->SetTextureRange({300.0f * parameters_.AttuckUp, 0}, numbersTextureSize);
	NumberSPData[kHealth].sprite->SetTextureRange({300.0f * parameters_.HPUp, 0}, numbersTextureSize);
	NumberSPData[kSpeed].sprite->SetTextureRange({300.0f * parameters_.SpeedUp, 0}, numbersTextureSize);
	NumberSPData[kArrow].sprite->SetTextureRange({300.0f * parameters_.AllowUp, 0}, numbersTextureSize);

	NumberSPData[kLv].translate = {LevelSPData.translate.x + 50, LevelSPData.translate.y + 12};
	NumberSPData[kLv].sprite->SetPosition(NumberSPData[kLv].translate);

	NumberSPData[kAttuck].translate = {AttackUpSPData.translate.x + 40, AttackUpSPData.translate.y + 20};
	NumberSPData[kAttuck].sprite->SetPosition(NumberSPData[kAttuck].translate);

	NumberSPData[kHealth].translate = {HealthUpSPData.translate.x + 40, HealthUpSPData.translate.y + 20};
	NumberSPData[kHealth].sprite->SetPosition(NumberSPData[kHealth].translate);

	NumberSPData[kSpeed].translate = {SpeedUpSPData.translate.x + 40, SpeedUpSPData.translate.y + 20};
	NumberSPData[kSpeed].sprite->SetPosition(NumberSPData[kSpeed].translate);

	NumberSPData[kArrow].translate = {AllowUpSPData.translate.x + 40, AllowUpSPData.translate.y + 20};
	NumberSPData[kArrow].sprite->SetPosition(NumberSPData[kArrow].translate);


	for (int i = 0; i < NumbersCountMax; i++) {
		NumberSPData[i].sprite->Update();
	}
	MaxSPData[0].translate = {NumberSPData[kLv].translate.x, NumberSPData[kLv].translate.y - 10};
	MaxSPData[1].translate = {NumberSPData[kAttuck].translate.x, NumberSPData[kAttuck].translate.y - 10};
	MaxSPData[2].translate = {NumberSPData[kHealth].translate.x, NumberSPData[kHealth].translate.y - 10};
	MaxSPData[3].translate = {NumberSPData[kSpeed].translate.x, NumberSPData[kSpeed].translate.y - 10};
	MaxSPData[4].translate = {NumberSPData[kArrow].translate.x, NumberSPData[kArrow].translate.y - 10};

	for (int i = 0; i < 5; i++) {

		MaxSPData[i].sprite->SetPosition(MaxSPData[i].translate);
		MaxSPData[i].sprite->Update();
	}
	int houseMaxHp = HouseHP::GetInstance()->GetMaxHP();
	int houseHp = std::clamp(HouseHP::GetInstance()->GetHP(), 0, houseMaxHp);
	int houseHpPercent = std::clamp(houseHp * 100 / houseMaxHp, 0, 100);
	houseHpDigitStartIndex = houseHpPercent >= 100 ? 0 : (houseHpPercent >= 10 ? 1 : 2);
	int houseDigits[3] = {houseHpPercent / 100, (houseHpPercent / 10) % 10, houseHpPercent % 10};
	float xOffset = houseHpPercentBasePosition.x;
	for (int i = houseHpDigitStartIndex; i < 3; i++) {
		houseHpNumberSPData[i].sprite->SetTextureRange({houseHpNumbersTextureSize.x * houseDigits[i], 0}, houseHpNumbersTextureSize);
		houseHpNumberSPData[i].translate = {xOffset, houseHpPercentBasePosition.y};
		houseHpNumberSPData[i].sprite->SetPosition(houseHpNumberSPData[i].translate);
		houseHpNumberSPData[i].sprite->Update();
		xOffset += 32.0f;
	}
	houseHpPercentSPData.translate = {xOffset, houseHpPercentBasePosition.y};
	houseHpPercentSPData.sprite->SetPosition(houseHpPercentSPData.translate);
	houseHpPercentSPData.sprite->Update();
	float houseHpLabelRight = xOffset + 32.0f;
	Vector2 houseHpLabelCenter = {(houseHpPercentBasePosition.x + houseHpLabelRight) * 0.5f, houseHpPercentBasePosition.y + houseHpStringOffset.y};
	houseHpStringSPData.translate = houseHpLabelCenter;
	houseHpStringSPData.sprite->SetPosition(houseHpStringSPData.translate);
	houseHpStringSPData.sprite->Update();
}

void UIManager::Draw() {

	SpriteCommon::GetInstance()->DrawCommon();

	playerHpSPData.sprite->Draw();
	playerHPFlameSPData.sprite->Draw();
	playerHPStringSPData.sprite->Draw();

	HowtoOperateSPData.sprite->Draw();

	LevelSPData.sprite->Draw();

	AttackUpSPData.sprite->Draw();
	HealthUpSPData.sprite->Draw();
	SpeedUpSPData.sprite->Draw();
	AllowUpSPData.sprite->Draw();
	SkillIconSPData.sprite->Draw();
	expBarBackSPData.sprite->Draw();
	expBarSPData.sprite->Draw();
	if (parameters_.Level < parameters_.MaxLevel) {
		NumberSPData[kLv].sprite->Draw();
	} else {
		MaxSPData[0].sprite->Draw();
	}
	if (parameters_.AttuckUp < 10) {
		NumberSPData[kAttuck].sprite->Draw();
	} else {
		MaxSPData[1].sprite->Draw();
	}
	if (parameters_.HPUp < 10) {
		NumberSPData[kHealth].sprite->Draw();
	} else {
		MaxSPData[2].sprite->Draw();
	}
	if (parameters_.SpeedUp < 10) {
		NumberSPData[kSpeed].sprite->Draw();
	} else {
		MaxSPData[3].sprite->Draw();
	}
	if (parameters_.AllowUp < 10) {
		NumberSPData[kArrow].sprite->Draw();
	} else {
		MaxSPData[4].sprite->Draw();
	}
	for (int i = houseHpDigitStartIndex; i < 3; i++) {
		houseHpNumberSPData[i].sprite->Draw();
	}
	houseHpPercentSPData.sprite->Draw();
	houseHpStringSPData.sprite->Draw();
}

void UIManager::SetPlayerHP(int HP) { playerHP = HP; }
void UIManager::SetPlayerHPMax(int HPMax) { playerHPMax = HPMax; }
void UIManager::SetPlayerParameters(Parameters parameters) { parameters_ = parameters; }