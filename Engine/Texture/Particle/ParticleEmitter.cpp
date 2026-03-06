#include "ParticleEmitter.h"
#include "Function.h"
#include "ParticleManager.h"
// -----------------------------------------
// コンストラクタ
// -----------------------------------------
ParticleEmitter::ParticleEmitter(const std::string& groupName) : name(groupName) {}

void ParticleEmitter::Update(const Transform& parentTransform) {
	transform_ = parentTransform;

	timer += frequency;
	while (timer >= 1.0f) {

		// Emitter 専用の発生エリア
		AABB fieldArea;
		fieldArea.min = areaMin_;
		fieldArea.max = areaMax_;

		// ★ SetFieldAcceleration はもう使わない（全体共有なので）
		// ★ SetFieldArea も使わない

		// Emit() に emitter の個別フィールドを渡す
		ParticleManager::GetInstance()->Emit(name, transform_, count, acceleration_, fieldArea, life, beforeColor_, afterColor_, emissionAngle_);

		timer -= 1.0f;
	}
}
void ParticleEmitter::Draw() {
	if (!emitVisible_) {
		return;
	}
	ParticleManager::GetInstance()->Draw(name);
}
// -----------------------------------------
// EmitVisible
// -----------------------------------------
void ParticleEmitter::EmitVisible(bool v) { emitVisible_ = v; }

// -----------------------------------------
// Emit
// -----------------------------------------
void ParticleEmitter::Emit() {
	AABB fieldArea;
	fieldArea.min = areaMin_;
	fieldArea.max = areaMax_;

	ParticleManager::GetInstance()->Emit(name, transform_, count, acceleration_, fieldArea, life, beforeColor_, afterColor_, emissionAngle_);
}

void ParticleEmitter::SetAcceleration(Vector3 acceleration) { acceleration_ = acceleration; }
void ParticleEmitter::SetCount(uint32_t cou) { count = cou; }
void ParticleEmitter::SetFrequency(float fre) { frequency = fre; }
void ParticleEmitter::SetAreaMin(Vector3 areamin) { areaMin_ = areamin; }
void ParticleEmitter::SetAreaMax(Vector3 areamax) { areaMax_ = areamax; }
void ParticleEmitter::SetEmissionAngle(float emissionAngle) { emissionAngle_ = emissionAngle; }