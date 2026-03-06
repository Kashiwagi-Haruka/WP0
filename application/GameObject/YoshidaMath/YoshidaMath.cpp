#include "YoshidaMath.h"
#include"Function.h"
#include<cmath>
#include<algorithm>
#include"Camera.h"

// 投影行列をポータル平面でクリップするように改造する関数
Matrix4x4 YoshidaMath::CalculateObliqueMatrix(const Matrix4x4& projection, const Vector4& clipPlane) {
    Matrix4x4 obliqueProj = projection;

    // 平面の方程式をプロジェクション空間の対角成分に反映させる計算
    // clipPlane は Camera Space での平面方程式 (ax + by + cz + d = 0)
    Vector4 q;
    q.x = (sgn(clipPlane.x) + projection.m[2][0]) / projection.m[0][0];
    q.y = (sgn(clipPlane.y) + projection.m[2][1]) / projection.m[1][1];
    q.z = -1.0f;
    q.w = (1.0f + projection.m[2][2]) / projection.m[3][2];

    float doubleDot = 2.0f / Dot(clipPlane, q);
    Vector4 c = {
        clipPlane.x * doubleDot,
         clipPlane.y * doubleDot,
        clipPlane.z * doubleDot,
         clipPlane.w * doubleDot,
    };

    obliqueProj.m[0][2] = c.x;
    obliqueProj.m[1][2] = c.y;
    obliqueProj.m[2][2] = c.z + 1.0f;
    obliqueProj.m[3][2] = c.w;

    return obliqueProj;
}

float YoshidaMath::Dot(const Vector4& v1, const Vector4& v2)
{
    // 内積を計算する
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;

}

float YoshidaMath::sgn(float num)
{
    return  (num < 0.0f) ? -1.0f : 1.0f;
}

float YoshidaMath::Dot(const Vector2& v1, const Vector2& v2)
{
    return { v1.x * v2.x + v1.y * v2.y };
}

float YoshidaMath::Length(const Vector2& v)
{
    return { sqrtf(YoshidaMath::Dot(v,v)) };
}

Vector2 YoshidaMath::Normalize(const Vector2& v)
{
    float length = Length(v);
    if (length != 0.0f) {
        return { v.x / length,v.y / length };
    } else {
        return { 0.0f, 0.0f };
    }
}

Vector3 YoshidaMath::GetForward(const Matrix4x4& m) {

    return  Function::Normalize({ m.m[2][0],m.m[2][1], m.m[2][2] });
}


Vector3 YoshidaMath::GetWorldPosByMat(const Matrix4x4& mat)
{
    return { mat.m[2][0], mat.m[2][1], mat.m[2][2] };
}

AABB YoshidaMath::GetAABBWorldPos(const AABB& localAABB, const Vector3& worldPos)
{
    return  { .min = localAABB.min + worldPos,.max = localAABB.max + worldPos };
}

Matrix4x4 YoshidaMath::MakeRotateMatrix(const Vector3& rotate)
{
    Matrix4x4 rotateX = Function::MakeRotateXMatrix(rotate.x);
    Matrix4x4 rotateY = Function::MakeRotateYMatrix(rotate.y);
    Matrix4x4 rotateZ = Function::MakeRotateZMatrix(rotate.z);

    return  Function::Multiply(Function::Multiply(rotateX, rotateY), rotateZ);
}

Matrix4x4 YoshidaMath::GetBillBordMatrix(Camera* camera)
{
    //ビルボードで作成する
    Matrix4x4 billboardMatrix = camera->GetWorldMatrix();
    billboardMatrix.m[3][0] = 0.0f;
    billboardMatrix.m[3][1] = 0.0f;
    billboardMatrix.m[3][2] = 0.0f;
    return billboardMatrix;
}

Matrix4x4 YoshidaMath::GetBillBordMatrix(const Matrix4x4& mat)
{
    //ビルボードで作成する
    Matrix4x4 billboardMatrix = mat;
    billboardMatrix.m[3][0] = 0.0f;
    billboardMatrix.m[3][1] = 0.0f;
    billboardMatrix.m[3][2] = 0.0f;
    return billboardMatrix;
}

Vector3 YoshidaMath::GetAABBScale(const AABB& aabb)
{
    return  {
        aabb.max.x - aabb.min.x,
        aabb.max.y - aabb.min.y,
        aabb.max.z - aabb.min.z
    };
}

void YoshidaMath::LookTarget(const Vector3& target, Transform& transform)
{
    Vector3 direction = target - transform.translate;
    if (Function::Length(direction) > 0.0f) {
        transform.rotate.y = std::atan2(direction.x, direction.z); // Y軸回転（ラジアン）
    }
}

Vector3 YoshidaMath::GetToTargetVec(const Vector3& target, const  Vector3& pos)
{
    Vector3 velocity = target - pos;
    velocity = Function::Normalize(velocity);
    return velocity;
}
