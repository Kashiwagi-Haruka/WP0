#pragma once
#include"Primitive/Primitive.h"
#include<memory>
#include"Transform.h"
#include"RigidBody.h"
#include"Audio.h"
#include"GameObject/YoshidaMath/CollisionManager/Collider.h"
#include"GameObject/WarpPos/WarpPos.h"
#include "RenderTexture2D.h"
#include "Object3d/Object3dCommon.h"
#include <functional>

class Camera;
class Portal : public YoshidaMath::Collider
{
public:
    //音の設定
    static void LoadSE();
    static void UnLoadSE();
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const  override;

    Portal();
    ~Portal();
    void Initialize();
    void Update();
    void SetCamera(Camera* camera);

    Transform& GetTransform() { return transform_; };
    const Sphere& GetSphere();

    //PortalのSRTをセットする
    void SetParentTransform(Transform* transform) { parentTransform = transform; };

    void SetPortalWorldMatrix();

    Camera* GetCamera() { return warpPos_->GetWarpPosCamera(); };
    //ワープ先の座標をセットする
    Transform* GetWarpPosParent() { return warpPos_->GetParent(); }
    void SetWarpPosParent(Transform* transform) { warpPos_->SetParent(transform); };
    //ワープ先を取得する
    WarpPos* GetWarpPos() { return warpPos_.get(); }

    void RenderPortalTextures(const std::function<void(Camera*)>& drawSceneWithoutPortals);
    void UpdateCameraMatrices();
    void DrawPortals();
    void DrawRings();
    void DrawWarpPos();
    bool GetIsPlayerHit() { return isPlayerHit_; };
private:
    const float kWarpTime_ = 2.0f;
    float warpCoolTimer_ = kWarpTime_;

    static bool isPlayerHit_;
    float scaleTimer_ = 0.0f;
    void UpdatePortalCamera(const Transform& destinationPortal, Camera* outCamera);
    Camera* sceneCamera_ = nullptr;
    //音楽
    static SoundData warpSE_;
    std::unique_ptr<Primitive>ring_ = nullptr;
    std::unique_ptr<Primitive>portalCircle_ = nullptr;
    Transform transform_ = {};
    float uvRotateZ_ = 0.0f;
    Sphere sphere_ = { 0.0f };
    //ワープ座標
    std::unique_ptr<WarpPos> warpPos_ = nullptr;
    std::unique_ptr<RenderTexture2D> portalRenderTexture_ = nullptr;
    Transform* parentTransform = nullptr;
};

