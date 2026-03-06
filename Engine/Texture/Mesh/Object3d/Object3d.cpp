#include "Object3d/Object3d.h"
#include "Camera.h"
#include "DirectXCommon.h"
#include "Function.h"
#include "Engine/Editor/Hierarchy.h"
#include "Model/Model.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include "TextureManager.h"
#include <algorithm>
#include <cassert>
#include <cmath>

Object3d::~Object3d() { Hierarchy::GetInstance()->UnregisterObject3d(this); }

void Object3d::Initialize() {

	camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();
	CreateResources();
	isUseSetWorld = false;
	animationTime_ = 0.0f;
	SetColor({1.0f, 1.0f, 1.0f, 1.0f});
	SetEnableLighting(true);
	SetUvTransform(uvScale_, uvRotate_, uvTranslate_, uvAnchor_);
	SetShininess(40.0f);
	SetEnvironmentCoefficient(0.0f);
	SetGrayscaleEnabled(false);
	SetSepiaEnabled(false);
	SetDistortionStrength(0.0f);
	SetDistortionFalloff(1.0f);
	Hierarchy* Hierarchy = Hierarchy::GetInstance();
	Hierarchy->RegisterObject3d(this);
}
namespace {
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
} // namespace

void Object3d::Update() {
	// [0]=モデル描画用で使う

	const Model::Node* baseNode = nullptr;
	if (model_) {
		const auto& rootNode = model_->GetModelData().rootnode;
		baseNode = &rootNode;
		if (IsIdentityMatrix(rootNode.localMatrix) && rootNode.children.size() == 1) {
			baseNode = &rootNode.children.front();
		}
	}
	Matrix4x4 localMatrix = baseNode ? baseNode->localMatrix : Function::MakeIdentity4x4();
	if (animation_ && model_) {
		float deltaTime = 1.0f / 60.0f;

		deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();

		animationTime_ = Animation::AdvanceTime(*animation_, animationTime_, deltaTime, isLoopAnimation_);

		const Animation::NodeAnimation* nodeAnimation = nullptr;
		if (baseNode) {
			auto it = animation_->nodeAnimations.find(baseNode->name);
			if (it != animation_->nodeAnimations.end()) {
				nodeAnimation = &it->second;
			}
		}
		if (nodeAnimation) {
			Vector3 translate = nodeAnimation->translate.keyframes.empty() ? Vector3{0.0f, 0.0f, 0.0f} : Animation::CalculateValue(nodeAnimation->translate, animationTime_);
			Vector4 rotate = nodeAnimation->rotation.keyframes.empty() ? Vector4{0.0f, 0.0f, 0.0f, 1.0f} : Animation::CalculateValue(nodeAnimation->rotation, animationTime_);
			Vector3 scale = nodeAnimation->scale.keyframes.empty() ? Vector3{1.0f, 1.0f, 1.0f} : Animation::CalculateValue(nodeAnimation->scale, animationTime_);
			localMatrix = Function::MakeAffineMatrix(scale, rotate, translate);
		}
	}

	if (!isUseSetWorld) {
		worldMatrix = Function::Multiply(localMatrix, Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate));
	} else {
		worldMatrix = Function::Multiply(localMatrix, worldMatrix);
	}

	UpdateCameraMatrices();
}

void Object3d::UpdateBillboard()
{
	// [0]=モデル描画用で使う

	const Model::Node* baseNode = nullptr;
	if (model_) {
		const auto& rootNode = model_->GetModelData().rootnode;
		baseNode = &rootNode;
		if (IsIdentityMatrix(rootNode.localMatrix) && rootNode.children.size() == 1) {
			baseNode = &rootNode.children.front();
		}
	}
	Matrix4x4 localMatrix = baseNode ? baseNode->localMatrix : Function::MakeIdentity4x4();
	if (animation_ && model_) {
		float deltaTime = 1.0f / 60.0f;

		deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();

		animationTime_ = Animation::AdvanceTime(*animation_, animationTime_, deltaTime, isLoopAnimation_);

		const Animation::NodeAnimation* nodeAnimation = nullptr;
		if (baseNode) {
			auto it = animation_->nodeAnimations.find(baseNode->name);
			if (it != animation_->nodeAnimations.end()) {
				nodeAnimation = &it->second;
			}
		}
		if (nodeAnimation) {
			Vector3 translate = nodeAnimation->translate.keyframes.empty() ? Vector3{ 0.0f, 0.0f, 0.0f } : Animation::CalculateValue(nodeAnimation->translate, animationTime_);
			Vector4 rotate = nodeAnimation->rotation.keyframes.empty() ? Vector4{ 0.0f, 0.0f, 0.0f, 1.0f } : Animation::CalculateValue(nodeAnimation->rotation, animationTime_);
			Vector3 scale = nodeAnimation->scale.keyframes.empty() ? Vector3{ 1.0f, 1.0f, 1.0f } : Animation::CalculateValue(nodeAnimation->scale, animationTime_);
			localMatrix = Function::MakeAffineMatrix(scale, rotate, translate);
		}
	}

	// 板ポリをビルボード化
	// カメラの回転部分だけを抜き出す（平行移動は無視）
	Matrix4x4 view = camera_->GetViewMatrix();
	Matrix4x4 invView = Function::Inverse(view);

	// invView の回転成分だけを抽出（位置をゼロにする）
	Matrix4x4 invViewRot = invView;
	invViewRot.m[3][0] = invViewRot.m[3][1] = invViewRot.m[3][2] = 0.0f;

	// 回転行列を作る（カメラの回転を打ち消して自分の回転を反映）
	Matrix4x4 rotMat = Function::Multiply(Function::Multiply(Function::MakeRotateZMatrix(transform_.rotate.z), Function::Multiply(Function::MakeRotateXMatrix(transform_.rotate.x), Function::MakeRotateYMatrix(transform_.rotate.y))), invViewRot);

	// スケール・平行移動
	Matrix4x4 scaleMat = Function::MakeScaleMatrix(transform_.scale);
	Matrix4x4 transMat = Function::MakeTranslateMatrix(transform_.translate);

	// ワールド行列構築
	worldMatrix = Function::Multiply(Function::Multiply(scaleMat, rotMat), transMat);

	UpdateCameraMatrices();
}

void Object3d::UpdateCameraMatrices() {
	if (camera_) {
		worldViewProjectionMatrix = Function::Multiply(worldMatrix, Function::Multiply(camera_->GetViewMatrix(), camera_->GetProjectionMatrix()));
	} else {
		worldViewProjectionMatrix = worldMatrix;
	}
	transformResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));

	transformationMatrixData_->WVP = worldViewProjectionMatrix;
	transformationMatrixData_->World = worldMatrix;
	transformationMatrixData_->LightWVP = Function::Multiply(worldMatrix, Object3dCommon::GetInstance()->GetDirectionalLightViewProjectionMatrix());
	transformationMatrixData_->WorldInverseTranspose = Function::Inverse(worldMatrix);
	transformResource_->Unmap(0, nullptr);
	cameraResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));
	if (camera_) {
		cameraData_->worldPosition = camera_->GetWorldTranslate();

	} else {
		cameraData_->worldPosition = {transform_.translate};
	}
	cameraData_->screenSize = {static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight)};
	cameraData_->fullscreenGrayscaleEnabled = Object3dCommon::GetInstance()->IsFullScreenGrayscaleEnabled() ? 1 : 0;
	cameraData_->fullscreenSepiaEnabled = Object3dCommon::GetInstance()->IsFullScreenSepiaEnabled() ? 1 : 0;
	cameraResource_->Unmap(0, nullptr);
}
void Object3d::Draw() {

	// --- 座標変換行列CBufferの場所を設定 ---
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformResource_->GetGPUVirtualAddress());
	// --- 平行光源CBufferの場所を設定 ---
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraResource_->GetGPUVirtualAddress());
	if (model_) {
		model_->Draw(skinCluster_, materialResource_.Get());
	}
}

void Object3d::SetModel(const std::string& filePath) {
	modelInstance_ = ModelManager::GetInstance()->CreateModelInstance(filePath);
	if (modelInstance_) {
		model_ = modelInstance_.get();
		return;
	}
	model_ = ModelManager::GetInstance()->FindModel(filePath);
}
void Object3d::SetCamera(Camera* camera) { camera_ = camera; }
void Object3d::SetScale(Vector3 scale) {
	transform_.scale = scale;
	isUseSetWorld = false;
}
void Object3d::SetRotate(Vector3 rotate) {
	transform_.rotate = rotate;
	isUseSetWorld = false;
}
void Object3d::SetTranslate(Vector3 translate) {
	transform_.translate = translate;
	isUseSetWorld = false;
}
void Object3d::SetColor(Vector4 color) {
	if (materialData_) {
		materialData_->color = color;
	}
}
void Object3d::SetEnableLighting(bool enable) {
	if (materialData_) {
		materialData_->enableLighting = enable ? 1 : 0;
	}
}
void Object3d::SetGrayscaleEnabled(bool enable) {
	if (materialData_) {
		materialData_->grayscaleEnabled = enable ? 1 : 0;
	}
}
void Object3d::SetSepiaEnabled(bool enable) {
	if (materialData_) {
		materialData_->sepiaEnabled = enable ? 1 : 0;
	}
}
void Object3d::SetDistortionStrength(float strength) {
	if (materialData_) {
		materialData_->distortionStrength = strength;
	}
}
void Object3d::SetDistortionFalloff(float falloff) {
	if (materialData_) {
		materialData_->distortionFalloff = falloff;
	}
}
void Object3d::SetUvTransform(const Matrix4x4& uvTransform) {
	if (materialData_) {
		materialData_->uvTransform = uvTransform;
	}
}
void Object3d::SetUvTransform(Vector3 scale, Vector3 rotate, Vector3 translate, Vector2 anchor) {
	uvScale_ = scale;
	uvRotate_ = rotate;
	uvTranslate_ = translate;
	uvAnchor_ = anchor;
	SetUvTransform(Function::MakeAffineMatrix(uvScale_, uvRotate_, uvTranslate_, uvAnchor_));
}
void Object3d::SetUvAnchor(Vector2 anchor) {
	uvAnchor_ = anchor;
	SetUvTransform(Function::MakeAffineMatrix(uvScale_, uvRotate_, uvTranslate_, uvAnchor_));
}
void Object3d::SetShininess(float shininess) {
	if (materialData_) {
		materialData_->shininess = shininess;
	}
}
void Object3d::SetEnvironmentCoefficient(float coefficient) {
	if (materialData_) {
		materialData_->environmentCoefficient = coefficient;
	}
}
Vector4 Object3d::GetColor() const {
	if (materialData_) {
		return materialData_->color;
	}
	return {1.0f, 1.0f, 1.0f, 1.0f};
}
bool Object3d::IsLightingEnabled() const {
	if (materialData_) {
		return materialData_->enableLighting != 0;
	}
	return true;
}
float Object3d::GetShininess() const {
	if (materialData_) {
		return materialData_->shininess;
	}
	return 40.0f;
}
float Object3d::GetEnvironmentCoefficient() const {
	if (materialData_) {
		return materialData_->environmentCoefficient;
	}
	return 0.0f;
}
bool Object3d::IsGrayscaleEnabled() const {
	if (materialData_) {
		return materialData_->grayscaleEnabled != 0;
	}
	return false;
}
float Object3d::GetDistortionStrength() const {
	if (materialData_) {
		return materialData_->distortionStrength;
	}
	return 0.0f;
}
float Object3d::GetDistortionFalloff() const {
	if (materialData_) {
		return materialData_->distortionFalloff;
	}
	return 1.0f;
}
bool Object3d::IsSepiaEnabled() const {
	if (materialData_) {
		return materialData_->sepiaEnabled != 0;
	}
	return false;
}
void Object3d::CreateResources() {
	transformResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(TransformationMatrix));
	cameraResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(CameraForGpu));
	const size_t alignedMaterialSize = (sizeof(Material) + 0xFF) & ~0xFF;
	materialResource_ = Object3dCommon::GetInstance()->CreateBufferResource(alignedMaterialSize);
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
}