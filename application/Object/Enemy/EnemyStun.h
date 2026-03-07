#pragma once
#include "Camera.h"
#include "Object3d/Object3d.h"
#include "Transform.h"
#include <memory>

class EnemyStun {

	std::unique_ptr<Object3d> object_;
	Camera* camera_ = nullptr;
	Transform transform_;

public:
	EnemyStun();
	~EnemyStun() = default;

	void SetCamera(Camera* camera);
	void SetTranslate(Vector3 translate);
	void Initialize();
	void Update();
	void Draw();
};
