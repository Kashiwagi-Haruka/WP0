#include "Sizuku.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include "GameBase.h"
void Sizuku::Initialize(){
	Sizuku_ = std::make_unique<Object3d>();

	Sizuku_->SetModel("sizuku");

	Sizuku_->Initialize();

	AnimationManager::GetInstance()->LoadAnimationGroup(animationGroupName_, "Resources/3d", "sizuku");
	AnimationManager::GetInstance()->ResetPlayback(animationGroupName_, "Idle", true);
	if (const Animation::AnimationData* idleAnimation = AnimationManager::GetInstance()->FindAnimation(animationGroupName_, "Idle")) {
		Sizuku_->SetAnimation(idleAnimation, true);
	}

	if (Model* sizukuModel = ModelManager::GetInstance()->FindModel("sizuku")) {
		sizukuSkeleton_ = std::make_unique<Skeleton>(Skeleton().Create(sizukuModel->GetModelData().rootnode));
		sizukuSkinCluster_ = CreateSkinCluster(*sizukuSkeleton_, *sizukuModel);
		if (!sizukuSkinCluster_.mappedPalette.empty()) {
			Sizuku_->SetSkinCluster(&sizukuSkinCluster_);
		}
	}
	Sizuku_->SetShininess(20.0f);
}
void Sizuku::SetCamera(Camera* camera) { camera_ = camera; }
void Sizuku::SetAnimation(std::string Name){ 
desiredAnimationName = Name; }
void Sizuku::Update() {
	
	bool loopAnimation = true;
	
	if (desiredAnimationName == "Idle"){
		loopAnimation = true;
	} else if (desiredAnimationName == "Walk"){
		loopAnimation = true;
	} else if (desiredAnimationName == "Attack1"){
		loopAnimation = false;
	} else if (desiredAnimationName == "Attack2"){
		loopAnimation = false;
	} else if (desiredAnimationName == "Attack3"){
		loopAnimation = false;
	} else if (desiredAnimationName == "Attack4"){
		loopAnimation = false;
	} else if (desiredAnimationName == "FallAttack") {
		loopAnimation = false;
	} else if (desiredAnimationName == "SkillAttack"){
		loopAnimation = false;
	} else if (desiredAnimationName == "damage"){
		loopAnimation = true;
	} else if (desiredAnimationName == "InitIdle") {
		loopAnimation = true;
	}

	
	const float deltaTime = GameBase::GetInstance()->GetDeltaTime();
	AnimationManager::PlaybackResult playbackResult{};
	if (AnimationManager::GetInstance()->UpdatePlayback(animationGroupName_, desiredAnimationName, loopAnimation, deltaTime, kAnimationBlendDuration_, blendedPoseAnimation_, playbackResult)) {
		animationFinished_ = playbackResult.animationFinished;
		if (playbackResult.changedAnimation && playbackResult.currentAnimation) {
			Sizuku_->SetAnimation(playbackResult.currentAnimation, loopAnimation);
		}

		if (sizukuSkeleton_ && playbackResult.animationToApply) {
			sizukuSkeleton_->ApplyAnimation(*playbackResult.animationToApply, playbackResult.animationTime);
			sizukuSkeleton_->Update();
			if (!sizukuSkinCluster_.mappedPalette.empty()) {
				UpdateSkinCluster(sizukuSkinCluster_, *sizukuSkeleton_);
			}
		}
	}
	playerWorld = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	Sizuku_->SetWorldMatrix(playerWorld);
	Sizuku_->SetCamera(camera_);

	Sizuku_->Update();
}
void Sizuku::Draw(){
	Sizuku_->Draw();

#ifdef _DEBUG
	if (sizukuSkeleton_) {
		sizukuSkeleton_->SetObjectMatrix(playerWorld);
		Object3dCommon::GetInstance()->DrawCommonWireframeNoDepth();
		/*sizukuSkeleton_->DrawBones(camera_, {0.2f, 0.9f, 1.0f, 1.0f}, {0.1f, 0.5f, 0.9f, 1.0f});*/
	}
#endif
}
std::optional<Matrix4x4> Sizuku::GetJointWorldMatrix(const std::string& jointName) const {
	if (!sizukuSkeleton_) {
		return std::nullopt;
	}

	const auto jointIndex = sizukuSkeleton_->FindJointIndex(jointName);
	if (!jointIndex.has_value()) {
		return std::nullopt;
	}

	const auto& joints = sizukuSkeleton_->GetJoints();
	if (*jointIndex < 0 || static_cast<size_t>(*jointIndex) >= joints.size()) {
		return std::nullopt;
	}

	const Matrix4x4 playerWorld = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	return Function::Multiply(joints[*jointIndex].skeletonSpaceMatrix, playerWorld);
}