#pragma once
#include "Audio.h"
#include "BaseScene.h"
#include <array>
#include <cstdint>
#include <memory>

class CameraController;
class MapchipField;
class Player;
class SkyDome;
class Sprite;
class Pause;
class TutorialUI;
class ExpCubeManager;
class TutorialScene : public BaseScene {

public:
	TutorialScene();
	~TutorialScene() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;

private:
	static constexpr int kStepCount = 8;
	static constexpr int kExpCubeTargetCount = 4;
	static constexpr int kSkillUseTargetCount = 3;
	static constexpr float kSkipHoldDuration = 1.0f;
	static constexpr float kAutoAdvanceDuration = 2.0f;
	static constexpr float kEndAutoAdvanceDuration = 2.0f;
	static constexpr float kMoveActionDuration = 2.0f;
	static constexpr float kLookActionDuration = 2.0f;

	std::unique_ptr<CameraController> cameraController_;
	std::unique_ptr<SkyDome> skyDome_;
	std::unique_ptr<Player> player_;
	std::unique_ptr<MapchipField> field_;
	std::unique_ptr<ExpCubeManager> expCubeManager_;
	std::unique_ptr<Sprite> controlSprite_;
	std::unique_ptr<TutorialUI> tutorialUI_;
	std::unique_ptr<Pause> pause_;
	uint32_t controlSpriteHandle_ = 0;

	std::array<bool, kStepCount> stepCompleted_{};
	int currentStepIndex_ = 0;
	bool isTutorialComplete_ = false;
	bool isPaused_ = false;
	bool wasSkipKeyHeld_ = false;
	float skipHoldTimer_ = 0.0f;
	float stepTimer_ = 0.0f;
	float stepActionTimer_ = 0.0f;
	float currentStepProgress_ = 0.0f;
	int expCubeCollectedCount_ = 0;
	bool expCubesSpawned_ = false;
	int skillUseCount_ = 0;
	bool attackComboCompleted_ = false;
	int previousStepIndex_ = -1;
	SoundData BGMData_{};
	bool isBGMPlaying_ = false;
};