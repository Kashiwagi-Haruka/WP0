#include "Chair.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"

Chair::Chair()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/chair", "chair");
    obj_->SetModel("chair");
    SetAABB({ .min = {-0.5f,0.0f,-0.5f},.max = {0.25f,0.5f,0.25f} });
    SetCollisionAttribute(kCollisionChair);
    SetCollisionMask(kCollisionPlayer);
}

void Chair::OnCollision(Collider* collider)
{
    if (collider->GetCollisionAttribute() == kCollisionPlayer) {

    }
}

Vector3 Chair::GetWorldPosition() const
{
    return obj_->GetTranslate();
}

void Chair::Update()
{
    obj_->SetTransform(transform_);
    obj_->Update();
}

void Chair::Initialize()
{
    obj_->Initialize();
    transform_.translate = { 1.0f,0.1f,1.0f };
    transform_.rotate = { 0.0f,0.0f,0.0f };
    transform_.scale = { 1.0f,1.0f,1.0f };
}

void Chair::Draw()
{
    obj_->UpdateCameraMatrices();
    obj_->Draw();
}

void Chair::CheckCollision()
{
    //椅子とrayの当たり判定
    if (OnCollisionRay()) {
        if (PlayerCommand::GetInstance()->Interact()) {
            // カーソルに追従させて持ち上げる処理
            Vector3 origin = playerCamera_->GetTransform().translate;
            origin.y -= 1.0f;
            transform_.translate = origin + (Function::Normalize(playerCamera_->GetRay().diff));
            transform_.translate.y = (std::max)(transform_.translate.y, 0.0f);
        }
    }
}

bool Chair::OnCollisionRay()
{
    return playerCamera_->OnCollisionRay(GetAABB(), transform_.translate);
}

void Chair::SetPlayerCamera(PlayerCamera* camera)
{
    playerCamera_ = camera;
}

void Chair::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
}
