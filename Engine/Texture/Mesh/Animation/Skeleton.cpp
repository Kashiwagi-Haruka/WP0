#include "Skeleton.h"
#include "Camera.h"
#include "Function.h"
#include "Object3d/Object3dCommon.h"
#include <cmath>

bool IsIdentityMatrix(const Matrix4x4& matrix) {
	const Matrix4x4 identity = Function::MakeIdentity4x4();
	const float epsilon = 1e-5f;
	return std::abs(matrix.m[0][0] - identity.m[0][0]) < epsilon && std::abs(matrix.m[0][1] - identity.m[0][1]) < epsilon && std::abs(matrix.m[0][2] - identity.m[0][2]) < epsilon &&
	       std::abs(matrix.m[0][3] - identity.m[0][3]) < epsilon && std::abs(matrix.m[1][0] - identity.m[1][0]) < epsilon && std::abs(matrix.m[1][1] - identity.m[1][1]) < epsilon &&
	       std::abs(matrix.m[1][2] - identity.m[1][2]) < epsilon && std::abs(matrix.m[1][3] - identity.m[1][3]) < epsilon && std::abs(matrix.m[2][0] - identity.m[2][0]) < epsilon &&
	       std::abs(matrix.m[2][1] - identity.m[2][1]) < epsilon && std::abs(matrix.m[2][2] - identity.m[2][2]) < epsilon && std::abs(matrix.m[2][3] - identity.m[2][3]) < epsilon &&
	       std::abs(matrix.m[3][0] - identity.m[3][0]) < epsilon && std::abs(matrix.m[3][1] - identity.m[3][1]) < epsilon && std::abs(matrix.m[3][2] - identity.m[3][2]) < epsilon &&
	       std::abs(matrix.m[3][3] - identity.m[3][3]) < epsilon;
}
int32_t CreateJoint(const Model::Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints) {
	Joint joint;
	joint.name = node.name;
	joint.transform = node.transform;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = Function::MakeIdentity4x4();
	joint.index = static_cast<int32_t>(joints.size());
	joint.parent = parent;
	joints.push_back(joint);

	const int32_t jointIndex = joint.index;
	for (const Model::Node& child : node.children) {
		int32_t childIndex = CreateJoint(child, jointIndex, joints);
		joints[jointIndex].children.push_back(childIndex);
	}

	return jointIndex;
}

Skeleton Skeleton::Create(const Model::Node& rootNode) {
	Skeleton skeleton;
	const Model::Node& baseNode = skeleton.GetRootNode(rootNode);
	skeleton.root_ = CreateJoint(baseNode, std::nullopt, skeleton.joints_);

	for (const Joint& joint : skeleton.joints_) {
		skeleton.jointMap_.emplace(joint.name, joint.index);
	}

	skeleton.Update();
	return skeleton;
}

const Model::Node& Skeleton::GetRootNode(const Model::Node& rootNode) const {
	if (IsIdentityMatrix(rootNode.localMatrix) && rootNode.children.size() == 1) {
		return rootNode.children.front();
	}
	return rootNode;
}
std::optional<int32_t> Skeleton::FindJointIndex(const std::string& name) const {
	auto it = jointMap_.find(name);
	if (it == jointMap_.end()) {
		return std::nullopt;
	}
	return it->second;
}
void Skeleton::Update() {
	for (Joint& joint : joints_) {
		joint.localMatrix = Function::MakeAffineMatrix(joint.transform.scale, joint.transform.quaternion, joint.transform.translate);
		if (joint.parent) {
			joint.skeletonSpaceMatrix = Function::Multiply(joint.localMatrix, joints_[*joint.parent].skeletonSpaceMatrix);
		} else {
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}

void Skeleton::ApplyAnimation(const Animation::AnimationData& animation, float animationTime) {
	for (Joint& joint : joints_) {
		auto it = animation.nodeAnimations.find(joint.name);
		if (it == animation.nodeAnimations.end()) {
			continue;
		}

		const Animation::NodeAnimation& nodeAnimation = it->second;
		if (!nodeAnimation.translate.keyframes.empty()) {
			joint.transform.translate = Animation::CalculateValue(nodeAnimation.translate, animationTime);
		}
		if (!nodeAnimation.rotation.keyframes.empty()) {
			joint.transform.quaternion = Animation::CalculateValue(nodeAnimation.rotation, animationTime);
		}
		if (!nodeAnimation.scale.keyframes.empty()) {
			joint.transform.scale = Animation::CalculateValue(nodeAnimation.scale, animationTime);
		}
	}
}

void Skeleton::SetObjectMatrix(const Matrix4x4& objectMatrix) { objectMatrix_ = objectMatrix; }

Vector3 Skeleton::GetJointWorldPosition(const Joint& joint) const {
	Matrix4x4 worldMatrix = Function::Multiply(joint.skeletonSpaceMatrix, objectMatrix_);
	return Function::TransformVM({0.0f, 0.0f, 0.0f}, worldMatrix);
}
void Skeleton::DrawBones(Camera* camera, const Vector4& jointColor, const Vector4& boneColor) {
	if (!camera) {
		return;
	}

	const size_t jointCount = joints_.size();
	if (debugJointPrimitives_.size() != jointCount) {
		debugJointPrimitives_.clear();
		debugJointPrimitives_.reserve(jointCount);
		for (size_t i = 0; i < jointCount; ++i) {
			auto primitive = std::make_unique<Primitive>();
			primitive->Initialize(Primitive::Sphere);
			debugJointPrimitives_.push_back(std::move(primitive));
		}
	}
	if (debugBonePrimitives_.size() != jointCount) {
		debugBonePrimitives_.clear();
		debugBonePrimitives_.reserve(jointCount);
		for (size_t i = 0; i < jointCount; ++i) {
			auto primitive = std::make_unique<Primitive>();
			primitive->Initialize(Primitive::Line);
			debugBonePrimitives_.push_back(std::move(primitive));
		}
	}

	Object3dCommon::GetInstance()->DrawCommonWireframeNoDepth();
	for (size_t i = 0; i < jointCount; ++i) {
		const Joint& joint = joints_[i];
		Vector3 jointPosition = GetJointWorldPosition(joint);
		Primitive* jointPrimitive = debugJointPrimitives_[i].get();
		jointPrimitive->SetCamera(camera);
		jointPrimitive->SetColor(jointColor);
		jointPrimitive->SetEnableLighting(false);
		jointPrimitive->SetTransform({
		    .scale{kJointRadius,    kJointRadius,    kJointRadius   },
		    .rotate{0.0f,            0.0f,            0.0f           },
		    .translate{jointPosition.x, jointPosition.y, jointPosition.z},
		});
		jointPrimitive->Update();
		jointPrimitive->Draw();
	}

	Object3dCommon::GetInstance()->DrawCommonLineNoDepth();
	for (size_t i = 0; i < jointCount; ++i) {
		const Joint& joint = joints_[i];
		if (!joint.parent.has_value()) {
			continue;
		}

		const Joint& parentJoint = joints_[*joint.parent];
		Vector3 jointPosition = GetJointWorldPosition(joint);
		Vector3 parentPosition = GetJointWorldPosition(parentJoint);
		Vector3 direction = jointPosition - parentPosition;
		float length = Function::Length(direction);
		if (length <= kBoneLengthEpsilon) {
			continue;
		}

		Vector3 center = {
		    (jointPosition.x + parentPosition.x) * 0.5f,
		    (jointPosition.y + parentPosition.y) * 0.5f,
		    (jointPosition.z + parentPosition.z) * 0.5f,
		};
		Vector3 axisX = Function::Normalize(direction);
		Vector3 up = {0.0f, 1.0f, 0.0f};
		if (std::abs(Function::Dot(axisX, up)) > 0.99f) {
			up = {0.0f, 0.0f, 1.0f};
		}
		Vector3 axisZ = Function::Normalize(Function::Cross(axisX, up));
		Vector3 axisY = Function::Cross(axisZ, axisX);
		Primitive* bonePrimitive = debugBonePrimitives_[i].get();
		bonePrimitive->SetCamera(camera);
		bonePrimitive->SetColor(boneColor);
		bonePrimitive->SetEnableLighting(false);
		bonePrimitive->SetLinePositions(parentPosition, jointPosition);
		bonePrimitive->Update();
		bonePrimitive->Draw();
	}
}