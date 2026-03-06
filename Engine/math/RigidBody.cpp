#define NOMINMAX
#include "RigidBody.h"
#include "Function.h"
#include <algorithm>

namespace RigidBody {
bool IsCollision(const AABB& aabb, const Vector3& point) {
	return (point.x >= aabb.min.x && point.x <= aabb.max.x && point.y >= aabb.min.y && point.y <= aabb.max.y && point.z >= aabb.min.z && point.z <= aabb.max.z);
}
bool isCollision(const AABB& aabb1, const AABB& aabb2) {
	// 各軸で重なっているか確認（Separating Axis Theoremの基本）
	if (aabb1.max.x < aabb2.min.x || aabb1.min.x > aabb2.max.x) {

		return false;
	}
	if (aabb1.max.y < aabb2.min.y || aabb1.min.y > aabb2.max.y) {

		return false;
	}
	if (aabb1.max.z < aabb2.min.z || aabb1.min.z > aabb2.max.z) {

		return false;
	}

	// 全軸で重なっているので当たっている
	return true;
}
bool isCollision(const AABB& aabb, const Sphere& sphere) {
	// 最近接点をAABB内から計算（クランプ）
	Vector3 closestPoint;

	closestPoint.x = std::clamp(sphere.center.x, aabb.min.x, aabb.max.x);
	closestPoint.y = std::clamp(sphere.center.y, aabb.min.y, aabb.max.y);
	closestPoint.z = std::clamp(sphere.center.z, aabb.min.z, aabb.max.z);

	// 最近接点と球の中心の距離の2乗を計算
	Vector3 difference = sphere.center - closestPoint;
	float distanceSquared = Function::Dot(difference, difference);

	// 球の半径の2乗と比較
	return distanceSquared <= (sphere.radius * sphere.radius);
}
bool isCollision(const AABB& aabb, const Segment& segment) {
	// 線分の始点と終点
	Vector3 p0 = segment.origin;
	Vector3 p1 = segment.diff;

	// 線分の各軸でのtの最小・最大値
	float tmin = 0.0f;
	float tmax = 1.0f;

	// 各軸ごとに計算
	for (int i = 0; i < 3; ++i) {
		float segStart = (&p0.x)[i];
		float segEnd = (&p1.x)[i];
		float aabbMin = (&aabb.min.x)[i];
		float aabbMax = (&aabb.max.x)[i];

		float d = segEnd - segStart;
		if (fabs(d) < 1e-6f) {
			// 線分が平行な場合：始点がAABB内にあるか
			if (segStart < aabbMin || segStart > aabbMax) {
				return false;
			}
		} else {
			// t0, t1: 線分がAABBのスラブを通過する割合
			float t0 = (aabbMin - segStart) / d;
			float t1 = (aabbMax - segStart) / d;
			if (t0 > t1)
				std::swap(t0, t1);
			tmin = std::max(tmin, t0);
			tmax = std::min(tmax, t1);
			// 重なりがなければ当たらない
			if (tmin > tmax) {
				return false;
			}
		}
	}
	// tmin <= tmaxならAABBと線分は交差している
	return true;
}

void DrawAABB(const AABB& aabb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {

	// 8頂点定義（ワールド座標）
	Vector3 vertices[8] = {
	    {aabb.min.x, aabb.min.y, aabb.min.z},
        {aabb.max.x, aabb.min.y, aabb.min.z},
        {aabb.min.x, aabb.max.y, aabb.min.z},
        {aabb.max.x, aabb.max.y, aabb.min.z},
	    {aabb.min.x, aabb.min.y, aabb.max.z},
        {aabb.max.x, aabb.min.y, aabb.max.z},
        {aabb.min.x, aabb.max.y, aabb.max.z},
        {aabb.max.x, aabb.max.y, aabb.max.z},
	};

	// 変換行列合成
	Matrix4x4 vpVpMatrix = Function::Multiply(viewProjectionMatrix, viewportMatrix);

	// 変換してスクリーン座標へ
	for (int i = 0; i < 8; ++i) {
		vertices[i] = Function::TransformVM(vertices[i], vpVpMatrix);
	}

	// 辺を線で描画
	int lines[12][2] = {
	    {0, 1},
        {1, 3},
        {3, 2},
        {2, 0}, // 前面
	    {4, 5},
        {5, 7},
        {7, 6},
        {6, 4}, // 背面
	    {0, 4},
        {1, 5},
        {2, 6},
        {3, 7}  // 側面
	};
	(uint32_t)color;
	// for (int i = 0; i < 12; ++i) {
	//	const Vector3& v0 = vertices[lines[i][0]];
	//	const Vector3& v1 = vertices[lines[i][1]];
	//	Novice::DrawLine(static_cast<int>(v0.x), static_cast<int>(v0.y), static_cast<int>(v1.x), static_cast<int>(v1.y), color);
	// }
}
} // namespace RigidBody