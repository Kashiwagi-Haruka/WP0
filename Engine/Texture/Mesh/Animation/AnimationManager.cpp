#include "AnimationManager.h"

std::unique_ptr<AnimationManager> AnimationManager::instance_ = nullptr;

AnimationManager* AnimationManager::GetInstance() {
	if (!instance_) {
		instance_ = std::make_unique<AnimationManager>();
	}
	return instance_.get();
}

void AnimationManager::Finalize() { instance_.reset(); }

void AnimationManager::LoadAnimationGroup(const std::string& groupName, const std::string& directoryPath, const std::string& filename) {
	if (animationGroups_.contains(groupName)) {
		return;
	}

	AnimationGroup group;
	group.clips = Animation::LoadAnimationClips(directoryPath, filename);
	for (size_t i = 0; i < group.clips.size(); ++i) {
		if (!group.nameToIndex.contains(group.clips[i].name)) {
			group.nameToIndex[group.clips[i].name] = i;
		}
	}
	animationGroups_.emplace(groupName, std::move(group));
}

const Animation::AnimationData* AnimationManager::FindAnimation(const std::string& groupName, const std::string& animationName) const {
	auto groupIt = animationGroups_.find(groupName);
	if (groupIt == animationGroups_.end()) {
		return nullptr;
	}
	const AnimationGroup& group = groupIt->second;
	auto indexIt = group.nameToIndex.find(animationName);
	if (indexIt == group.nameToIndex.end()) {
		return nullptr;
	}
	return &group.clips[indexIt->second];
}

void AnimationManager::ResetPlayback(const std::string& groupName, const std::string& animationName, bool loop) {
	auto groupIt = animationGroups_.find(groupName);
	if (groupIt == animationGroups_.end() || groupIt->second.clips.empty()) {
		return;
	}
	const AnimationGroup& group = groupIt->second;
	PlaybackState& state = playbackStates_[groupName];
	state.currentIndex = ResolveAnimationIndex(group, animationName, 0);
	state.currentTime = 0.0f;
	state.currentLoop = loop;
	state.previousIndex = state.currentIndex;
	state.previousTime = 0.0f;
	state.blendTime = 0.0f;
	state.isBlending = false;
	state.initialized = true;
}

bool AnimationManager::UpdatePlayback(
    const std::string& groupName, const std::string& desiredAnimationName, bool loop, float deltaTime, float blendDuration, Animation::AnimationData& blendedPoseBuffer, PlaybackResult& outResult) {
	outResult = {};
	auto groupIt = animationGroups_.find(groupName);
	if (groupIt == animationGroups_.end() || groupIt->second.clips.empty()) {
		return false;
	}
	const AnimationGroup& group = groupIt->second;
	PlaybackState& state = playbackStates_[groupName];
	if (!state.initialized) {
		state.currentIndex = ResolveAnimationIndex(group, desiredAnimationName, 0);
		state.currentTime = 0.0f;
		state.currentLoop = loop;
		state.previousIndex = state.currentIndex;
		state.previousTime = 0.0f;
		state.blendTime = 0.0f;
		state.isBlending = false;
		state.initialized = true;
		outResult.changedAnimation = true;
	}

	size_t desiredIndex = ResolveAnimationIndex(group, desiredAnimationName, state.currentIndex);
	if (desiredIndex != state.currentIndex || loop != state.currentLoop) {
		state.previousIndex = state.currentIndex;
		state.previousTime = state.currentTime;
		state.currentIndex = desiredIndex;
		state.currentTime = 0.0f;
		state.currentLoop = loop;
		state.blendTime = 0.0f;
		state.isBlending = true;
		outResult.changedAnimation = true;
	}

	const Animation::AnimationData& currentAnimation = group.clips[state.currentIndex];
	outResult.currentAnimation = &currentAnimation;
	outResult.animationFinished = false;
	if (!loop && currentAnimation.duration > 0.0f) {
		outResult.animationFinished = (state.currentTime + deltaTime) >= currentAnimation.duration;
	}

	if (state.isBlending && state.previousIndex < group.clips.size()) {
		const Animation::AnimationData& previousAnimation = group.clips[state.previousIndex];
		state.previousTime = Animation::AdvanceTime(previousAnimation, state.previousTime, deltaTime, true);
		state.currentTime = Animation::AdvanceTime(currentAnimation, state.currentTime, deltaTime, loop);
		state.blendTime += deltaTime;
		float blendFactor = Animation::CalculateBlendFactor(state.blendTime, blendDuration);
		if (blendFactor >= 1.0f) {
			blendFactor = 1.0f;
			state.isBlending = false;
		}
		Animation::BuildBlendedPose(previousAnimation, state.previousTime, currentAnimation, state.currentTime, blendFactor, blendedPoseBuffer);
		outResult.animationToApply = &blendedPoseBuffer;
		outResult.animationTime = 0.0f;
		return true;
	}

	state.currentTime = Animation::AdvanceTime(currentAnimation, state.currentTime, deltaTime, loop);
	outResult.animationToApply = &currentAnimation;
	outResult.animationTime = state.currentTime;
	return true;
}

size_t AnimationManager::ResolveAnimationIndex(const AnimationGroup& group, const std::string& animationName, size_t fallbackIndex) const {
	auto it = group.nameToIndex.find(animationName);
	if (it != group.nameToIndex.end()) {
		return it->second;
	}
	if (fallbackIndex < group.clips.size()) {
		return fallbackIndex;
	}
	return 0;
}