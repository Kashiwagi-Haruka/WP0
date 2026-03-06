#pragma once

#include "Animation/Animation.h"
#include "CameraForGPU.h"
#include "Light/DirectionalLight.h"
#include "Matrix4x4.h"
#include "Model/Model.h"
#include "Transform.h"
#include "Vector2.h"
#include "Vector4.h"
#include "VertexData.h"
#include <Windows.h>
#include <d3d12.h>
#include <memory>
#include <string>
#include <wrl.h>
class Camera;
struct SkinCluster;

class Object3d {

	struct alignas(256) TransformationMatrix {
		Matrix4x4 WVP;                   // 64 バイト
		Matrix4x4 LightWVP;              // 64 バイト
		Matrix4x4 World;                 // 64 バイト
		Matrix4x4 WorldInverseTranspose; // 64 バイト
	};

	Transform transform_ = {
	    {1.0f, 1.0f, 1.0f},
	    {0.0f, 0.0f, 0.0f},
	    {0.0f, 0.0f, 0.0f},
	};

	Camera* camera_;

	TransformationMatrix* transformationMatrixData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformResource_;
	CameraForGpu* cameraData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource_;
	Material* materialData_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Model* model_ = nullptr;
	Matrix4x4 worldMatrix;
	Matrix4x4 worldViewProjectionMatrix;
	bool isUseSetWorld;
	std::unique_ptr<Model> modelInstance_;
	const Animation::AnimationData* animation_ = nullptr;
	float animationTime_ = 0.0f;
	bool isLoopAnimation_ = true;
	SkinCluster* skinCluster_ = nullptr;
	Vector3 uvScale_ = {1.0f, 1.0f, 1.0f};
	Vector3 uvRotate_ = {0.0f, 0.0f, 0.0f};
	Vector3 uvTranslate_ = {0.0f, 0.0f, 0.0f};
	Vector2 uvAnchor_ = {0.0f, 0.0f};

public:
	~Object3d();
	void Initialize();
	void Update();
	void UpdateBillboard();
	void UpdateCameraMatrices();
	void Draw();

	void CreateResources();
	void SetModel(const std::string& filePath);

	void SetCamera(Camera* camera);

	void SetTranslate(Vector3 translate);
	void SetRotate(Vector3 Rotate);
	void SetScale(Vector3 Scale);
	void SetTransform(Transform transform) {
		transform_ = transform;
		isUseSetWorld = false;
	}
	void SetWorldMatrix(Matrix4x4 matrix) {
		worldMatrix = matrix;
		isUseSetWorld = true;
	}
	void SetColor(Vector4 color);
	void SetEnableLighting(bool enable);
	void SetGrayscaleEnabled(bool enable);
	void SetSepiaEnabled(bool enable);
	void SetDistortionStrength(float strength);
	void SetDistortionFalloff(float falloff);
	void SetUvTransform(const Matrix4x4& uvTransform);
	void SetUvTransform(Vector3 scale, Vector3 rotate, Vector3 translate, Vector2 anchor = {0.0f, 0.0f});
	void SetUvAnchor(Vector2 anchor);
	void SetShininess(float shininess);
	void SetEnvironmentCoefficient(float coefficient);
	Vector4 GetColor() const;
	bool IsLightingEnabled() const;
	float GetShininess() const;
	float GetEnvironmentCoefficient() const;
	bool IsGrayscaleEnabled() const;
	bool IsSepiaEnabled() const;
	float GetDistortionStrength() const;
	float GetDistortionFalloff() const;
	Vector2 GetUvAnchor() const { return uvAnchor_; }
	void SetAnimation(const Animation::AnimationData* animation, bool loop = true) {
		animation_ = animation;
		isLoopAnimation_ = loop;
		animationTime_ = 0.0f;
	}
	void SetSkinCluster(SkinCluster* skinCluster) { skinCluster_ = skinCluster; }
	void ResetAnimationTime(float time = 0.0f) { animationTime_ = time; }
	Vector3 GetTranslate() { return transform_.translate; }
	Vector3 GetRotate() { return transform_.rotate; }
	Vector3 GetScale() { return transform_.scale; }
	Transform GetTransform() const { return transform_; }
	const Matrix4x4& GetWorldMatrix() const { return worldMatrix; }
};