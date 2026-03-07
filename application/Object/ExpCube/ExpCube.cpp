#include "ExpCube.h"
#include "Camera.h"
#include <cmath>

namespace {
const Vector4 kExpCubeColor = {0.6f, 1.0f, 0.3f, 1.0f};
const float kRotateSpeed = 0.03f;
const float kReturnSpeed = 0.2f;
} // namespace

void ExpCube::Initialize(Camera* camera, const Vector3& position) {
	camera_ = camera;
	isCollected_ = false;
	primitive_ = std::make_unique<Primitive>();
	primitive_->Initialize(Primitive::Box, "Resources/2d/ExpGauge.png");
	primitive_->SetEnableLighting(false);
	primitive_->SetColor(kExpCubeColor);
	transform_ = {
	    {0.35f,      0.35f,      0.35f     },
	    {0.0f,       0.0f,       0.0f      },
	    {position.x, position.y, position.z},
	};
	primitive_->SetTransform(transform_);
}

void ExpCube::Update(const Vector3& movementLimitCenter, float movementLimitRadius) {
	if (isCollected_) {
		return;
	}

	const float dx = transform_.translate.x - movementLimitCenter.x;
	const float dz = transform_.translate.z - movementLimitCenter.z;
	const float distSquared = dx * dx + dz * dz;
	if (distSquared > movementLimitRadius * movementLimitRadius) {
		const float dist = std::sqrt(distSquared);
		const float scale = movementLimitRadius / dist;
		const Vector3 target = {
		    movementLimitCenter.x + dx * scale,
		    transform_.translate.y,
		    movementLimitCenter.z + dz * scale,
		};
		const float toX = target.x - transform_.translate.x;
		const float toZ = target.z - transform_.translate.z;
		const float moveDistSquared = toX * toX + toZ * toZ;
		if (moveDistSquared <= kReturnSpeed * kReturnSpeed) {
			transform_.translate.x = target.x;
			transform_.translate.z = target.z;
		} else {
			const float moveDist = std::sqrt(moveDistSquared);
			transform_.translate.x += (toX / moveDist) * kReturnSpeed;
			transform_.translate.z += (toZ / moveDist) * kReturnSpeed;
		}
	}

	transform_.rotate.y += kRotateSpeed;
	primitive_->SetCamera(camera_);
	primitive_->SetTransform(transform_);
	primitive_->Update();
}

void ExpCube::Draw() {
	if (isCollected_) {
		return;
	}
	primitive_->Draw();
}