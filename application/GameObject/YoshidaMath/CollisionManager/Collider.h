#pragma once
#include "Vector3.h"
#include <cstdint>
#include<memory>
#include"RigidBody.h"
#include"Primitive/Primitive.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/YoshidaMath/CollisionManager/CollisionConfig.h"

class Camera;
class Collider;
namespace YoshidaMath {

    enum ColliderType {
        kSphere,
        kAABB
    };

    struct CollisionInfo {
        bool collided = false;
        Vector3 normal = { 0.0f,0.0f,0.0f }; //法線
        float penetration = 0;//めり込み量
    };
    //rayとAABBの衝突を取得する
    bool RayIntersectsAABB(const Ray& ray, const AABB& box, float tMin, float tMax);
    //衝突情報を取得する
    CollisionInfo GetCollisionInfo(const AABB& a, const AABB& b);
    //衝突情報をもとに座標のめり込みを考える
    void ResolveCollision(Vector3& pos, Vector3& velocity, const YoshidaMath::CollisionInfo& info);
    //AABBの中心を取得する
    Vector3 GetAABBCenter(const AABB& aabb);

}

/// @brief 衝突判定オブジェクト
namespace YoshidaMath {
    class Collider {
    private:
        YoshidaMath::CollisionInfo collisionInfo_;
        ColliderType type_ = ColliderType::kSphere;
        uint32_t collisionAttribute_ = 0xffffffff;	// 衝突属性
        uint32_t collisionMask_ = 0xffffffff;		// 衝突マスク
        float radius_ = 1.0f;	// 衝突半径
        AABB AABB_;

#ifdef USE_IMGUI
        //デバック用
        std::unique_ptr<Primitive>primitive_;
#endif // DEBUG

    public:

        Collider();
        void SetCamera(Camera* camera);
        /// @brief 衝突時コールバック関数
        virtual void OnCollision(Collider* collider) = 0;

        /// @brief ワールド座標を取得する
        /// @return ワールド座標
        virtual Vector3 GetWorldPosition() const = 0;
  
        /// @brief 衝突半径を設定する
        /// @param radius 衝突半径
        void SetRadius(float radius);
        void SetAABB(const AABB& aabb);
        /// @brief 衝突半径を取得する
        /// @return 衝突半径
        float GetRadius() const { return radius_; }
        const AABB& GetAABB() const { return AABB_; }

        ColliderType GetType() const { return type_; }

        /// @brief 衝突属性を取得する
        /// @return 衝突属性
        uint32_t GetCollisionAttribute() const { return collisionAttribute_; }

        /// @brief 衝突属性を設定する
        /// @param attribute 衝突属性
        void SetCollisionAttribute(uint32_t attribute) { collisionAttribute_ = attribute; }

        /// @brief 衝突マスクを取得する
        /// @return 衝突マスク
        uint32_t GetCollisionMask() const { return collisionMask_; }

        /// @brief 衝突マスクを設定する
        /// @param mask 衝突マスク
        void SetCollisionMask(uint32_t mask) { collisionMask_ = mask; }
        void ColliderUpdate();
        void ColliderDraw();
        void OnCollisionCollider();
        void SetCollisionInfo(const CollisionInfo& info) { collisionInfo_ = info; };
        CollisionInfo& GetCollisionInfo() {
            return collisionInfo_;
        }
    };

    //AABBのワールド座標を取得する
    AABB GetAABBWorldPos(YoshidaMath::Collider* aabb);
    //Sphereのワールド座標を取得する
    Sphere GetSphereWorldPos(YoshidaMath::Collider* sphere);
    bool IsCollision(const Sphere& s1, const Sphere& s2);
    bool IsCollision(const AABB& a, const AABB& b);
}
