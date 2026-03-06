#pragma once
#include "Animation.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

class AnimationManager {
public:
	struct PlaybackResult {
		const Animation::AnimationData* animationToApply = nullptr;
		const Animation::AnimationData* currentAnimation = nullptr;
		float animationTime = 0.0f;
		bool animationFinished = false;
		bool changedAnimation = false;
	};

	static AnimationManager* GetInstance();
	static void Finalize();

	void LoadAnimationGroup(const std::string& groupName, const std::string& directoryPath, const std::string& filename);
	const Animation::AnimationData* FindAnimation(const std::string& groupName, const std::string& animationName) const;
	void ResetPlayback(const std::string& groupName, const std::string& animationName, bool loop);
	bool UpdatePlayback(
	    const std::string& groupName, const std::string& desiredAnimationName, bool loop, float deltaTime, float blendDuration, Animation::AnimationData& blendedPoseBuffer, PlaybackResult& outResult);

private:
	struct AnimationGroup {
		std::vector<Animation::AnimationData> clips;
		std::map<std::string, size_t> nameToIndex;
	};

	struct PlaybackState {
		size_t currentIndex = 0;
		float currentTime = 0.0f;
		bool currentLoop = true;
		size_t previousIndex = 0;
		float previousTime = 0.0f;
		float blendTime = 0.0f;
		bool isBlending = false;
		bool initialized = false;
	};

	static std::unique_ptr<AnimationManager> instance_;
	std::map<std::string, AnimationGroup> animationGroups_;
	std::map<std::string, PlaybackState> playbackStates_;

	size_t ResolveAnimationIndex(const AnimationGroup& group, const std::string& animationName, size_t fallbackIndex) const;
};