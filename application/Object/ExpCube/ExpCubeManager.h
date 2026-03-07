#pragma once
#include "ExpCube.h"
#include "Vector3.h"
#include <memory>
#include <vector>

class Camera;

class ExpCubeManager {
public:
	void Initialize(Camera* camera);
	void Update(Camera* camera, const Vector3& movementLimitCenter, float movementLimitRadius);
	void Draw();
	void SpawnDrops(const Vector3& position, int count);
	void RemoveCollected();
	std::vector<std::unique_ptr<ExpCube>>& GetCubes() { return expCubes_; }

private:
	std::vector<std::unique_ptr<ExpCube>> expCubes_;
	Camera* camera_ = nullptr;
};