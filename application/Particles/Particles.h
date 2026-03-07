#pragma once
#include "ParticleEmitter.h"
#include <memory>
class Particles {

	std::unique_ptr<ParticleEmitter> particleArrow = nullptr;

	Vector3 playerPos_;
	Vector3 cameraPos_;
	Vector3 goalPos_;
	Transform playerEmitterTransform;

	bool isgoal;

public:
	Particles();
	~Particles();
	void Update();
	void Draw();
	void SetPlayerPos(Vector3 playerPos);
	void SetCameraPos(Vector3 cameraPos);
	void SetGoalPos(Vector3 goalPos);
	void EditSingleEmitter(ParticleEmitter* e);
	bool Isgoal(bool isgoal);
};
