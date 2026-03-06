#define NOMINMAX
#include "Collider.h"
#include"Function.h"
#include "Object3d/Object3dCommon.h"
YoshidaMath::Collider::Collider()
{

#ifdef USE_IMGUI

    primitive_ = std::make_unique<Primitive>();
#endif // _DEBUG

    collisionInfo_.collided = false;
    collisionInfo_.normal = { 0.0f,0.0f,0.0f };
    collisionInfo_.penetration = { 0.0f };
}

void YoshidaMath::Collider::SetCamera(Camera* camera)
{

#ifdef USE_IMGUI
    primitive_->SetCamera(camera);
#endif // _DEBUG
}

void YoshidaMath::Collider::SetRadius(float radius)
{
   type_ = ColliderType::kSphere;
    radius_ = radius;

#ifdef USE_IMGUI
    if (primitive_) {
        primitive_->Initialize(Primitive::Sphere);
        float scale = radius;
        primitive_->SetScale({ scale,scale,scale });
    }
#endif // DEBUG

}


void YoshidaMath::Collider::SetAABB(const AABB& aabb) {

    type_ = ColliderType::kAABB;
   AABB_ = aabb;

#ifdef USE_IMGUI
    if (primitive_) {
        primitive_->Initialize(Primitive::Box);
        Vector3 scale =
        {
        aabb.max.x - aabb.min.x,
        aabb.max.y - aabb.min.y,
        aabb.max.z - aabb.min.z
        };
        primitive_->SetScale(scale);
    }
#endif // DEBUG

};

void YoshidaMath::Collider::ColliderUpdate()
{

#ifdef USE_IMGUI
    if (primitive_) {
        primitive_->SetColor({ 1.0f,1.0f,0.0f,0.5f });
        if (type_ == kAABB) {
            primitive_->SetTranslate(GetWorldPosition() + YoshidaMath::GetAABBCenter(AABB_));
        } else {
            primitive_->SetTranslate(GetWorldPosition());
        }

        primitive_->Update();
    }
#endif // _DEBUG
}

void YoshidaMath::Collider::ColliderDraw()
{

#ifdef USE_IMGUI
    if (primitive_) {
        primitive_->UpdateCameraMatrices();
        primitive_->Draw();
    }
#endif // _DEBUG
}

void YoshidaMath::Collider::OnCollisionCollider()
{

#ifdef USE_IMGUI
    if (primitive_) {
        primitive_->SetColor({ 1.0f,0.0f,0.0f,0.5f });
    }
#endif // _DEBUG

}

bool YoshidaMath::RayIntersectsAABB(const Ray& ray, const AABB& box, float tMin, float tMax) {

    //tMin = 0.0f;
    //tMax = std::numeric_limits<float>::max();

    float minVal = 0.0f;
    float maxVal = 0.0f;
    float origin = 0.0f;
    float dir = 0.0f;

    for (int i = 0; i < 3; i++) {

        if (i == 0) {
            minVal = box.min.x;
            maxVal = box.max.x;
            origin = ray.origin.x;
            dir = ray.diff.x;
        }
        if (i == 1) {
            minVal = box.min.y;
            maxVal = box.max.y;
            origin = ray.origin.y;
            dir = ray.diff.y;
        }
        if (i == 2) {
            minVal = box.min.z;
            maxVal = box.max.z;
            origin = ray.origin.z;
            dir = ray.diff.z;
        }

        if (std::abs(dir) < 1e-6f) {
            // レイが軸に平行
            if (origin < minVal || origin > maxVal) {
                return false;
            }
        } else {
            float t1 = (minVal - origin) / dir;
            float t2 = (maxVal - origin) / dir;

            if (t1 > t2) std::swap(t1, t2);

            tMin = std::max(tMin, t1);
            tMax = std::min(tMax, t2);

            if (tMin > tMax) {
                return false;
            }
        }
    }

    return true;
}


YoshidaMath::CollisionInfo YoshidaMath::GetCollisionInfo(const AABB& a, const AABB& b) {

    CollisionInfo result;

    if (!IsCollision(a, b)) {
        result.collided = false;
        return result;
    }

    result.collided = true;
    //オーバーラップを調べる
    float overlapX = std::min(a.max.x - b.min.x, b.max.x - a.min.x);
    float overlapY = std::min(a.max.y - b.min.y, b.max.y - a.min.y);
    float overlapZ = std::min(a.max.z - b.min.z, b.max.z - a.min.z);

    Vector3 centerA = GetAABBCenter(a);
    Vector3 centerB = GetAABBCenter(b);

    //最小のオーバーラップ軸を分離する
    if (overlapX <= overlapY && overlapX <= overlapZ) {

        result.penetration = overlapX;
        result.normal = (centerA.x < centerB.x) ? Vector3(-1.0f, 0.0f, 0.0f) : Vector3(1.0f, 0.0f, 0.0f);

    } else if (overlapY <= overlapZ) {
        result.penetration = overlapY;
        result.normal = (centerA.y < centerB.y) ? Vector3(0.0f, -1.0f, 0.0f) : Vector3(0.0f, 1.0f, 0.0f);
    } else {
        result.penetration = overlapZ;
        result.normal = (centerA.z < centerB.z) ? Vector3(0.0f, 0.0f, -1.0f) : Vector3(0.0f, 0.0f, 1.0f);
    }

    return result;
}

void YoshidaMath::ResolveCollision(Vector3& pos, Vector3& velocity, const YoshidaMath::CollisionInfo& info) {

    if (!info.collided) return;

    pos += info.normal * info.penetration;

    float normalVelocity = Function::Dot(velocity, info.normal);

    if (normalVelocity < 0.0f) {
        velocity.x -= info.normal.x * normalVelocity;
        velocity.y -= info.normal.y * normalVelocity;
        velocity.z -= info.normal.z * normalVelocity;
    }
}
Sphere YoshidaMath::GetSphereWorldPos(YoshidaMath::Collider* sphere)
{
    return Sphere{
 .center = sphere->GetWorldPosition(),
 .radius = sphere->GetRadius()
    };
}

bool YoshidaMath::IsCollision(const Sphere& s1, const Sphere& s2)
{

    //2つの急の中心点間距離を求める 
    float distance = Function::Length({ s2.center - s1.center });

    if (distance <= s1.radius + s2.radius) {
        return true;
    }

    return false;
}

bool YoshidaMath::IsCollision(const AABB& a, const AABB& b)
{

    if ((a.min.x <= b.max.x && a.max.x >= b.min.x) &&//x軸
        (a.min.y <= b.max.y && a.max.y >= b.min.y) &&//y軸
        (a.min.z <= b.max.z && a.max.z >= b.min.z)) {
        return true;
    }

    return false;
}

AABB YoshidaMath::GetAABBWorldPos(YoshidaMath::Collider* aabb)
{
    AABB aabbWorld = aabb->GetAABB();
    Vector3 pos = aabb->GetWorldPosition();
    aabbWorld.min += aabbWorld.min+pos;
    aabbWorld.max += pos;
    return aabbWorld;

}

Vector3 YoshidaMath::GetAABBCenter(const AABB& aabb)
{
    return (aabb.min + aabb.max) * 0.5f;
}

