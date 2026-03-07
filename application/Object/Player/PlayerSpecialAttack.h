#pragma once
#include "Object3d/Object3d.h"
#include "Transform.h"
#include <memory>
#include <vector>
class PlayerSpecialAttack {


	Camera* camera_ = nullptr;
	Transform transform_;

public:
	PlayerSpecialAttack();
	void Initialize();
	void Update(const Transform& playerTransform);
	void Draw();
	void SetCamera(Camera* camera) { camera_ = camera; }

};