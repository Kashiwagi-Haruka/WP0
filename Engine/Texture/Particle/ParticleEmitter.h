#pragma once
#include "Transform.h"
#include "Vector3.h"
#include "Vector4.h"
#include <cstdint>
#include <string>

class ParticleEmitter {
public:
	// コンストラクタ
	ParticleEmitter(const std::string& groupName);

	// 更新
	void Update(const Transform& transform);
	void Draw();
	// 発生
	void Emit();
	void EmitVisible(bool v);

	void SetFrequency(float frequency);
	void SetCount(uint32_t count);
	void SetAcceleration(Vector3 acceleration);
	void SetAreaMin(Vector3 areaMin);
	void SetAreaMax(Vector3 areaMax);
	void SetEmissionAngle(float emissionAngle);
	float GetFrequency() { return frequency; }
	uint32_t GetCount() { return count; }
	Vector3 GetAcceleration() { return acceleration_; }
	Vector3 GetAreaMin() { return areaMin_; }
	Vector3 GetAreaMax() { return areaMax_; }
	float GetEmissionAngle() const { return emissionAngle_; }
	// === Transform Getter / Setter ===
	Transform& GetTransformRef() { return transform_; }
	const Transform& GetTransform() const { return transform_; }

	void SetTransform(const Transform& t) { transform_ = t; }
	void SetLife(float life) { this->life = life; }
	void SetBeforeColor(Vector4 color) { beforeColor_ = color; }
	void SetAfterColor(Vector4 color) { afterColor_ = color; }
	Vector4 GetBeforeColor() const { return beforeColor_; }
	Vector4 GetAfterColor() const { return afterColor_; }
	float GetLife() const { return life; }

private:
	// メンバ変数（初期値はヘッダーで設定、必要値はSetterで変更）
	std::string name;
	Transform transform_{};
	float frequency = 0.1f;            // 秒間またはフレームごとの発生頻度
	uint32_t count = 1;                // 一回のEmitで発生する数
	Vector3 acceleration_ = {0, 0, 0}; // 加速度
	Vector3 areaMin_ = {0, 0, 0};      // 発生エリア最小座標
	Vector3 areaMax_ = {1, 1, 1};      // 発生エリア最大座標
	bool emitVisible_ = true;
	float timer = 0.0f; // 発生管理用のタイマー
	float life = 1.0f;
	Vector4 beforeColor_ = {1, 1, 1, 1};
	Vector4 afterColor_ = {1, 1, 1, 0};
	float emissionAngle_ = 2.0f * 3.1415926535f;
};