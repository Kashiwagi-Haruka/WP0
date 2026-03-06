#pragma once
#include"Object3d/Object3d.h"
#include<memory>
#include"RigidBody.h"
#include"Primitive/Primitive.h"
#include"GameObject/YoshidaMath/CollisionManager/Collider.h"

class WhiteBoard  : public YoshidaMath::Collider
{
public:
    WhiteBoard();
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const  override;

    virtual void Initialize();
    virtual void Update();
    virtual void Draw();
    void SetCamera(Camera* camera);
    void SetModel(const std::string& filePath);
    AABB GetAABB();
    Transform& GetTransform() { return collisionTransform_; }
    virtual void ResetCollisionAttribute();
protected:
    std::unique_ptr<Object3d>obj_ = nullptr;
#ifdef _DEBUG
    std::unique_ptr<Primitive>primitive_ = nullptr;
#endif
    AABB localAABB_ = {};
    Transform collisionTransform_ = {};
};