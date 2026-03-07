#pragma once
#include "Primitive/Primitive.h"
#include "Transform.h"
#include "Vector3.h"
#include "Vector4.h"
#include <memory>

class Camera;

class ExpCube {
public:
	void Initialize(Camera* camera, const Vector3& position);
	void Update(const Vector3& movementLimitCenter, float movementLimitRadius);
	void Draw();
	void SetCamera(Camera* camera) { camera_ = camera; }
	Vector3 GetPosition() const { return transform_.translate; }
	Vector3 GetScale() const { return transform_.scale; }
	bool IsCollected() const { return isCollected_; }
	void Collect() { isCollected_ = true; }

private:
	std::unique_ptr<Primitive> primitive_;
	Transform transform_ = {
	    {1.0f, 1.0f, 1.0f},
	    {0.0f, 0.0f, 0.0f},
	    {0.0f, 0.0f, 0.0f},
	};
	Camera* camera_ = nullptr;
	bool isCollected_ = false;
};