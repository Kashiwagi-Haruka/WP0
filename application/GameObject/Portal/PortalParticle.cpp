#include "PortalParticle.h"
#include "GameObject/YoshidaMath/YoshidaMath.h"
#include "ParticleEmitter.h"
#include "ParticleManager.h"
#include <algorithm>
#include <string>

uint32_t PortalParticle::nextId_ = 0;

PortalParticle::PortalParticle() {
	ParticleManager::GetInstance()->CreateParticleGroup("portalBall", "Resources/2d/defaultParticle.png");

	emitter_ = std::make_unique<ParticleEmitter>("portalBall");
	emitter_->SetFrequency(0.008f);
	emitter_->SetCount(100);
	emitter_->SetLife(1.0f);
	emitter_->SetAcceleration({0.0f, 0.0f, 0.0f});
	emitter_->SetAreaMin({-0.5f, -0.5f, -0.5f});
	emitter_->SetAreaMax({0.5f, 0.5f, 0.5f});
	emitter_->SetBeforeColor({0.4f, 0.7f, 1.0f, 1.0f});
	emitter_->SetAfterColor({0.2f, 1.0f, 0.3f, 0.0f});
	emitter_->SetEmissionAngle(2.0f * 3.1415926535f);

	const std::string pathGroup = "portalPath" + std::to_string(nextId_++);
	ParticleManager::GetInstance()->CreateParticleGroup(pathGroup, "Resources/2d/defaultParticle.png");
	pathEmitter_ = std::make_unique<ParticleEmitter>(pathGroup);
	pathEmitter_->SetFrequency(0.004f);
	pathEmitter_->SetCount(100);
	pathEmitter_->SetLife(1.0f);
	pathEmitter_->SetAcceleration({0.0f, 0.0f, 0.0f});
	pathEmitter_->SetAreaMin({-0.12f, -0.12f, -0.12f});
	pathEmitter_->SetAreaMax({0.12f, 0.12f, 0.12f});
	pathEmitter_->SetBeforeColor({0.4f, 0.7f, 1.0f, 1.0f});
	pathEmitter_->SetAfterColor({0.2f, 1.0f, 0.3f, 0.0f});
	pathEmitter_->SetEmissionAngle(2.0f * 3.1415926535f);

	primitive_ = std::make_unique<Primitive>();
	primitive_->Initialize(Primitive::Sphere);
	primitive_->SetEnableLighting(false);
	primitive_->SetColor({0.2f, 0.6f, 1.0f, 1.0f});
	transform_.scale = {0.1f, 0.1f, 0.1f};
	primitive_->SetScale(transform_.scale);


	particleTransform_.scale = {0.01f, 0.01f, 0.01f};
}

void PortalParticle::Initialize() {}

void PortalParticle::SetCamera(Camera* camera) {
	if (primitive_) {
		primitive_->SetCamera(camera);
	}
	ParticleManager::GetInstance()->SetCamera(camera);
}

void PortalParticle::Start(const Vector3& from, const Vector3& to) {
	from_ = from;
	to_ = to;
	effectTimer_ = 0.0f;
	isActive_ = true;
	isVisible_ = true;
	if (pathEmitter_) {
		Transform pathTransform{};
		pathTransform.scale = particleTransform_.scale;
		pathTransform.translate = from_;
		pathEmitter_->SetTransform(pathTransform);
		pathEmitter_->Emit();
	}
	if (emitter_) {
		particleTransform_.translate = from_;
		emitter_->SetTransform(particleTransform_);
		emitter_->Emit();
	}
	Update();
}

void PortalParticle::Update() {
	if (!isVisible_) {
		return;
	}

	effectTimer_ += YoshidaMath::kDeltaTime;
	if (effectTimer_ >= effectDuration_) {
		effectTimer_ = effectDuration_;
		isActive_ = false;
		isVisible_ = false;
	}

	const float t = std::clamp(effectTimer_ / effectDuration_, 0.0f, 1.0f);
	const Vector3 beamCurrent = {
	    from_.x + (to_.x - from_.x) * t,
	    from_.y + (to_.y - from_.y) * t,
	    from_.z + (to_.z - from_.z) * t,
	};
	if (primitive_) {
		primitive_->SetEnableLighting(false);
		primitive_->SetScale(transform_.scale);
		primitive_->SetTranslate(beamCurrent);
		primitive_->Update();
	}

	if (pathEmitter_) {
		Transform pathTransform{};
		pathTransform.scale = particleTransform_.scale;
		constexpr float kRingRadius = 0.18f;
		constexpr float kVerticalRadius = 0.08f;
		constexpr float kRingAngularSpeed = 24.0f;
		const float swirlAngle = effectTimer_ * kRingAngularSpeed;
		Vector3 ringOffset{
		    std::cos(swirlAngle) * kRingRadius,
		    std::sin(swirlAngle * 1.7f) * kVerticalRadius,
		    std::sin(swirlAngle) * kRingRadius,
		};

		pathTransform.translate = {
		    beamCurrent.x + ringOffset.x,
		    beamCurrent.y + ringOffset.y,
		    beamCurrent.z + ringOffset.z,
		};
		pathEmitter_->SetTransform(pathTransform);
		pathEmitter_->Emit();
		pathEmitter_->Update(pathTransform);
	}

	transform_.translate = to_;
	particleTransform_.translate = beamCurrent;

	if (emitter_) {
		emitter_->SetTransform(particleTransform_);
		emitter_->Emit();
		emitter_->Update(particleTransform_);
	}
}

void PortalParticle::Draw() {
	if (!isVisible_) {
		return;
	}


	if (primitive_) {
		primitive_->Draw();
	}

	if (pathEmitter_) {
		pathEmitter_->Draw();
	}

	if (emitter_) {
		emitter_->Draw();
	}
}

bool PortalParticle::IsFinished() const { return !isVisible_ && effectTimer_ >= effectDuration_; }