#pragma once
#include "Audio.h"
#include "BaseScene.h"
#include "SceneTransition/SceneTransition.h"
#include "Sprite.h"
#include <imgui.h>
#include <memory>
class GameBase;

class ResultScene : public BaseScene {

	struct SpriteData {
		std::unique_ptr<Sprite> sprite = nullptr;
		uint32_t handle = 0;
		Vector2 size = {100, 100};
		float rotate = 0;
		Vector2 translate = {0, 0};
	};

	SpriteData logoSP_;
	SpriteData starSP_[3];
	SpriteData timeDigitSP_[4];
	SpriteData houseHpStringSP_;

	uint32_t starOnHandle_ = 0;
	uint32_t starOffHandle_ = 0;
	uint32_t numberHandle_ = 0;
	uint32_t houseHpStringHandle_ = 0;

	int resultStars_ = 0;
	int resultMinutes_ = 0;
	int resultSeconds_ = 0;
	Vector2 numberTextureSize_ = {300, 300};
	Vector2 timeDigitSize_ = {48, 48};
	Vector2 starSize_ = {120, 120};
	Vector2 houseHpStringSize_ = {320, 180};
	std::unique_ptr<Sprite> pressSpaceSprite = nullptr;
	uint32_t pressSpaceHandle = 0;

	Vector2 pressSpacePos = {640, 420};
	Vector2 pressSpaceSize = {300, 300};

	std::unique_ptr<SceneTransition> transition = nullptr;
	bool isTransitionIn = false;
	bool isTransitionOut = false;

	SoundData BGM_;
	bool isBGMPlaying = false;

public:
	ResultScene();
	~ResultScene() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
};