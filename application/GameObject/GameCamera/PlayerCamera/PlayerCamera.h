#pragma once
#include <memory>
#include "Camera.h"
#include"Transform.h"
#include"RigidBody.h"
#include"GameObject/UI/RaySprite.h"

class PlayerCamera
{
public:
    PlayerCamera();
    void Update();
    Camera* GetCamera() { return camera_.get(); };
    Transform& GetTransform() { return cameraTransform_; }
    void SetPlayerTransformPtr(Transform* transformPtr) {
        playerTransform_ = transformPtr;
    }
    //回転
    void Rotate();
    //Rayをセットする
    void SetRay();
    Ray& GetRay() { return ray_; };
    void DrawRaySprite();
    void Initialize();
    //RayとAABBの当たり判定を共通化しました。
    bool OnCollisionRay(const AABB& localAABB, const Vector3& translate);
private:
    const float kTMin_ = 0.0f;
    const float kTMax_ = 5.0f;

    void SetTransform();
    Vector3 GetForward();
    Transform cameraTransform_ = {};
    //カメラからのRay
    Ray ray_;
    //カメラの設定
    std::unique_ptr<Camera> camera_ = nullptr;
    Transform* playerTransform_ = nullptr;
    //カメラの感度
    float eyeRotateSpeed_ = 0.3f;
    //raySprite
    std::unique_ptr<RaySprite> raySprite_ = nullptr;

};

