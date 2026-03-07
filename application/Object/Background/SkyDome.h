#pragma once
#include "Input.h"
#include "Object3d/Object3d.h"
#include "Transform.h"
#include <memory>

class GameBase;
class Camera;

class SkyDome {

private:
	Transform transform_;
	std::unique_ptr<Object3d> skyDomeObject_ = nullptr;
	Camera* camera_;

public:
	SkyDome();
	~SkyDome() = default;
	void Initialize(Camera* camera);
	void Update();
	void Draw();
	void SetCamera(Camera* camera) { camera_ = camera; }
};
