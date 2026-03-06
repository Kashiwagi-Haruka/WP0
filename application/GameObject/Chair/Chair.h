#pragma once
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include "GameObject/YoshidaMath/CollisionManager/Collider.h"
#include"Object3d/Object3d.h"
#include<memory>

class Camera;

class Chair :
    public YoshidaMath::Collider
{
public:
    Chair();
    /// @brief 衝突時コールバック関数
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const override;
    void Update();
    void Initialize();
    void Draw();
    void CheckCollision();
    void SetPlayerCamera(PlayerCamera* camera);
    void SetCamera(Camera* camera);
private:
    bool OnCollisionRay();
    PlayerCamera* playerCamera_ = nullptr;
    Transform transform_ = {};
    std::unique_ptr<Object3d>obj_ = nullptr;
};

