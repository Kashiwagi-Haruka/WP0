#pragma once
#include "Audio.h"
#include "BaseScene.h"
#include "Object/Character/Model/CharacterModel.h"
#include "CollisionManager/CollisionManager.h"
#include "GameBase.h"
#include "Light/DirectionalLight.h"
#include "Light/PointLight.h"
#include "Light/SpotLight.h"
#include "Object/House/House.h"
#include "Object/MapchipField.h"
#include "Object3d/Object3d.h"
#include "Particles/Particles.h"
#include "Pause/Pause.h"
#include "SceneTransition/SceneTransition.h"
#include "Sprite.h"
#include "UIManager/UIManager.h"
#include "Object/Character/CharacterDisplay/CharacterDisplay.h"
#include "Vector2.h"
#include <array>
#include <cstdint>
#include <imgui.h>
#include <string>
class Player;
class Enemy;
class CameraController;
class SkyDome;
class EnemyManager;
class ExpCubeManager;
class Boss;

class GameScene : public BaseScene {

private:
	bool IsPKey = false;
	bool IsXButton = false;
	bool IsKeyboard = true;

	uint32_t color;
	std::unique_ptr<Particles> particles;
	std::unique_ptr<UIManager> uimanager;
	std::unique_ptr<SceneTransition> sceneTransition;
	std::unique_ptr<Player> player;
	std::unique_ptr<EnemyManager> enemyManager;
	std::unique_ptr<ExpCubeManager> expCubeManager;
	std::unique_ptr<SkyDome> skyDome;
	std::unique_ptr<CameraController> cameraController;
	std::unique_ptr<MapchipField> field;
	std::unique_ptr<House> house;
	std::unique_ptr<Pause> pause;

	CollisionManager collisionManager_;

	bool isTransitionIn = true;
	bool isTransitionOut = false;
	std::string nextSceneName;
	bool isBGMPlaying = false;
	bool isPause = false;
	bool isCharacterDisplayMode_ = false;

	SoundData BGMData;

	CharacterModel characterModel;
	std::unique_ptr<CharacterDisplay> characterDisplay_;

	// レベルアップ選択専用スプライト
	std::unique_ptr<Sprite> levelupIcons[4]; // 0:Atk, 1:Speed, 2:HP, 3:Allow
	std::unique_ptr<Sprite> phaseSprites_[5];
	std::unique_ptr<Sprite> bossHpBarSprite_;
	std::unique_ptr<Sprite> bossHpBarBackSprite_;
	std::unique_ptr<Sprite> warningSprite_;
	int currentPhaseSpriteIndex_ = 0;
	int lastWave_ = 0;
	bool isPhaseSpriteActive_ = false;
	bool isPhaseSpritePaused_ = false;
	float phaseSpriteX_ = 0.0f;
	float phaseSpriteStopTimer_ = 0.0f;
	Vector2 phaseSpriteSize_ = {400.0f, 120.0f};
	float phaseSpriteY_ = 80.0f;
	Vector2 bossHpBarSize_ = {400.0f, 120.0f};
	Vector2 warningSpriteBaseScale_ = {600.0f, 200.0f};
	bool isWarningActive_ = false;
	float warningTimer_ = 0.0f;
	float warningDuration_ = 2.5f;
	bool isBossActive_ = false;
	std::unique_ptr<Boss> boss_;

	DirectionalLight directionalLight_{};
	std::array<PointLight, kMaxPointLights> pointLights_{};
	uint32_t activePointLightCount_ = 0;
	std::array<SpotLight, kMaxSpotLights> spotLights_{};
	uint32_t activeSpotLightCount_ = 0;

	bool goalActive = false; // 敵全滅後に true になる

	bool sceneEndClear = false;
	bool sceneEndOver = false;
	// =====================
	// レベルアップ選択用
	// =====================
	bool isLevelSelecting = false;
	int selectChoices[2]; // 0:Atk, 1:Speed, 2:HP, 3:Allow
	int cursorIndex = 0;  // 0 or 1
	int debugPhaseSelection_ = 0;

	void ApplyPhaseSelection(int selectionIndex);

public:
	GameScene();
	~GameScene() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;

	void DebugImGui();
};
