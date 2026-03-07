#pragma once
#include <array>
#include <cstdint>
#include <memory>

class Sprite;

class TutorialUI {

public:
	void Initialize();
	void Update(float tutorialProgress, float skipProgress, int stepIndex, bool showSkip);
	void Draw();

private:
	static constexpr int kStepCount = 8;
	static constexpr int kStepSpriteCount = kStepCount;
	static constexpr float kStepSpriteWidth = 320.0f;
	static constexpr float kStepSpriteHeight = 180.0f;
	static constexpr float kProgressBarWidth = 360.0f;
	static constexpr float kProgressBarHeight = 20.0f;
	static constexpr float kSkipBarWidth = 300.0f;
	static constexpr float kSkipBarHeight = 12.0f;

	std::array<std::unique_ptr<Sprite>, kStepSpriteCount> tutorialStepSprites_;
	std::unique_ptr<Sprite> tutorialIconSprite_;
	std::unique_ptr<Sprite> tutorialProgressBackSprite_;
	std::unique_ptr<Sprite> tutorialProgressFillSprite_;
	std::unique_ptr<Sprite> tutorialProgressLabelSprite_;
	std::unique_ptr<Sprite> tutorialSkipBackSprite_;
	std::unique_ptr<Sprite> tutorialSkipFillSprite_;
	std::unique_ptr<Sprite> tutorialSkipLabelSprite_;

	std::array<uint32_t, kStepSpriteCount> tutorialStepHandles_{};
	uint32_t tutorialIconHandle_ = 0;
	uint32_t tutorialProgressBackHandle_ = 0;
	uint32_t tutorialProgressFillHandle_ = 0;
	uint32_t tutorialProgressLabelHandle_ = 0;
	uint32_t tutorialSkipBackHandle_ = 0;
	uint32_t tutorialSkipFillHandle_ = 0;
	uint32_t tutorialSkipLabelHandle_ = 0;

	int currentStepIndex_ = 0;
	bool showSkip_ = false;
};