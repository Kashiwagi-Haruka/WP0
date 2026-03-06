#pragma once
#include "Animation/Animation.h"
#include "Matrix4x4.h"
#include "Model/Model.h"
#include "Primitive/Primitive.h"
#include "QuaternionTransform.h"
#include "Vector3.h"
#include "Vector4.h"
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct Joint {
	QuaternionTransform transform{};
	Matrix4x4 localMatrix{};
	Matrix4x4 skeletonSpaceMatrix{};
	std::string name;
	std::vector<int32_t> children;
	int32_t index = -1;
	std::optional<int32_t> parent;
};
class Camera;
class Skeleton {
public:
	Skeleton Create(const Model::Node& rootNode);
	const Model::Node& GetRootNode(const Model::Node& rootNode) const;
	const std::vector<Joint>& GetJoints() const { return joints_; }
	std::optional<int32_t> FindJointIndex(const std::string& name) const;
	void Update();
	void ApplyAnimation(const Animation::AnimationData& animation, float animationTime);
	void SetObjectMatrix(const Matrix4x4& objectMatrix);
	Vector3 GetJointWorldPosition(const Joint& joint) const;
	void DrawBones(Camera* camera, const Vector4& jointColor, const Vector4& boneColor);

private:
	int32_t root_ = -1;
	std::map<std::string, int32_t> jointMap_{};
	std::vector<Joint> joints_{};
	Matrix4x4 objectMatrix_{};
	float kJointRadius = 0.03f;
	float kBoneLengthEpsilon = 0.0001f;
	std::vector<std::unique_ptr<Primitive>> debugBonePrimitives_{};
	std::vector<std::unique_ptr<Primitive>> debugJointPrimitives_{};
};