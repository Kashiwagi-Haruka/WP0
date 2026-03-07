#pragma once
#include "Audio.h"
#include "BaseScene.h"
#include "Object/Character/Model/CharacterModel.h"
#include "SceneTransition/SceneTransition.h"
#include "Sprite.h"
#include <memory>
class GameBase;

class TitleScene : public BaseScene {

	struct SpriteData {
		std::unique_ptr<Sprite> sprite = nullptr;
		uint32_t handle = 0;
		Vector2 size = {100, 100};
		float rotate = {0};
		Vector2 translate = {0, 0};
	};

	SpriteData BGSP_;
	SpriteData logoSP_;
	SpriteData ruleSP_;
	std::unique_ptr<Sprite> pressSpaceSprite = nullptr;
	uint32_t pressSpaceHandle = 0;
	Vector2 pressSpacePos = {640, 420};
	Vector2 pressSpaceSize = {300, 300};
	SoundData BGMData;
	bool isBGMPlaying;
	bool isTransitionIn = false;
	bool isTransitionOut = false;
	std::unique_ptr<SceneTransition> transition = nullptr;

	CharacterModel characterModel_;

public:
	TitleScene();
	~TitleScene() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
};
