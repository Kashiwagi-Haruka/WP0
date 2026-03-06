#pragma once
#include "WhiteBoard.h"
#include "Animation/Animation.h"
#include "Animation/Skeleton.h"
#include "Animation/SkinCluster.h"

class WalkWhiteBoard :
    public WhiteBoard
{
private:

    //アニメーションクリップ
    static std::vector<Animation::AnimationData> animationClips_;
    //骨
    std::unique_ptr<Skeleton> skeleton_{};
    //スキン
    SkinCluster skinCluster_{};
    //現在のアニメーションインデックス
    size_t currentAnimationIndex_ = 0; 
    //アニメーション時間
    float animationTime_ = 0.0f;
public:
    void OnCollision(Collider* collider)override;
    void SetTargetPosPtr(Vector3* pos) { targetPos_ = pos; };
    static void LoadAnimation(const std::string& directionalPath, const std::string& filePath);
    void Initialize()override;
    void Update()override;
    void Draw()override;
    void SetAnimationIndex(const size_t& index) { currentAnimationIndex_ = index; }
    void ResetCollisionAttribute()override;
private:
    bool isMove_ = false;
  
    Vector3 velocity_ = { 0.0f };
    Transform transform_ = {};
    Vector3* targetPos_ = nullptr;
    void Animation();
};

