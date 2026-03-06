#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include <map>
#include <string>
#include <vector>

class Animation {
public:
	template<typename tValue> struct Keyframe {
		float time;
		tValue value;
	};

	using KeyframeVector3 = Keyframe<Vector3>;
	using KeyframeVector4 = Keyframe<Vector4>;
	using KeyframeFloat = Keyframe<float>;

	template<typename tValue> struct AnimationCurve {
		std::vector<Keyframe<tValue>> keyframes;
	};

	struct NodeAnimation {
		AnimationCurve<Vector3> translate;
		AnimationCurve<Vector4> rotation;
		AnimationCurve<Vector3> scale;
	};

	struct AnimationData {
		std::string name;
		float duration;
		std::map<std::string, NodeAnimation> nodeAnimations;
	};

	static AnimationData LoadAnimationData(const std::string& directoryPath, const std::string& filename);
	static AnimationData LoadAnimationData(const std::string& directoryPath, const std::string& filename, size_t animationIndex);
	static std::vector<AnimationData> LoadAnimationClips(const std::string& directoryPath, const std::string& filename);
	static Vector3 CalculateValue(const AnimationCurve<Vector3>& keyframes, float time);
	static Vector4 CalculateValue(const AnimationCurve<Vector4>& keyframes, float time);
	static float CalculateValue(const AnimationCurve<float>& keyframes, float time);
	static float AdvanceTime(const AnimationData& animation, float currentTime, float deltaTime, bool loop = true);
	static float CalculateBlendFactor(float elapsedTime, float blendDuration);
	static AnimationData BuildBlendedPose(const AnimationData& fromAnimation, float fromTime, const AnimationData& toAnimation, float toTime, float blendFactor);
	static void BuildBlendedPose(const AnimationData& fromAnimation, float fromTime, const AnimationData& toAnimation, float toTime, float blendFactor, AnimationData& outBlendedAnimation);
	static float CalculateValueOrDefault(const AnimationCurve<float>& keyframes, float time, float defaultValue);
};