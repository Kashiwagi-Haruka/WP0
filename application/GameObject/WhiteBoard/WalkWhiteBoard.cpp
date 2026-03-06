#include "WalkWhiteBoard.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"Object3d/Object3dCommon.h"
#include "Model/ModelManager.h"
#include"DirectXCommon.h"
#include<imgui.h>

//アニメーションクリップ
std::vector<Animation::AnimationData>WalkWhiteBoard:: animationClips_;

void WalkWhiteBoard::OnCollision(Collider* collider)
{
    
    if (collider->GetCollisionAttribute() == kCollisionPlayer) {
        isMove_ = true;
    }
}

void WalkWhiteBoard::LoadAnimation(const std::string& directionalPath, const std::string& filePath)
{
    //アニメーションクリップ
    animationClips_ = Animation::LoadAnimationClips(directionalPath, filePath);
}

void WalkWhiteBoard::Initialize()
{
    isMove_ = false;
    obj_->Initialize();
    transform_ = obj_->GetTransform();
    velocity_ = { 0.0f };
    localAABB_ = { .min = { -0.5f,-0.5f,-0.5f},.max = {0.5f,0.5f,0.5f} };
    /* SetRadius(1.0f);*/
    SetAABB(AABB{ .min = {-1.0f,0.0f,-1.0f}, .max = {1.0f,1.5f,1.0f} });
    SetCollisionAttribute(kCollisionEnemy);
    SetCollisionMask(kCollisionPlayer);

#ifdef _DEBUG
    primitive_->Initialize(Primitive::Box);
    primitive_->SetColor({ 1.0f,1.0f,1.0f,0.1f });
#endif
    localAABB_ = { .min = { -0.5f,-0.5f,-0.5f},.max = {0.5f,0.5f,0.5f} };

    if (!animationClips_.empty()) {
        SetAnimationIndex(1);
        obj_->SetAnimation(&animationClips_[currentAnimationIndex_], true);
    }

    if (Model* walkModel = ModelManager::GetInstance()->FindModel("whiteBoard")) {
        skeleton_ = std::make_unique<Skeleton>(Skeleton().Create(walkModel->GetModelData().rootnode));
        skinCluster_ = CreateSkinCluster(*skeleton_, *walkModel);
        if (!skinCluster_.mappedPalette.empty()) {
            obj_->SetSkinCluster(&skinCluster_);
        }
    }

}

void WalkWhiteBoard::Update()
{
    if (isMove_) {
        if (targetPos_) {
            YoshidaMath::LookTarget(*targetPos_, transform_);
            velocity_ = YoshidaMath::GetToTargetVec(*targetPos_, transform_.translate);
            velocity_ *= YoshidaMath::kDeltaTime;
            transform_.translate.x += velocity_.x;
            /*       transform_.translate.y += velocity;*/
            transform_.translate.z += velocity_.z;
        }
        obj_->SetTransform(transform_);

        Animation();
    }

    obj_->Update();

    collisionTransform_ = transform_;

    collisionTransform_.scale = YoshidaMath::GetAABBScale(localAABB_);
    collisionTransform_.rotate = { 0.0f };
    //objectからの相対距離
    collisionTransform_.translate.y += 1.375f;

#ifdef _DEBUG
    primitive_->SetTransform(collisionTransform_);
    primitive_->Update();


#endif
#ifdef USE_IMGUI
    ImGui::Begin("WalkWhiteBoard");
    ImGui::Checkbox("isMove", &isMove_);
    ImGui::End();
#endif


}

void WalkWhiteBoard::Draw()
{
    obj_->Draw();
#ifdef _DEBUG
    primitive_->Draw();
#endif
}

void WalkWhiteBoard::ResetCollisionAttribute()
{
    SetCollisionAttribute(kCollisionEnemy);
}

void WalkWhiteBoard::Animation()
{
    float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();

    if (skeleton_ && !animationClips_.empty()) {
        const Animation::AnimationData& currentAnimation = animationClips_[currentAnimationIndex_];
        animationTime_ = Animation::AdvanceTime(currentAnimation, animationTime_, deltaTime, true);
        skeleton_->ApplyAnimation(currentAnimation, animationTime_);
        skeleton_->Update();
        if (!skinCluster_.mappedPalette.empty()) {
            UpdateSkinCluster(skinCluster_, *skeleton_);
        }
        Matrix4x4 humanWorld = obj_->GetWorldMatrix();
        skeleton_->SetObjectMatrix(humanWorld);
    }
}
