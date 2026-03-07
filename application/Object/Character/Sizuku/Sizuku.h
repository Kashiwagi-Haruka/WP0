#pragma once
#include "Object/Playable/PlayableParameter.h"
#include <string>
#include "Function.h"
#include <memory>
#include "Camera.h"
#include "Animation/AnimationManager.h"
#include "Animation/SkinCluster.h"
#include "Object3d/Object3d.h"
class Sizuku {

	bool isHave_;

	PlayableParameter parameter_;


	Matrix4x4 playerWorld;
	std::unique_ptr<Object3d> Sizuku_;
	Transform transform_;
	Camera* camera_;
	std::unique_ptr<Skeleton> sizukuSkeleton_{};
	SkinCluster sizukuSkinCluster_{};
	Animation::AnimationData blendedPoseAnimation_{};
	const std::string animationGroupName_ = "sizuku";
	const float kAnimationBlendDuration_ = 0.3f;
	bool animationFinished_ = false;
	std::string desiredAnimationName = "Idle";
	public:
	void Initialize();
	void SetAnimation(std::string Name);
	void Update();
	void Draw();
	void SetCamera(Camera* camera);
	void SetTransform(Transform transform) { transform_ = transform; }
	std::optional<Matrix4x4> GetJointWorldMatrix(const std::string& jointName) const;
	bool IsAnimationFinished() const { return animationFinished_; }
};
