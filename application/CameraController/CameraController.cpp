#include "CameraController.h"
#include "Camera.h"
#include "Input.h"
#include <imgui.h>
#include <random>
#include "Function.h"
CameraController::~CameraController() {}
void CameraController::Initialize() {

	transform_ = {
	    .scale{1.0f,        1.0f,      1.0f  },
        .rotate{orbitPitch_, orbitYaw_, 0.0f  },
        .translate{0.0f,        10.0f,     -50.0f}
    };

	camera_ = std::make_unique<Camera>();
	camera_->SetTransform(transform_);
}
void CameraController::Update() {

#ifdef USE_IMGUI

	/*if (ImGui::Begin("CameraController")) {
	    ImGui::DragFloat3("CameraScale", &transform_.scale.x, 0.01f);
	    ImGui::DragFloat("OrbitYaw", &orbitYaw_, 0.01f);
	    ImGui::DragFloat("OrbitPitch", &orbitPitch_, 0.01f);
	    ImGui::DragFloat3("CameraTranslate", &transform_.translate.x, 0.1f);
	}
	ImGui::End();*/

#endif
	const Vector2 mouseMove = Input::GetInstance()->GetMouseMove();
	orbitYaw_ += mouseMove.x * mouseSensitivity_;
	orbitPitch_ += mouseMove.y * mouseSensitivity_;

	const float maxPitch = 1.2f;
	const float minPitch = -1.2f;
	if (orbitPitch_ > maxPitch) {
		orbitPitch_ = maxPitch;
	}
	if (orbitPitch_ < minPitch) {
		orbitPitch_ = minPitch;
	}

	const float distance = 10.0f;
	Vector3 orbitDir = {sinf(orbitYaw_) * cosf(orbitPitch_), -sinf(orbitPitch_), cosf(orbitYaw_) * cosf(orbitPitch_)};

	transform_.translate = playerPos - orbitDir * distance;
	transform_.rotate = {orbitPitch_, orbitYaw_, 0.0f};

	if (shakeTimer_ > 0.0f) {
		shakeTimer_ -= 1.0f / 60.0f;
		if (shakeTimer_ < 0.0f) {
			shakeTimer_ = 0.0f;
		}
		const float decay = shakeDuration_ > 0.0f ? (shakeTimer_ / shakeDuration_) : 0.0f;
		const float strength = shakeAmplitude_ * decay;
		static std::mt19937 rng{std::random_device{}()};
		std::uniform_real_distribution<float> offsetDist(-1.0f, 1.0f);
		transform_.translate.x += offsetDist(rng) * strength;
		transform_.translate.y += offsetDist(rng) * strength;
	}

	camera_->SetTransform(transform_);
	camera_->Update();
}
void CameraController::SpecialAttackUpdate() {}
Camera* CameraController::GetCamera() { return camera_.get(); }
void CameraController::StartShake(float durationSeconds) {
	shakeDuration_ = durationSeconds;
	shakeTimer_ = durationSeconds;
}