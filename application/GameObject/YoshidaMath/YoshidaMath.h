#pragma once
#include"Vector2.h"
#include"Vector3.h"
#include"Vector4.h"
#include"RigidBody.h"
#include"Matrix4x4.h"
#include"Transform.h"

class Camera;
class Collider;
namespace YoshidaMath {
    const constexpr float kDeltaTime = 1.0f / 60.0f;
    const constexpr float kGravity = 0.98f;

    float Dot(const Vector2& v1, const Vector2& v2);
    float Length(const Vector2& v);
    Vector2 Normalize(const Vector2& v);
    //行列から前方のベクトルを取得する
    Vector3 GetForward(const Matrix4x4& m);
    //ワールド行列からワールド座標を取得する
    Vector3 GetWorldPosByMat(const Matrix4x4& mat);
    AABB GetAABBWorldPos(const AABB& localAABB, const Vector3& worldPos);
    //オイラー角から回転行列を取得する
    Matrix4x4 MakeRotateMatrix(const Vector3& rotate);
    Matrix4x4 GetBillBordMatrix(Camera* camera);
    Matrix4x4 GetBillBordMatrix(const Matrix4x4& mat);
    Vector3 GetAABBScale(const AABB& aabb);
    void LookTarget(const Vector3& target, Transform& transform);
    Vector3 GetToTargetVec(const Vector3& target,const Vector3& pos);
    Matrix4x4 CalculateObliqueMatrix(const Matrix4x4& projection, const Vector4& clipPlane);
    // 内積を計算する
    float Dot(const Vector4& v1, const Vector4& v2);
    float sgn(float num);

}
