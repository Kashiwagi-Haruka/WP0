#pragma once
#include "Matrix4x4.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include <cstdint>
#include <numbers>

namespace Function {

// 円周率定数
inline constexpr float kPi = std::numbers::pi_v<float>;

// 内積（2ベクトルの向きの近さ）を返す
float Dot(const Vector3& v1, const Vector3& v2);
// ベクトル長を返す
float Length(const Vector3& v);
// 長さの二乗
float LengthSquared(const Vector3& v);
// direction（向きたい方向）から回転角を計算する
// forwardAxis：モデルの前方向（Cube は X 軸→ {1,0,0}）
Vector3 DirectionToRotation(const Vector3& direction, const Vector3& forwardAxis);
// ベクトルを正規化する
Vector3 Normalize(const Vector3& v);

// 各軸の回転行列を生成する
Matrix4x4 MakeRotateXMatrix(float radian);
Matrix4x4 MakeRotateYMatrix(float radian);
Matrix4x4 MakeRotateZMatrix(float radian);
// クォータニオンから回転行列を生成する
Matrix4x4 MakeRotateMatrix(const Vector4& rotate);
// 4x4行列の積を計算する
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);
// 外積を計算する
Vector3 Cross(const Vector3& v1, const Vector3& v2);
// 2点間の差分ベクトルを返す
Vector3 Distance(const Vector3& pos1, const Vector3& pos2);

// ベクトルを行列変換する
Vector3 TransformVM(const Vector3& vector, const Matrix4x4& matrix4x4);
// 逆行列を計算する
Matrix4x4 Inverse(const Matrix4x4& m);
// 平行移動行列を生成する
Matrix4x4 MakeTranslateMatrix(Vector3 translate);
// 平行移動行列を生成する（スカラー指定）
Matrix4x4 MakeTranslateMatrix(float x, float y, float z);
// 拡大縮小行列を生成する
Matrix4x4 MakeScaleMatrix(Vector3 scale);
// SRT（オイラー回転）行列を生成する
Matrix4x4 MakeAffineMatrix(Vector3 scale, Vector3 rotate, Vector3 translate);
// SRT（オイラー回転）行列を生成する（アンカー指定）
Matrix4x4 MakeAffineMatrix(Vector3 scale, Vector3 rotate, Vector3 translate, Vector2 anchor);
// SRT（クォータニオン回転）行列を生成する
Matrix4x4 MakeAffineMatrix(Vector3 scale, Vector4 rotate, Vector3 translate);

// 透視投影行列を生成する
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

// 正射影行列を生成する
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

// ビューポート行列を生成する
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);
// 転置行列を返す
Matrix4x4 Transpose(const Matrix4x4& m);
// 線形補間（Vector3）
Vector3 Lerp(const Vector3& start, const Vector3& end, float ratio);
// 線形補間（float）
float Lerp(float start, float end, float ratio);
// 単位行列を生成する
Matrix4x4 MakeIdentity4x4();

// クォータニオンを正規化する
Vector4 NormalizeQuaternion(const Vector4& q);
// クォータニオン積を計算する（q1 * q2）
Vector4 MultiplyQuaternion(const Vector4& q1, const Vector4& q2);
// クォータニオンの共役を返す
Vector4 ConjugateQuaternion(const Vector4& q);
// 軸角からクォータニオンを生成する
Vector4 MakeQuaternionFromAxisAngle(const Vector3& axis, float radian);
// クォータニオンでベクトルを回転する
Vector3 RotateVectorByQuaternion(const Vector3& v, const Vector4& q);

} // namespace Function
// Vector3同士の加算
Vector3 operator+(const Vector3& v1, const Vector3& v2);
// Vector3同士の減算
Vector3 operator-(const Vector3& v1, const Vector3& v2);
// 単項マイナス
Vector3 operator-(const Vector3& v);
// スカラー倍
Vector3 operator*(const Vector3& v, float scalar);
// スカラー倍（左項）
Vector3 operator*(float scalar, const Vector3& v);
// スカラー除算
Vector3 operator/(const Vector3& v, float scalar);
// 加算代入
Vector3& operator+=(Vector3& v1, const Vector3& v2);
// 減算代入
Vector3& operator-=(Vector3& v1, const Vector3& v2);
// スカラー倍代入
Vector3& operator*=(Vector3& v, float scalar);
// スカラー除算代入
Vector3& operator/=(Vector3& v, float scalar);

