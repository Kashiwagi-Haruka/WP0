#include "Portal.h"
#include"Function.h"
#include"application/GameObject/YoshidaMath/YoshidaMath.h"
#include<cassert>
#include"GameObject/YoshidaMath/Easing.h"

bool Portal::isPlayerHit_ = false;
//音楽
SoundData Portal::warpSE_;

void Portal::LoadSE()
{
    warpSE_ = Audio::GetInstance()->SoundLoadFile("Resources/audio/SE/magic.mp3");
    Audio::GetInstance()->SetSoundVolume(&warpSE_, 1.0f);
}

void Portal::UnLoadSE()
{
    Audio::GetInstance()->SoundUnload(&warpSE_);
}

void Portal::OnCollision(Collider* collider)
{
    if (!isPlayerHit_) {
        if (collider->GetCollisionAttribute() == kCollisionPlayer) {
            if (warpCoolTimer_ == kWarpTime_) {
                warpCoolTimer_ = 0.0f;
                Audio::GetInstance()->SoundPlayWave(warpSE_, false);
                isPlayerHit_ = true;
            }
        }
    }
}

Vector3 Portal::GetWorldPosition() const
{
    return transform_.translate;
}

Portal::Portal()
{
    transform_ = { .scale = {1.5f,1.5f,1.5f},.rotate = {0.0f,0.0f,0.0f},.translate = {0.0f,0.75f,0.0f} };
    sphere_ = { .center = {transform_.translate},.radius = 0.5f };

    SetRadius(sphere_.radius);
    SetCollisionAttribute(kCollisionPortal);
    SetCollisionMask(kCollisionPlayer);

    ring_ = std::make_unique<Primitive>();
    portalCircle_ = std::make_unique<Primitive>();
    //ワープ座標
    warpPos_ = std::make_unique<WarpPos>();
}

Portal::~Portal()
{

}

void Portal::Initialize()
{
    warpCoolTimer_ = kWarpTime_;

    isPlayerHit_ = false;
    scaleTimer_ = 0.0f;
    transform_ = { .scale = {0.0f,0.0f,0.0f},.rotate = {0.0f,0.0f,0.0f},.translate = {0.0f,0.0f,0.0f} };

    portalCircle_->Initialize(Primitive::Circle, 48);
    /*   portalCircle_->SetColor({ 0.3f, 0.7f, 1.0f, 1.0f });*/
    portalCircle_->SetEnableLighting(false);

    ring_->Initialize(Primitive::Ring, "Resources/TD3_3102/2d/ring.png", 128);
    //ライティングしない
    ring_->SetEnableLighting(false);
    ring_->SetColor({ 1.0f,1.0f,1.0f,1.0f });
    /*   ring_->SetColor({ 0.3f, 0.7f, 1.0f, 1.0f });*/

    uvRotateZ_ = 0.0f;
    sphere_ = { .center = {transform_.translate},.radius = 0.5f };
    //ワープ座標
    warpPos_->Initialize();

    portalRenderTexture_ = std::make_unique<RenderTexture2D>();
    portalRenderTexture_->Initialize(WinApp::kClientWidth, WinApp::kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, { 0.05f, 0.05f, 0.1f, 1.0f });
    if (portalRenderTexture_->IsReady()) {
        portalCircle_->SetTextureIndex(portalRenderTexture_->GetSrvIndex());
        portalCircle_->SetSecondaryTextureIndex(portalRenderTexture_->GetSrvIndex());
    }

    portalCircle_->SetPortalProjectionEnabled(true);
}

void Portal::Update()
{
    isPlayerHit_ = false;

    warpCoolTimer_ += YoshidaMath::kDeltaTime;
    warpCoolTimer_ = std::clamp(warpCoolTimer_, 0.0f, kWarpTime_);

    uvRotateZ_ += YoshidaMath::kDeltaTime * 2.0f;
    ring_->SetUvTransform(Vector3(1, 1, 1), Vector3(0, 0, uvRotateZ_), Vector3(0, 0, 0), Vector2(0.5f, 0.5f));
    SetPortalWorldMatrix();
    ring_->Update();
    //ワープ地点
    warpPos_->Update();
}


void Portal::DrawPortals() {
    Object3dCommon::GetInstance()->DrawCommonPortal();
    portalCircle_->Draw();
}

void Portal::DrawRings() {
    //Object3dCommon::GetInstance()->DrawCommonNoCull();
    ring_->Draw();
}

void Portal::DrawWarpPos()
{
    warpPos_->Draw();
}

void Portal::UpdatePortalCamera(const Transform& destinationPortal, Camera* outCamera) {
    if (!outCamera) {
        return;
    }

    const Matrix4x4 destinationPortalWorld = Function::MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, destinationPortal.rotate, destinationPortal.translate);
    const Matrix4x4 halfTurn = Function::MakeRotateYMatrix(std::numbers::pi_v<float>);
    Matrix4x4 portalCameraWorld = Function::Multiply(halfTurn, destinationPortalWorld);

    Vector3 destinationForward = { destinationPortalWorld.m[2][0], destinationPortalWorld.m[2][1], destinationPortalWorld.m[2][2] };
    if (Function::LengthSquared(destinationForward) < 0.0001f) {
        destinationForward = { 0.0f, 0.0f, 1.0f };
    }
    destinationForward = Function::Normalize(destinationForward);
    portalCameraWorld.m[3][0] += destinationForward.x * 0.05f;
    portalCameraWorld.m[3][1] += destinationForward.y * 0.05f;
    portalCameraWorld.m[3][2] += destinationForward.z * 0.05f;

    const Matrix4x4 portalViewMatrix = Function::Inverse(portalCameraWorld);

    // ポータルテクスチャ専用カメラはメインカメラに追従させず、ポータルの位置・角度だけで固定する。
    outCamera->SetViewProjectionMatrix(portalViewMatrix, outCamera->GetProjectionMatrix());
}

void Portal::RenderPortalTextures(const std::function<void(Camera*)>& drawSceneWithoutPortals)
{
    auto* camera = GetCamera();
    if (!sceneCamera_ && !camera) {
        return;
    }

    if (portalRenderTexture_ && portalRenderTexture_->IsReady()) {
        UpdatePortalCamera(warpPos_->GetTransform(), camera);
        portalRenderTexture_->BeginRender();
        drawSceneWithoutPortals(camera);
        portalRenderTexture_->TransitionToShaderResource();
    }

    portalCircle_->SetPortalProjectionMatrices(camera->GetViewProjectionMatrix(), camera->GetViewProjectionMatrix(), camera->GetWorldMatrix(), camera->GetWorldMatrix());

}

void Portal::UpdateCameraMatrices() {
    portalCircle_->UpdateCameraMatrices();
    ring_->UpdateCameraMatrices();
}

void Portal::SetCamera(Camera* camera)
{
    sceneCamera_ = camera;
    portalCircle_->SetCamera(camera);
    ring_->SetCamera(camera);
    //ワープ地点
    warpPos_->SetCamera(camera);
}

void Portal::SetPortalWorldMatrix()
{
    if (sceneCamera_ == nullptr) {
        return;
    }

    if (parentTransform == nullptr) {
        return;
    }
    Vector3 forward = YoshidaMath::GetForward(sceneCamera_->GetWorldMatrix());

    transform_ = *parentTransform;


    scaleTimer_ += YoshidaMath::kDeltaTime;
    scaleTimer_ = std::clamp(scaleTimer_, 0.0f, 1.0f);

    transform_.scale = YoshidaMath::Easing::EaseInOutBack({ 0.0f,0.0f,0.0f }, parentTransform->scale, scaleTimer_);

    transform_.translate -= forward * 0.125f;

    Matrix4x4  worldMatrix = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

    portalCircle_->SetWorldMatrix(worldMatrix);
    transform_.translate -= forward * 0.125f;
    worldMatrix = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

    ring_->SetWorldMatrix(worldMatrix);

}

const Sphere& Portal::GetSphere()
{
    sphere_ = { .center = {transform_.translate},.radius = 0.5f };
    return sphere_;
}
