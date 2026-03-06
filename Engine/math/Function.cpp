#include "Function.h"
#include <cassert>
#include <cmath>

static const int kColumnWidth = 60;
static const int kRowHeight = 30;

namespace Function {

// ベクトルの長さを計算する
float Length(const Vector3& v) { return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }
// 長さの二乗
float LengthSquared(const Vector3& v) { return v.x * v.x + v.y * v.y + v.z * v.z; }
// direction = 向きたい方向（正規化推奨）
// forwardAxis = モデルの前方向（Cube は X軸 {1,0,0}）
// return = 回転角（x, y, z）
// 方向ベクトルからピッチ・ヨー・ロールを計算する
Vector3 DirectionToRotation(const Vector3& direction, const Vector3& forwardAxis) {
	// 方向を正規化
	Vector3 dir = Normalize(direction);

	// --- Yaw（水平回転）---
	// atan2(y, x) ではなく、x, z を使用
	float yaw = std::atan2(dir.z, dir.x);

	// --- Pitch（上下の角度）---
	float horizontalLen = std::sqrt(dir.x * dir.x + dir.z * dir.z);
	float pitch = std::atan2(dir.y, horizontalLen);

	// Roll は不要なので 0
	float roll = 0.0f;

	return {pitch, -yaw, roll};
}

// Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
//	Matrix4x4 result{};
//	float f = 1.0f / std::tanf(fovY * 0.5f);
//	result.m[0][0] = f / aspectRatio;
//	result.m[1][1] = f;
//	result.m[2][2] = farClip / (farClip - nearClip);
//	result.m[2][3] = 1.0f;
//	result.m[3][2] = (-nearClip * farClip) / (farClip - nearClip);
//	result.m[3][3] = 0.0f;
//	return result;
// }
// 透視投影行列を作る
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspect, float nearZ, float farZ) {
	Matrix4x4 result{};
	float f = 1.0f / tanf(fovY * 0.5f);

	result.m[0][0] = f / aspect;
	result.m[1][1] = f;
	result.m[2][2] = farZ / (farZ - nearZ);
	result.m[2][3] = 1.0f;
	result.m[3][2] = (-nearZ * farZ) / (farZ - nearZ);
	result.m[3][3] = 0.0f;

	return result;
}

// 正射影行列を作る
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
	Matrix4x4 result{};
	result.m[0][0] = 2.0f / (right - left);
	result.m[1][1] = 2.0f / (top - bottom);
	result.m[2][2] = 1.0f / (farClip - nearClip);
	result.m[3][0] = -(right + left) / (right - left);
	result.m[3][1] = -(top + bottom) / (top - bottom);
	result.m[3][2] = -nearClip / (farClip - nearClip);
	result.m[3][3] = 1.0f;
	return result;
}
// ビューポート変換行列を作る
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {

	Matrix4x4 result{};
	result.m[0][0] = width / 2.0f;
	result.m[1][1] = -height / 2.0f;
	result.m[2][2] = maxDepth - minDepth;
	result.m[3][0] = left + width / 2.0f;
	result.m[3][1] = top + height / 2.0f;
	result.m[3][2] = minDepth;
	result.m[3][3] = 1.0f;
	return result;
}

// Vector3を線形補間する
Vector3 Lerp(const Vector3& start, const Vector3& end, float ratio) {
	Vector3 result;
	result.x = start.x + (end.x - start.x) * ratio;
	result.y = start.y + (end.y - start.y) * ratio;
	result.z = start.z + (end.z - start.z) * ratio;
	return result;
}
// floatを線形補間する
float Lerp(float start, float end, float ratio) { return start + (end - start) * ratio; }
// 平行移動行列を作る
Matrix4x4 MakeTranslateMatrix(Vector3 translate) {
	Matrix4x4 result{};
	result.m[0][0] = 1.0f;
	result.m[1][1] = 1.0f;
	result.m[2][2] = 1.0f;
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	result.m[3][3] = 1.0f;

	return result;
}
// 平行移動行列を作る（スカラー指定）
Matrix4x4 Function::MakeTranslateMatrix(float x, float y, float z) {
	Matrix4x4 result{};

	result.m[0][0] = 1.0f;
	result.m[1][1] = 1.0f;
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;

	result.m[3][0] = x;
	result.m[3][1] = y;
	result.m[3][2] = z;

	return result;
}

// 拡大縮小行列を作る
Matrix4x4 MakeScaleMatrix(Vector3 scale) {
	Matrix4x4 result{};
	result.m[0][0] = scale.x;
	result.m[1][1] = scale.y;
	result.m[2][2] = scale.z;
	result.m[3][3] = 1.0f;

	return result;
}

// X軸回転行列を作る
Matrix4x4 MakeRotateXMatrix(float radian) {
	Matrix4x4 result{};
	result.m[0][0] = 1.0f;
	result.m[1][1] = std::cosf(radian);
	result.m[1][2] = std::sinf(radian);
	result.m[2][1] = -std::sinf(radian);
	result.m[2][2] = std::cosf(radian);
	result.m[3][3] = 1.0f;
	return result;
}

// Y軸回転行列を作る
Matrix4x4 MakeRotateYMatrix(float radian) {
	Matrix4x4 result{};
	result.m[0][0] = std::cosf(radian);
	result.m[0][2] = -std::sinf(radian);
	result.m[1][1] = 1.0f;
	result.m[2][0] = std::sinf(radian);
	result.m[2][2] = std::cosf(radian);
	result.m[3][3] = 1.0f;
	return result;
}

// Z軸回転行列を作る
Matrix4x4 MakeRotateZMatrix(float radian) {
	Matrix4x4 result{};
	result.m[0][0] = std::cosf(radian);
	result.m[0][1] = std::sinf(radian);
	result.m[1][0] = -std::sinf(radian);
	result.m[1][1] = std::cosf(radian);
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;
	return result;
}

// クォータニオンから回転行列を作る
Matrix4x4 MakeRotateMatrix(const Vector4& rotate) {
	Matrix4x4 result{};
	float length = std::sqrt(rotate.x * rotate.x + rotate.y * rotate.y + rotate.z * rotate.z + rotate.w * rotate.w);
	Vector4 q = rotate;
	if (length > 0.0f) {
		q.x /= length;
		q.y /= length;
		q.z /= length;
		q.w /= length;
	}

	float xx = q.x * q.x;
	float yy = q.y * q.y;
	float zz = q.z * q.z;
	float xy = q.x * q.y;
	float xz = q.x * q.z;
	float yz = q.y * q.z;
	float wx = q.w * q.x;
	float wy = q.w * q.y;
	float wz = q.w * q.z;

	result.m[0][0] = 1.0f - 2.0f * (yy + zz);
	result.m[0][1] = 2.0f * (xy + wz);
	result.m[0][2] = 2.0f * (xz - wy);
	result.m[0][3] = 0.0f;

	result.m[1][0] = 2.0f * (xy - wz);
	result.m[1][1] = 1.0f - 2.0f * (xx + zz);
	result.m[1][2] = 2.0f * (yz + wx);
	result.m[1][3] = 0.0f;

	result.m[2][0] = 2.0f * (xz + wy);
	result.m[2][1] = 2.0f * (yz - wx);
	result.m[2][2] = 1.0f - 2.0f * (xx + yy);
	result.m[2][3] = 0.0f;

	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;

	return result;
}

// 4x4行列同士を乗算する
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result{};
	for (int row = 0; row < 4; ++row) {
		for (int colum = 0; colum < 4; ++colum) {
			result.m[row][colum] = m1.m[row][0] * m2.m[0][colum] + m1.m[row][1] * m2.m[1][colum] + m1.m[row][2] * m2.m[2][colum] + m1.m[row][3] * m2.m[3][colum];
		}
	}
	return result;
}

// 外積を計算する
Vector3 Cross(const Vector3& v1, const Vector3& v2) {
	Vector3 result;
	result.x = v1.y * v2.z - v1.z * v2.y;
	result.y = v1.z * v2.x - v1.x * v2.z;
	result.z = v1.x * v2.y - v1.y * v2.x;
	return result;
}

// SRT行列を作る（オイラー角）
Matrix4x4 MakeAffineMatrix(Vector3 scale, Vector3 rotate, Vector3 translate) { return MakeAffineMatrix(scale, rotate, translate, {0.0f, 0.0f}); }

// SRT行列を作る（オイラー角・アンカー指定）
Matrix4x4 MakeAffineMatrix(Vector3 scale, Vector3 rotate, Vector3 translate, Vector2 anchor) {
	Matrix4x4 result{};

	Matrix4x4 rotateX = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateY = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZ = MakeRotateZMatrix(rotate.z);
	Matrix4x4 rotateXYZ = Multiply(rotateZ, Multiply(rotateX, rotateY));

	Matrix4x4 scaleRotateMatrix = Multiply(MakeScaleMatrix(scale), rotateXYZ);
	Matrix4x4 toAnchorMatrix = MakeTranslateMatrix(-anchor.x, -anchor.y, 0.0f);
	Matrix4x4 fromAnchorMatrix = MakeTranslateMatrix(anchor.x, anchor.y, 0.0f);

	result = Multiply(Multiply(toAnchorMatrix, scaleRotateMatrix), Multiply(fromAnchorMatrix, MakeTranslateMatrix(translate)));

	return result;
}

// SRT行列を作る（クォータニオン）
Matrix4x4 MakeAffineMatrix(Vector3 scale, Vector4 rotate, Vector3 translate) {
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
	Matrix4x4 rotateMatrix = MakeRotateMatrix(rotate);
	Matrix4x4 scaleRotateMatrix = Multiply(scaleMatrix, rotateMatrix);
	return Multiply(scaleRotateMatrix, MakeTranslateMatrix(translate));
}

// ベクトルを行列で変換する
Vector3 TransformVM(const Vector3& vector, const Matrix4x4& matrix4x4) {
	Vector3 result;
	result.x = vector.x * matrix4x4.m[0][0] + vector.y * matrix4x4.m[1][0] + vector.z * matrix4x4.m[2][0] + matrix4x4.m[3][0];
	result.y = vector.x * matrix4x4.m[0][1] + vector.y * matrix4x4.m[1][1] + vector.z * matrix4x4.m[2][1] + matrix4x4.m[3][1];
	result.z = vector.x * matrix4x4.m[0][2] + vector.y * matrix4x4.m[1][2] + vector.z * matrix4x4.m[2][2] + matrix4x4.m[3][2];
	float w = vector.x * matrix4x4.m[0][3] + vector.y * matrix4x4.m[1][3] + vector.z * matrix4x4.m[2][3] + matrix4x4.m[3][3];

	// wが0なら変換できない（カメラの視野外など）
	if (w != 0.0f) {
		result.x /= w;
		result.y /= w;
		result.z /= w;
	} else {
		// 代替処理（例：そのまま返す・0で埋める・巨大な数を入れるなど）
		result = {0.0f, 0.0f, 0.0f}; // 安全に落とす場合
	}

	return result;
}
// 4x4行列の逆行列を求める
Matrix4x4 Inverse(const Matrix4x4& m) {
	Matrix4x4 i;

	float d =
	    m.m[0][0] *
	        (m.m[1][1] * (m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2]) - m.m[1][2] * (m.m[2][1] * m.m[3][3] - m.m[2][3] * m.m[3][1]) + m.m[1][3] * (m.m[2][1] * m.m[3][2] - m.m[2][2] * m.m[3][1])) -
	    m.m[0][1] *
	        (m.m[1][0] * (m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2]) - m.m[1][2] * (m.m[2][0] * m.m[3][3] - m.m[2][3] * m.m[3][0]) + m.m[1][3] * (m.m[2][0] * m.m[3][2] - m.m[2][2] * m.m[3][0])) +
	    m.m[0][2] *
	        (m.m[1][0] * (m.m[2][1] * m.m[3][3] - m.m[2][3] * m.m[3][1]) - m.m[1][1] * (m.m[2][0] * m.m[3][3] - m.m[2][3] * m.m[3][0]) + m.m[1][3] * (m.m[2][0] * m.m[3][1] - m.m[2][1] * m.m[3][0])) -
	    m.m[0][3] *
	        (m.m[1][0] * (m.m[2][1] * m.m[3][2] - m.m[2][2] * m.m[3][1]) - m.m[1][1] * (m.m[2][0] * m.m[3][2] - m.m[2][2] * m.m[3][0]) + m.m[1][2] * (m.m[2][0] * m.m[3][1] - m.m[2][1] * m.m[3][0]));

	i.m[0][0] =
	    (m.m[1][1] * (m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2]) - m.m[1][2] * (m.m[2][1] * m.m[3][3] - m.m[2][3] * m.m[3][1]) + m.m[1][3] * (m.m[2][1] * m.m[3][2] - m.m[2][2] * m.m[3][1])) / d;
	i.m[0][1] =
	    -(m.m[0][1] * (m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2]) - m.m[0][2] * (m.m[2][1] * m.m[3][3] - m.m[2][3] * m.m[3][1]) + m.m[0][3] * (m.m[2][1] * m.m[3][2] - m.m[2][2] * m.m[3][1])) / d;
	i.m[0][2] =
	    (m.m[0][1] * (m.m[1][2] * m.m[3][3] - m.m[1][3] * m.m[3][2]) - m.m[0][2] * (m.m[1][1] * m.m[3][3] - m.m[1][3] * m.m[3][1]) + m.m[0][3] * (m.m[1][1] * m.m[3][2] - m.m[1][2] * m.m[3][1])) / d;
	i.m[0][3] =
	    -(m.m[0][1] * (m.m[1][2] * m.m[2][3] - m.m[1][3] * m.m[2][2]) - m.m[0][2] * (m.m[1][1] * m.m[2][3] - m.m[1][3] * m.m[2][1]) + m.m[0][3] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1])) / d;

	i.m[1][0] =
	    -(m.m[1][0] * (m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2]) - m.m[1][2] * (m.m[2][0] * m.m[3][3] - m.m[2][3] * m.m[3][0]) + m.m[1][3] * (m.m[2][0] * m.m[3][2] - m.m[2][2] * m.m[3][0])) / d;
	i.m[1][1] =
	    (m.m[0][0] * (m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2]) - m.m[0][2] * (m.m[2][0] * m.m[3][3] - m.m[2][3] * m.m[3][0]) + m.m[0][3] * (m.m[2][0] * m.m[3][2] - m.m[2][2] * m.m[3][0])) / d;
	i.m[1][2] =
	    -(m.m[0][0] * (m.m[1][2] * m.m[3][3] - m.m[1][3] * m.m[3][2]) - m.m[0][1] * (m.m[1][0] * m.m[3][3] - m.m[1][3] * m.m[3][0]) + m.m[0][3] * (m.m[1][0] * m.m[3][2] - m.m[1][2] * m.m[3][0])) / d;
	i.m[1][3] =
	    (m.m[0][0] * (m.m[1][2] * m.m[2][3] - m.m[1][3] * m.m[2][2]) - m.m[0][1] * (m.m[1][0] * m.m[2][3] - m.m[1][3] * m.m[2][0]) + m.m[0][2] * (m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0])) / d;

	i.m[2][0] =
	    (m.m[1][0] * (m.m[2][1] * m.m[3][3] - m.m[2][3] * m.m[3][1]) - m.m[1][1] * (m.m[2][0] * m.m[3][3] - m.m[2][3] * m.m[3][0]) + m.m[1][3] * (m.m[2][0] * m.m[3][1] - m.m[2][1] * m.m[3][0])) / d;
	i.m[2][1] =
	    -(m.m[0][0] * (m.m[2][1] * m.m[3][3] - m.m[2][3] * m.m[3][1]) - m.m[0][1] * (m.m[2][0] * m.m[3][3] - m.m[2][3] * m.m[3][0]) + m.m[0][3] * (m.m[2][0] * m.m[3][1] - m.m[2][1] * m.m[3][0])) / d;
	i.m[2][2] =
	    (m.m[0][0] * (m.m[1][1] * m.m[3][3] - m.m[1][3] * m.m[3][1]) - m.m[0][1] * (m.m[1][0] * m.m[3][3] - m.m[1][3] * m.m[3][0]) + m.m[0][3] * (m.m[1][0] * m.m[3][1] - m.m[1][1] * m.m[3][0])) / d;
	i.m[2][3] =
	    -(m.m[0][0] * (m.m[1][1] * m.m[2][3] - m.m[1][3] * m.m[2][1]) - m.m[0][1] * (m.m[1][0] * m.m[2][3] - m.m[1][3] * m.m[2][0]) + m.m[0][2] * (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0])) / d;

	i.m[3][0] =
	    -(m.m[1][0] * (m.m[2][1] * m.m[3][2] - m.m[2][2] * m.m[3][1]) - m.m[1][1] * (m.m[2][0] * m.m[3][2] - m.m[2][2] * m.m[3][0]) + m.m[1][2] * (m.m[2][0] * m.m[3][1] - m.m[2][1] * m.m[3][0])) / d;
	i.m[3][1] =
	    (m.m[0][0] * (m.m[2][1] * m.m[3][2] - m.m[2][2] * m.m[3][1]) - m.m[0][1] * (m.m[2][0] * m.m[3][2] - m.m[2][2] * m.m[3][0]) + m.m[0][2] * (m.m[2][0] * m.m[3][1] - m.m[2][1] * m.m[3][0])) / d;
	i.m[3][2] =
	    -(m.m[0][0] * (m.m[1][1] * m.m[3][2] - m.m[1][2] * m.m[3][1]) - m.m[0][1] * (m.m[1][0] * m.m[3][2] - m.m[1][2] * m.m[3][0]) + m.m[0][2] * (m.m[1][0] * m.m[3][1] - m.m[1][1] * m.m[3][0])) / d;
	i.m[3][3] =
	    (m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]) - m.m[0][1] * (m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0]) + m.m[0][2] * (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0])) / d;

	return i;
}

// 4x4行列を転置する
Matrix4x4 Transpose(const Matrix4x4& m) {
	Matrix4x4 result;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.m[i][j] = m.m[j][i];
		}
	}
	return result;
}

// ベクトルを正規化する
Vector3 Normalize(const Vector3& v) {
	float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	if (len > 1e-6f)
		return {v.x / len, v.y / len, v.z / len};
	else
		return {0.0f, 0.0f, 0.0f};
}

// 単位行列を作る
Matrix4x4 MakeIdentity4x4() {

	Matrix4x4 result;

	result = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

	return result;
}
// 内積を計算する
float Dot(const Vector3& v1, const Vector3& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }

// 2点間の差分ベクトルを返す
Vector3 Distance(const Vector3& pos1, const Vector3& pos2) { return {pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z}; }

// クォータニオンを正規化する
Vector4 NormalizeQuaternion(const Vector4& q) {
	float len = std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
	if (len <= 1e-6f) {
		return {0.0f, 0.0f, 0.0f, 1.0f};
	}
	return {q.x / len, q.y / len, q.z / len, q.w / len};
}

// クォータニオン積を計算する
Vector4 MultiplyQuaternion(const Vector4& q1, const Vector4& q2) {
	return {
	    q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y, q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x, q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
	    q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z};
}

// クォータニオン共役を返す
Vector4 ConjugateQuaternion(const Vector4& q) { return {-q.x, -q.y, -q.z, q.w}; }

// 軸角からクォータニオンを作る
Vector4 MakeQuaternionFromAxisAngle(const Vector3& axis, float radian) {
	Vector3 n = Normalize(axis);
	float half = radian * 0.5f;
	float s = std::sinf(half);
	return NormalizeQuaternion({n.x * s, n.y * s, n.z * s, std::cosf(half)});
}

// クォータニオンでベクトルを回転する
Vector3 RotateVectorByQuaternion(const Vector3& v, const Vector4& q) {
	Vector4 nq = NormalizeQuaternion(q);
	Vector4 vq{v.x, v.y, v.z, 0.0f};
	Vector4 rq = MultiplyQuaternion(MultiplyQuaternion(nq, vq), ConjugateQuaternion(nq));
	return {rq.x, rq.y, rq.z};
}

} // namespace Function
// Vector3加算
Vector3 operator+(const Vector3& v1, const Vector3& v2) { return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z}; }
// Vector3減算
Vector3 operator-(const Vector3& v1, const Vector3& v2) { return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z}; }
// Vector3単項マイナス
Vector3 operator-(const Vector3& v) { return {-v.x, -v.y, -v.z}; }
// Vector3スカラー倍
Vector3 operator*(const Vector3& v, float scalar) { return {v.x * scalar, v.y * scalar, v.z * scalar}; }
// Vector3スカラー倍（左項）
Vector3 operator*(float scalar, const Vector3& v) { return v * scalar; }
// Vector3スカラー除算
Vector3 operator/(const Vector3& v, float scalar) {
	assert(std::fabs(scalar) > 1e-6f);
	float inv = 1.0f / scalar;
	return {v.x * inv, v.y * inv, v.z * inv};
}
// Vector3加算代入
Vector3& operator+=(Vector3& v1, const Vector3& v2) {
	v1 = v1 + v2;
	return v1;
}
// Vector3減算代入
Vector3& operator-=(Vector3& v1, const Vector3& v2) {
	v1 = v1 - v2;
	return v1;
}
// Vector3スカラー倍代入
Vector3& operator*=(Vector3& v, float scalar) {
	v = v * scalar;
	return v;
}
// Vector3スカラー除算代入
Vector3& operator/=(Vector3& v, float scalar) {
	v = v / scalar;
	return v;
}