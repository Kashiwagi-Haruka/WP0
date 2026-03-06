#include "PlayerCamera.h"
#include"imgui.h"
#include"Function.h"
#include"GameObject/KeyBindConfig.h"
#include<algorithm>
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/YoshidaMath/CollisionManager/CollisionManager.h"
#include"WinApp.h"


PlayerCamera::PlayerCamera()
{
    //カメラの設定
    cameraTransform_ = {
    .scale{1.0f, 1.0f, 1.0f  },
    .rotate{0.0f, 0.0f, 0.0f  },
    .translate{0.0f, 5.0f, -10.0f}
    };

    //カメラを生成する
    camera_ = std::make_unique<Camera>();
    camera_->SetTransform(cameraTransform_);
    //Rayの設定
    ray_ = { .origin = {0.0f},.diff = {0.0f} };

    //raySpriteの設定   
    raySprite_ = std::make_unique<RaySprite>();


}

void PlayerCamera::Update()
{
    SetTransform();
    SetRay();
    camera_->Update();

    // レイの判定&インタラクト
    auto* playerCommand = PlayerCommand::GetInstance();
    
    if (playerCommand->Interact()) {
    }
  
    raySprite_->Update();
    
}

void PlayerCamera::Rotate()
{
    Vector2 deltaRotate = PlayerCommand::GetInstance()->Rotate(eyeRotateSpeed_);

    playerTransform_->rotate.y += deltaRotate.y;
    cameraTransform_.rotate.x += deltaRotate.x;
#ifdef USE_IMGUI
    if (ImGui::TreeNode("Eye")) {
        ImGui::DragFloat("eyeRotateSpeed", &eyeRotateSpeed_, 0.1f, 0.1f);
        ImGui::DragFloat("eyeRotateX", &cameraTransform_.rotate.x, 0.1f);

        ImGui::DragFloat3("origin", &ray_.origin.x, 0.3f);
        ImGui::DragFloat3("diff", &ray_.diff.x, 0.3f);


        ImGui::TreePop();
    }
#endif
}

void PlayerCamera::SetRay()
{
  
    ray_.origin = cameraTransform_.translate;
    ray_.diff = GetForward();

    //// 左下が０、右上が１とした時のマウスポジション
    //float ndcX = (cameraTransform_.translate.x / WinApp::kClientWidth) * 2.0f - 1.0f;
    //float ndcY = 1.0f - (cameraTransform_.translate.y / WinApp::kClientHeight) * 2.0f; // Yは上下反転

    //// クリップ空間でZ=0(near)とZ=1(far)の2点を作る
    //Vector3 nearPoint = { ndcX, ndcY, 0.0f };
    //Vector3 farPoint = { ndcX, ndcY, 1.0f };

    //// 逆射影行列
    //Matrix4x4 inverseViewProj = Function::Inverse(camera_->GetViewProjectionMatrix());

    //// ワールド空間に変換
    //Vector3 nearWorld = Function::TransformVM(nearPoint, inverseViewProj);
    //Vector3 farWorld = Function::TransformVM(farPoint, inverseViewProj);

    //// マウスレイの始点・方向
    //ray_.origin = nearWorld;
    //ray_.diff = Function::Normalize(farWorld- ray_.origin);
}

void PlayerCamera::DrawRaySprite()
{
    raySprite_->Draw();
}

void PlayerCamera::Initialize()
{
    raySprite_->Initialize();
}

bool PlayerCamera::OnCollisionRay(const AABB& localAABB,const Vector3& translate)
{
    return YoshidaMath::RayIntersectsAABB(GetRay(), YoshidaMath::GetAABBWorldPos(localAABB, translate), kTMin_, kTMax_);
}


void PlayerCamera::SetTransform()
{
    assert(playerTransform_);

    Rotate();

    //Playerからの視点
    cameraTransform_.scale = { 1.0f,1.0f,1.0f };
    float halfPi = Function::kPi * 0.5f;
    cameraTransform_.rotate.x =

        std::clamp(
            cameraTransform_.rotate.x,
            -halfPi,
            halfPi- halfPi * 0.25f);

    cameraTransform_.rotate.y = playerTransform_->rotate.y;
    cameraTransform_.rotate.z = 0.0f;

    cameraTransform_.translate = playerTransform_->translate;
    cameraTransform_.translate.y += 1.6f;
    cameraTransform_.translate += ray_.diff*1.25f;
    camera_->SetTransform(cameraTransform_);

}

Vector3 PlayerCamera::GetForward()
{
    return  YoshidaMath::GetForward(camera_->GetWorldMatrix());
}
