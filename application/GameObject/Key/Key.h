#pragma once
#include <Transform.h>
#include <string>
#include <Camera.h>
#include <RigidBody.h>
#include <memory>
#include <Object3d/Object3d.h>
#include <GameObject/TimeCard/TimeCardWatch.h>
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include <GameObject/YoshidaMath/CollisionManager/Collider.h>

class Key : public YoshidaMath::Collider
{
public:
    Key();
    void Initialize();
    void Update();
    void Draw();
    void SetPlayerCamera(PlayerCamera* camera);
    void SetCamera(Camera* camera);
    void SetModel(const std::string& filePath);
    Transform& GetTransform() { return collisionTransform_; }
    void CheckCollision();
    bool OnCollisionRay();
    /// @brief 衝突時コールバック関数
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const override;
private:
    std::unique_ptr<Object3d>obj_ = nullptr;
#ifdef _DEBUG
    std::unique_ptr<Primitive>primitive_ = nullptr;
#endif
    AABB localAABB_ = {};
    Transform collisionTransform_ = {};
    PlayerCamera* playerCamera_ = nullptr;

	Transform worldTransform_ = {};

	bool isGrabbed_ = false;
};

