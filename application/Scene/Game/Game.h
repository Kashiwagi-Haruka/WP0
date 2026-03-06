#pragma once
#include "D3DResourceLeakChecker.h"
#include "FrameWork.h"
#include "GameBase.h"
#include "Scene/SceneFactory/SceneFactory.h"
#include <memory>
class BaseScene;
class Game : public FrameWork {

	D3DResourceLeakChecker d3dResourceLeakChecker;

	std::unique_ptr<SceneFactory> sceneFactory_ = nullptr;

public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
	~Game() = default;
};
