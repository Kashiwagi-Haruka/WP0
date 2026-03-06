#pragma once
#include"Primitive/Primitive.h"
#include<memory>
#include"Transform.h"
#include"GameObject/YoshidaMath/CollisionManager/Collider.h"
class Camera;
class TestField : public YoshidaMath::Collider
{
public:
    TestField();
    void Initialize();
    void Update();
    void Draw();
    void SetCamera(Camera* camera);
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const  override;
private:
    std::unique_ptr<Primitive>plane_ = nullptr;
    Transform transform_ = {};
};

