#pragma once
#include "Matrix4x4.h"
#include "Vector3.h"
#include <cstdint>

struct Line /*直線*/ {
	Vector3 origin; //!< 始点
	Vector3 diff;   //!< 終点への差分ベクトル
};
struct Ray /*半直線*/ {

	Vector3 origin; //!< 始点
	Vector3 diff;   //!< 終点への差分ベクトル
};
struct Segment /*線分*/ {

	Vector3 origin; //!< 始点
	Vector3 diff;   //!< 終点への差分ベクトル
};
struct Sphere {
	Vector3 center;
	float radius;
};

struct Plane {
	Vector3 normal;
	float distance;
};
struct AABB {
	Vector3 min;
	Vector3 max;
};

namespace RigidBody {

// -----------------------------------------
// AABB と 点 の当たり判定
// -----------------------------------------
bool IsCollision(const AABB& aabb, const Vector3& point);

bool isCollision(const AABB& aabb1, const AABB& aabb2);
bool isCollision(const AABB& aabb, const Sphere& sphere);
bool isCollision(const AABB& aabb, const Segment& segment);
void DrawAABB(const AABB& aabb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color);

}; // namespace RigidBody
