#pragma once
#include "Vector3.h"
#include "Vector4.h"
struct QuaternionTransform {
	Vector3 scale;
	Vector4 quaternion;
	Vector3 translate;
};