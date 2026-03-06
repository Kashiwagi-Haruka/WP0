#include "Animation.h"
#include "Function.h"

#include <algorithm>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <cassert>
#include <cmath>

namespace {
float Dot(const Vector4& a, const Vector4& b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

Animation::AnimationData BuildAnimationData(const aiAnimation* animationAssimp, const std::string& fallbackName) {
	const double ticksPerSecond = (animationAssimp->mTicksPerSecond != 0.0) ? animationAssimp->mTicksPerSecond : 1.0;
	std::string name = animationAssimp->mName.C_Str();
	if (name.empty()) {
		name = fallbackName;
	}

	Animation::AnimationData animation;
	animation.name = name;
	animation.duration = static_cast<float>(animationAssimp->mDuration / ticksPerSecond);

	for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex) {
		const aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
		Animation::NodeAnimation& nodeAnimation = animation.nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];

		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex) {
			const aiVectorKey& keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
			Animation::KeyframeVector3 keyframe{};
			keyframe.time = static_cast<float>(keyAssimp.mTime / ticksPerSecond);
			keyframe.value = {-keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z};
			nodeAnimation.translate.keyframes.push_back(keyframe);
		}

		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex) {
			const aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
			Animation::KeyframeVector4 keyframe{};
			keyframe.time = static_cast<float>(keyAssimp.mTime / ticksPerSecond);
			keyframe.value = {keyAssimp.mValue.x, -keyAssimp.mValue.y, -keyAssimp.mValue.z, keyAssimp.mValue.w};
			nodeAnimation.rotation.keyframes.push_back(keyframe);
		}

		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex) {
			const aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
			Animation::KeyframeVector3 keyframe{};
			keyframe.time = static_cast<float>(keyAssimp.mTime / ticksPerSecond);
			keyframe.value = {keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z};
			nodeAnimation.scale.keyframes.push_back(keyframe);
		}
	}

	return animation;
}

Vector4 Normalize(const Vector4& v) {
	float length = std::sqrt(Dot(v, v));
	if (length == 0.0f) {
		return {0.0f, 0.0f, 0.0f, 1.0f};
	}
	return {v.x / length, v.y / length, v.z / length, v.w / length};
}

Vector4 Slerp(const Vector4& start, const Vector4& end, float ratio) {
	Vector4 normalizedStart = Normalize(start);
	Vector4 normalizedEnd = Normalize(end);
	float dot = Dot(normalizedStart, normalizedEnd);
	if (dot < 0.0f) {
		normalizedEnd = {-normalizedEnd.x, -normalizedEnd.y, -normalizedEnd.z, -normalizedEnd.w};
		dot = -dot;
	}

	const float kThreshold = 0.9995f;
	if (dot > kThreshold) {
		Vector4 result{
		    normalizedStart.x + (normalizedEnd.x - normalizedStart.x) * ratio,
		    normalizedStart.y + (normalizedEnd.y - normalizedStart.y) * ratio,
		    normalizedStart.z + (normalizedEnd.z - normalizedStart.z) * ratio,
		    normalizedStart.w + (normalizedEnd.w - normalizedStart.w) * ratio,
		};
		return Normalize(result);
	}

	float theta = std::acos(std::clamp(dot, -1.0f, 1.0f));
	float sinTheta = std::sin(theta);
	float startScale = std::sin((1.0f - ratio) * theta) / sinTheta;
	float endScale = std::sin(ratio * theta) / sinTheta;

	return {
	    startScale * normalizedStart.x + endScale * normalizedEnd.x,
	    startScale * normalizedStart.y + endScale * normalizedEnd.y,
	    startScale * normalizedStart.z + endScale * normalizedEnd.z,
	    startScale * normalizedStart.w + endScale * normalizedEnd.w,
	};
}

Animation::NodeAnimation BuildBlendedNodeAnimation(const Animation::NodeAnimation* fromNode, float fromTime, const Animation::NodeAnimation* toNode, float toTime, float blendFactor) {
	Animation::NodeAnimation blendedNode{};
	if (fromNode && !fromNode->translate.keyframes.empty() && toNode && !toNode->translate.keyframes.empty()) {
		Vector3 fromValue = Animation::CalculateValue(fromNode->translate, fromTime);
		Vector3 toValue = Animation::CalculateValue(toNode->translate, toTime);
		blendedNode.translate.keyframes.push_back({0.0f, Function::Lerp(fromValue, toValue, blendFactor)});
	} else if (toNode && !toNode->translate.keyframes.empty()) {
		blendedNode.translate.keyframes.push_back({0.0f, Animation::CalculateValue(toNode->translate, toTime)});
	} else if (fromNode && !fromNode->translate.keyframes.empty()) {
		blendedNode.translate.keyframes.push_back({0.0f, Animation::CalculateValue(fromNode->translate, fromTime)});
	}

	if (fromNode && !fromNode->rotation.keyframes.empty() && toNode && !toNode->rotation.keyframes.empty()) {
		Vector4 fromValue = Animation::CalculateValue(fromNode->rotation, fromTime);
		Vector4 toValue = Animation::CalculateValue(toNode->rotation, toTime);
		blendedNode.rotation.keyframes.push_back({0.0f, Slerp(fromValue, toValue, blendFactor)});
	} else if (toNode && !toNode->rotation.keyframes.empty()) {
		blendedNode.rotation.keyframes.push_back({0.0f, Animation::CalculateValue(toNode->rotation, toTime)});
	} else if (fromNode && !fromNode->rotation.keyframes.empty()) {
		blendedNode.rotation.keyframes.push_back({0.0f, Animation::CalculateValue(fromNode->rotation, fromTime)});
	}

	if (fromNode && !fromNode->scale.keyframes.empty() && toNode && !toNode->scale.keyframes.empty()) {
		Vector3 fromValue = Animation::CalculateValue(fromNode->scale, fromTime);
		Vector3 toValue = Animation::CalculateValue(toNode->scale, toTime);
		blendedNode.scale.keyframes.push_back({0.0f, Function::Lerp(fromValue, toValue, blendFactor)});
	} else if (toNode && !toNode->scale.keyframes.empty()) {
		blendedNode.scale.keyframes.push_back({0.0f, Animation::CalculateValue(toNode->scale, toTime)});
	} else if (fromNode && !fromNode->scale.keyframes.empty()) {
		blendedNode.scale.keyframes.push_back({0.0f, Animation::CalculateValue(fromNode->scale, fromTime)});
	}
	return blendedNode;
}
} // namespace

Animation::AnimationData Animation::LoadAnimationData(const std::string& directoryPath, const std::string& filename) { return LoadAnimationData(directoryPath, filename, 0); }

Animation::AnimationData Animation::LoadAnimationData(const std::string& directoryPath, const std::string& filename, size_t animationIndex) {
	std::string filePath = directoryPath + "/" + filename + ".gltf";

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath, 0);
	assert(scene);
	assert(scene->mNumAnimations != 0);
	assert(animationIndex < scene->mNumAnimations);

	const aiAnimation* animationAssimp = scene->mAnimations[animationIndex];
	std::string fallbackName = filename;
	if (animationIndex > 0) {
		fallbackName += "_" + std::to_string(animationIndex);
	}

	return BuildAnimationData(animationAssimp, fallbackName);
}

std::vector<Animation::AnimationData> Animation::LoadAnimationClips(const std::string& directoryPath, const std::string& filename) {
	std::string filePath = directoryPath + "/" + filename + ".gltf";

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath, 0);
	assert(scene);
	assert(scene->mNumAnimations != 0);

	std::vector<AnimationData> animations;
	animations.reserve(scene->mNumAnimations);
	for (size_t index = 0; index < scene->mNumAnimations; ++index) {
		std::string fallbackName = filename;
		if (index > 0) {
			fallbackName += "_" + std::to_string(index);
		}
		animations.push_back(BuildAnimationData(scene->mAnimations[index], fallbackName));
	}

	return animations;
}

Vector3 Animation::CalculateValue(const AnimationCurve<Vector3>& keyframes, float time) {
	assert(!keyframes.keyframes.empty());
	if (keyframes.keyframes.size() == 1 || time <= keyframes.keyframes.front().time) {
		return keyframes.keyframes.front().value;
	}

	for (size_t index = 0; index + 1 < keyframes.keyframes.size(); ++index) {
		const KeyframeVector3& current = keyframes.keyframes[index];
		const KeyframeVector3& next = keyframes.keyframes[index + 1];
		if (current.time <= time && time <= next.time) {
			float t = (time - current.time) / (next.time - current.time);
			return Function::Lerp(current.value, next.value, t);
		}
	}

	return keyframes.keyframes.back().value;
}

Vector4 Animation::CalculateValue(const AnimationCurve<Vector4>& keyframes, float time) {
	assert(!keyframes.keyframes.empty());
	if (keyframes.keyframes.size() == 1 || time <= keyframes.keyframes.front().time) {
		return keyframes.keyframes.front().value;
	}

	for (size_t index = 0; index + 1 < keyframes.keyframes.size(); ++index) {
		const KeyframeVector4& current = keyframes.keyframes[index];
		const KeyframeVector4& next = keyframes.keyframes[index + 1];
		if (current.time <= time && time <= next.time) {
			float t = (time - current.time) / (next.time - current.time);
			return Slerp(current.value, next.value, t);
		}
	}

	return keyframes.keyframes.back().value;
}
float Animation::CalculateValue(const AnimationCurve<float>& keyframes, float time) {
	assert(!keyframes.keyframes.empty());
	if (keyframes.keyframes.size() == 1 || time <= keyframes.keyframes.front().time) {
		return keyframes.keyframes.front().value;
	}

	for (size_t index = 0; index + 1 < keyframes.keyframes.size(); ++index) {
		const KeyframeFloat& current = keyframes.keyframes[index];
		const KeyframeFloat& next = keyframes.keyframes[index + 1];
		if (current.time <= time && time <= next.time) {
			float t = (time - current.time) / (next.time - current.time);
			return Function::Lerp(current.value, next.value, t);
		}
	}

	return keyframes.keyframes.back().value;
}
float Animation::AdvanceTime(const AnimationData& animation, float currentTime, float deltaTime, bool loop) {
	if (animation.duration <= 0.0f) {
		return 0.0f;
	}

	float nextTime = currentTime + deltaTime;
	if (loop) {
		nextTime = std::fmod(nextTime, animation.duration);
		if (nextTime < 0.0f) {
			nextTime += animation.duration;
		}
	} else {
		nextTime = std::clamp(nextTime, 0.0f, animation.duration);
	}
	return nextTime;
}
float Animation::CalculateBlendFactor(float elapsedTime, float blendDuration) {
	if (blendDuration <= 0.0f) {
		return 1.0f;
	}
	return std::clamp(elapsedTime / blendDuration, 0.0f, 1.0f);
}

Animation::AnimationData Animation::BuildBlendedPose(const AnimationData& fromAnimation, float fromTime, const AnimationData& toAnimation, float toTime, float blendFactor) {
	AnimationData blendedAnimation{};
	BuildBlendedPose(fromAnimation, fromTime, toAnimation, toTime, blendFactor, blendedAnimation);
	return blendedAnimation;
}

void Animation::BuildBlendedPose(const AnimationData& fromAnimation, float fromTime, const AnimationData& toAnimation, float toTime, float blendFactor, AnimationData& outBlendedAnimation) {
	blendFactor = std::clamp(blendFactor, 0.0f, 1.0f);
	outBlendedAnimation.name = "BlendedPose";
	outBlendedAnimation.duration = 0.0f;
	outBlendedAnimation.nodeAnimations.clear();

	for (const auto& [nodeName, fromNode] : fromAnimation.nodeAnimations) {
		const auto toIt = toAnimation.nodeAnimations.find(nodeName);
		const NodeAnimation* toNode = (toIt != toAnimation.nodeAnimations.end()) ? &toIt->second : nullptr;
		outBlendedAnimation.nodeAnimations[nodeName] = BuildBlendedNodeAnimation(&fromNode, fromTime, toNode, toTime, blendFactor);
	}

	for (const auto& [nodeName, toNode] : toAnimation.nodeAnimations) {
		if (outBlendedAnimation.nodeAnimations.find(nodeName) != outBlendedAnimation.nodeAnimations.end()) {
			continue;
		}
		outBlendedAnimation.nodeAnimations[nodeName] = BuildBlendedNodeAnimation(nullptr, fromTime, &toNode, toTime, blendFactor);
	}
}

float Animation::CalculateValueOrDefault(const AnimationCurve<float>& keyframes, float time, float defaultValue) {
	if (keyframes.keyframes.empty()) {
		return defaultValue;
	}
	return CalculateValue(keyframes, time);
}