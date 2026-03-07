#pragma once
#include <memory>
#include "Object3d/Object3d.h"
#include <vector>
#include "Transform.h"
#include "Primitive/Primitive.h"
#include "Vector3.h"
class EnemyHitEffect {

	std::unique_ptr<Object3d> hitEffect_;
	std::unique_ptr<Primitive> hitPrimitive_;
	std::unique_ptr<Primitive> hitPrimitiveInner_;
	Transform hitTransform_;
	Transform hitPrimitiveTransform_;
	Transform hitPrimitiveInnerTransform_;
	Camera* camera_ = nullptr;
	Vector3 enemyPosition_;
	bool isActive_ = false;
	float activeTimer_ = 0.0f;
	float activeDuration_ = 0.2f;

public:
	void Initialize();
	void SetCamera(Camera* camera) { camera_ = camera; };
	void SetPosition(const Vector3& position) { enemyPosition_ = position; };
	void Activate(const Vector3& position);
	bool IsActive() const { return isActive_; }
	void Update();
	void Draw();


};
