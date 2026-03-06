#pragma once
#include "Primitive/Primitive.h"
#include "Transform.h"
#include <cstdint>
#include <memory>
#include <string>
#include "Particle/ParticleEmitter.h"

class Camera;

class PortalParticle {
public:
	PortalParticle();
	~PortalParticle() = default;

	void Initialize();
	void SetCamera(Camera* camera);
	void Start(const Vector3& from, const Vector3& to);
	void Update();
	void Draw();
	bool IsActive() const { return isActive_; }
	bool IsVisible() const { return isVisible_; }
	bool IsFinished() const;

private:
	std::unique_ptr<Primitive> primitive_ = nullptr;
	std::unique_ptr<ParticleEmitter> emitter_ = nullptr;
	std::unique_ptr<ParticleEmitter> pathEmitter_ = nullptr;
	Transform transform_{};
	Transform particleTransform_{};
	Vector3 from_{};
	Vector3 to_{};
	float effectTimer_ = 0.0f;
	float effectDuration_ = 0.28f;
	bool isActive_ = false;
	bool isVisible_ = false;
	static uint32_t nextId_;
};