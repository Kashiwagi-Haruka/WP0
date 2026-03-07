#pragma once
#include "Audio.h"
#include "BaseScene.h"
#include "SceneTransition/SceneTransition.h"
#include "Sprite.h"
#include <imgui.h>
#include <memory>
class GameBase;
class GameOverScene : public BaseScene {

	struct SpriteData {
		std::unique_ptr<Sprite> sprite = nullptr;
		uint32_t handle = 0;
		Vector2 size = {100, 100};
		Vector2 rotate = {0, 0};
		Vector2 translate = {0, 0};
	};

	SpriteData logoSP_;
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
	GameOverScene();
	~GameOverScene() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
};
