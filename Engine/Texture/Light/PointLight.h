#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include <cstddef>
#include <cstdint>
constexpr size_t kMaxPointLights = 20;
struct PointLight {
	Vector4 color;
	Vector3 position;
	float intensity;
	float radius;
	float decay;
	float padding[2];
};
struct PointLightCount {
	uint32_t count;
	float padding[3];
};
struct PointLightSet {
	PointLight lights[kMaxPointLights];
	int count;
	float padding[3];
};