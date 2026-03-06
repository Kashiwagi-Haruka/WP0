#pragma once
#include"Camera.h"
#include<memory>
#include"Object3d/Object3d.h"
#include"Primitive/Primitive.h"
class WarpPos
{
public:
    WarpPos();
    void SetCamera(Camera* camera);
    Transform& GetTransform() { return transform_; }
    Vector3 GetWorldPos();
    void Initialize();
    void Update();
    void Draw();
    Camera* GetWarpPosCamera() { return camera_.get(); };
    void SetParent(Transform* transform) { parentTransform_ = transform; }
    Transform* GetParent() { return parentTransform_; }
private:
    std::unique_ptr<Camera> camera_ = nullptr;
    std::unique_ptr<Object3d>object3d_ = nullptr;
    Transform* parentTransform_;
    Transform transform_ = {};
    float sinTheta_ = 0.0f;

};

