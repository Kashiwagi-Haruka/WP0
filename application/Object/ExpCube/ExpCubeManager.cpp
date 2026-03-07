#include "ExpCubeManager.h"
#include "Camera.h"
#include "Object3d/Object3dCommon.h"
#include <algorithm>
#include <cstdlib>

namespace {
const float kDropSpread = 0.8f;
const float kDropLift = 0.2f;
} // namespace

void ExpCubeManager::Initialize(Camera* camera) {
	camera_ = camera;
	expCubes_.clear();
}

void ExpCubeManager::Update(Camera* camera, const Vector3& movementLimitCenter, float movementLimitRadius) {
	camera_ = camera;
	for (auto& cube : expCubes_) {
		if (!cube->IsCollected()) {
			cube->SetCamera(camera_);
			cube->Update(movementLimitCenter, movementLimitRadius);
		}
	}
	RemoveCollected();
}

void ExpCubeManager::Draw() {
	Object3dCommon::GetInstance()->DrawCommonEmissive();
	for (auto& cube : expCubes_) {
		cube->Draw();
	}
	Object3dCommon::GetInstance()->DrawCommon();
}

void ExpCubeManager::SpawnDrops(const Vector3& position, int count) {
	for (int i = 0; i < count; ++i) {
		float offsetX = ((static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f) * kDropSpread;
		float offsetZ = ((static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f) * kDropSpread;
		Vector3 spawnPos = position;
		spawnPos.x += offsetX;
		spawnPos.y += kDropLift;
		spawnPos.z += offsetZ;

		auto cube = std::make_unique<ExpCube>();
		cube->Initialize(camera_, spawnPos);
		expCubes_.push_back(std::move(cube));
	}
}

void ExpCubeManager::RemoveCollected() {
	expCubes_.erase(std::remove_if(expCubes_.begin(), expCubes_.end(), [](const std::unique_ptr<ExpCube>& cube) { return cube->IsCollected(); }), expCubes_.end());
}