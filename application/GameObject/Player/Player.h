#pragma once
#include<memory>
#include"Object3d/Object3d.h"
#include "Animation/Animation.h"
#include "Animation/Skeleton.h"
#include "Animation/SkinCluster.h"
#include"RigidBody.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/YoshidaMath/CollisionManager/Collider.h"

class Camera;

class Player : public YoshidaMath::Collider
{
private:
#pragma region//体やメッシュの情報
    //体のObj
    std::unique_ptr<Object3d> bodyObj_ = nullptr;
    //アニメーションクリップ
    std::vector<Animation::AnimationData> animationClips_{};
    //現在のアニメーションインデックス
    size_t currentAnimationIndex_ = 0;
    //骨
    std::unique_ptr<Skeleton> skeleton_{};
    //スキン
    SkinCluster skinCluster_{};
    //アニメーション時間
    float animationTime_ = 0.0f;
#pragma endregion
    //体の座標
    Transform transform_{};
    //速度
    Vector3 velocity_ = { 0.0f };
    Vector3 forward_ = { 0.0f };
    //移動の速さ
    float moveSpeed_ = { 0.0f };

    AABB localAABB_ = { 0.0f };
    //衝突情報
    YoshidaMath::CollisionInfo collisionInfo_;

public:
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const  override;
    //障害物との衝突処理
    void OnCollisionObstacle();
    Transform& GetTransform() { return transform_; };
    //前方のベクトルを取得する
    const Vector3& GetForward() const { return forward_; };
    void SetTranslate(const Vector3& translate) { transform_.translate = translate; };
    void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
    //コンストラクタ
    Player();
    //カメラのセッター
    void SetCamera(Camera* camera);
    //初期化
    void Initialize();
    //更新処理
    void Update();

    //描画処理
    void Draw();
    //デバック
    void Debug();
    //移動
    void Move();
    //重力処理
    void Gravity();

    //アニメーション
    void Animation();
    //ワールド行列の取得
    const Matrix4x4& GetWorldMatrix() const { return bodyObj_->GetWorldMatrix(); }

};

