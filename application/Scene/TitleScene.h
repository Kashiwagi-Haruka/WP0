#pragma once
#include "Audio.h"
#include "BaseScene.h"
#include "SceneTransition/SceneTransition.h"
#include "Sprite.h"
#include <memory>
class GameBase;

class TitleScene : public BaseScene {
	SoundData BGMData;
	bool isBGMPlaying;
	bool isTransitionIn = false;
	bool isTransitionOut = false;
	std::unique_ptr<SceneTransition> transition = nullptr;


public:
	TitleScene();
	~TitleScene() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
};
