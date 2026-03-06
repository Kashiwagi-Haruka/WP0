#pragma once
#include "BaseScene.h"
#include "Camera.h"
#include "DebugCamera.h"
#include "Primitive/Primitive.h"
#include "Transform.h"
#include <array>
#include <cstddef>
#include <memory>

class CoffeeScene : public BaseScene {
private:
	static constexpr size_t kWallCount_ = 6;
	std::array<std::unique_ptr<Primitive>, kWallCount_> roomWalls_{};
	std::unique_ptr<Camera> camera_ = nullptr;
	std::unique_ptr<DebugCamera> debugCamera_ = nullptr;
	Transform cameraTransform_{};
	bool useDebugCamera_ = false;

public:
	CoffeeScene();
	~CoffeeScene() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
};