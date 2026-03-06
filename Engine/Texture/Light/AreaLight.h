#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include <cstddef>
#include <cstdint>
constexpr size_t kMaxAreaLights = 20;
struct AreaLight {
	Vector4 color;
	Vector3 position;
	float intensity;
	Vector3 normal;
	float width;
	float height;
	float radius;
	float decay;
	float padding;
};
struct AreaLightCount {
	uint32_t count;
	float padding[3];
};