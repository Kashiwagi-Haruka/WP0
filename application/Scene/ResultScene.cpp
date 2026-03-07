#include "ResultScene.h"
#include "Input.h"
#include "GameTimer/GameTimer.h"
#include "Object/House/HouseHP.h"
#include "SceneManager.h"
#include "Sprite/SpriteCommon.h"
#include "TextureManager.h"
#include <algorithm>

namespace {
constexpr float kGoodTimeSeconds = 120.0f;
constexpr float kOkTimeSeconds = 180.0f;
constexpr float kHighHpRatio = 0.8f;
constexpr float kMidHpRatio = 0.5f;
} // namespace

int CalculateStarCount(float timeSeconds, int houseHp) {
	float hpRatio = std::clamp(houseHp / static_cast<float>(HouseHP::GetInstance()->GetMaxHP()), 0.0f, 1.0f);
	int stars = 1;
	if (timeSeconds <= kGoodTimeSeconds && hpRatio >= kHighHpRatio) {
		stars = 3;
	} else if (timeSeconds <= kOkTimeSeconds && hpRatio >= kMidHpRatio) {
		stars = 2;
	}
	return stars;
}
ResultScene::ResultScene() {

	logoSP_.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/result.png");
	logoSP_.sprite = std::make_unique<Sprite>();
	logoSP_.sprite->Initialize(logoSP_.handle);
	starOnHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/StarOn.png");
	starOffHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/StarOff.png");
	numberHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/No.png");
	houseHpStringHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/houseHPString.png");
	BGM_ = Audio::GetInstance()->SoundLoadFile("Resources/audio/BGM/アンドロイドの涙.mp3");

	for (int i = 0; i < 4; ++i) {
		timeDigitSP_[i].handle = numberHandle_;
		timeDigitSP_[i].sprite = std::make_unique<Sprite>();
		timeDigitSP_[i].sprite->Initialize(timeDigitSP_[i].handle);
	}
	houseHpStringSP_.handle = houseHpStringHandle_;
	houseHpStringSP_.sprite = std::make_unique<Sprite>();
	houseHpStringSP_.sprite->Initialize(houseHpStringSP_.handle);

	transition = std::make_unique<SceneTransition>();
}

void ResultScene::Finalize() { Audio::GetInstance()->SoundUnload(&BGM_); }

void ResultScene::Initialize() {
	logoSP_.sprite->SetAnchorPoint({0.5f, 0.5f});
	logoSP_.size = {1280, 720};
	logoSP_.translate = {640, 360};
	logoSP_.sprite->SetScale(logoSP_.size);

	logoSP_.sprite->SetPosition(logoSP_.translate);
	logoSP_.sprite->Update();

	isSceneEnd_ = false;
	// SPACE 画像読み込み
	pressSpaceHandle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/SPACE.png");

	pressSpaceSprite = std::make_unique<Sprite>();
	pressSpaceSprite->Initialize(pressSpaceHandle);

	// 中央寄せ
	pressSpaceSprite->SetAnchorPoint({0.5f, 0.5f});
	pressSpaceSprite->SetTextureRange({0, 0}, {768, 768});
	// 300 × 300
	pressSpaceSprite->SetScale(pressSpaceSize);

	// 画面中央より少し下（Y = 360 より下）
	pressSpaceSprite->SetPosition(pressSpacePos);

	pressSpaceSprite->Update();
	float totalSeconds = GameTimer::GetInstance()->GetTimer();
	resultMinutes_ = static_cast<int>(totalSeconds) / 60;
	resultSeconds_ = static_cast<int>(totalSeconds) % 60;
	resultStars_ = CalculateStarCount(totalSeconds, HouseHP::GetInstance()->GetHP());

	Vector2 starStart = {460, 360};
	float starSpacing = 180.0f;
	for (int i = 0; i < 3; ++i) {
		starSP_[i].handle = (i < resultStars_) ? starOnHandle_ : starOffHandle_;
		starSP_[i].sprite = std::make_unique<Sprite>();
		starSP_[i].sprite->Initialize(starSP_[i].handle);
		starSP_[i].sprite->SetAnchorPoint({0.5f, 0.5f});
		starSP_[i].size = starSize_;
		starSP_[i].translate = {starStart.x + starSpacing * i, starStart.y};
		starSP_[i].sprite->SetScale(starSP_[i].size);
		starSP_[i].sprite->SetPosition(starSP_[i].translate);
		starSP_[i].sprite->Update();
	}
	Vector2 starTopLeft = {starStart.x - starSize_.x*3.0f, starStart.y+80.0f};
	houseHpStringSP_.sprite->SetAnchorPoint({0.0f, 1.0f});
	houseHpStringSP_.size = houseHpStringSize_;
	houseHpStringSP_.translate = starTopLeft;
	houseHpStringSP_.sprite->SetScale(houseHpStringSP_.size);
	houseHpStringSP_.sprite->SetPosition(houseHpStringSP_.translate);
	houseHpStringSP_.sprite->Update();
	int minuteTens = (resultMinutes_ / 10) % 10;
	int minuteOnes = resultMinutes_ % 10;
	int secondTens = (resultSeconds_ / 10) % 10;
	int secondOnes = resultSeconds_ % 10;

	int digits[4] = {minuteTens, minuteOnes, secondTens, secondOnes};
	Vector2 digitStart = {580, 280};
	float digitSpacing = 50.0f;
	for (int i = 0; i < 4; ++i) {
		timeDigitSP_[i].sprite->SetAnchorPoint({0.5f, 0.5f});
		timeDigitSP_[i].size = timeDigitSize_;
		timeDigitSP_[i].translate = {digitStart.x + digitSpacing * i, digitStart.y};
		timeDigitSP_[i].sprite->SetScale(timeDigitSP_[i].size);
		timeDigitSP_[i].sprite->SetPosition(timeDigitSP_[i].translate);
		timeDigitSP_[i].sprite->SetTextureRange({300.0f * digits[i], 0}, numberTextureSize_);
		timeDigitSP_[i].sprite->Update();
	}

	Input::GetInstance()->SetIsCursorStability(false);
	Input::GetInstance()->SetIsCursorVisible(true);
	transition->Initialize(false);
	isTransitionIn = true;
	isTransitionOut = false;
	isBGMPlaying = false;
}
void ResultScene::Update() {
	if (!isBGMPlaying) {
		Audio::GetInstance()->SoundPlayWave(BGM_, true);
		isBGMPlaying = true;
	}
	logoSP_.sprite->Update();
	for (int i = 0; i < 3; ++i) {
		starSP_[i].sprite->Update();
	}
	for (int i = 0; i < 4; ++i) {
		timeDigitSP_[i].sprite->Update();
	}
	houseHpStringSP_.sprite->Update();
	if (Input::GetInstance()->TriggerKey(DIK_SPACE) && !isTransitionOut) {
		transition->Initialize(true);
		isTransitionOut = true;
	}
	if (isTransitionIn || isTransitionOut) {
		transition->Update();
		if (transition->IsEnd() && isTransitionIn) {
			isTransitionIn = false;
		}
		if (transition->IsEnd() && isTransitionOut) {
			SceneManager::GetInstance()->ChangeScene("Title");
		}
	}
#ifdef USE_IMGUI
	ImGui::Begin("resultScene");
	ImGui::End();
#endif // USE_IMGUI
}

void ResultScene::Draw() {
	SpriteCommon::GetInstance()->DrawCommon();
	logoSP_.sprite->Draw();
	for (int i = 0; i < 3; ++i) {
		starSP_[i].sprite->Draw();
	}
	for (int i = 0; i < 4; ++i) {
		timeDigitSP_[i].sprite->Draw();
	}
	houseHpStringSP_.sprite->Draw();
	pressSpaceSprite->Draw();
	if (isTransitionIn || isTransitionOut) {
		transition->Draw();
	}
}