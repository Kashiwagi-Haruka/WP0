
#pragma once

#include "CameraForGPU.h"
#include "Material.h"
#include "Matrix4x4.h"
#include "Transform.h"
#include "VertexData.h"

#include <d3d12.h>
#include <string>
#include <wrl.h>

class Camera;

class PortalMesh {
public:
	void Initialize(const std::string& texturePath);
	void Update();
	void Draw();

	void SetTransform(const Transform& transform) { transform_ = transform; }
	void SetWorldMatrix(const Matrix4x4& worldMatrix) {
		worldMatrix_ = worldMatrix;
		useWorldMatrix_ = true;
	}
	void SetUvTransform(const Matrix4x4& uvTransform);
	void SetColor(const Vector4& color);
	void SetTextureIndex(uint32_t textureIndex) { textureIndex_ = textureIndex; }
	void SetObjectCamera(Camera* camera) { objectCamera_ = camera; }
	void SetTextureCamera(Camera* camera) { textureCamera_ = camera; }

private:
	struct alignas(256) TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 LightWVP;
		Matrix4x4 World;
		Matrix4x4 WorldInverseTranspose;
	};

	struct alignas(256) PortalTextureCameraForGpu {
		Matrix4x4 textureViewProjection;
		Matrix4x4 portalCameraWorld;
		Matrix4x4 textureWorldViewProjection;
		Vector3 textureWorldPosition;
		int usePortalProjection;
		float padding[3];
	};

	Transform transform_{
	    {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };
	Matrix4x4 worldMatrix_{};
	bool useWorldMatrix_ = false;

	Camera* objectCamera_ = nullptr;
	Camera* textureCamera_ = nullptr;

	uint32_t textureIndex_ = 0;

	Microsoft::WRL::ComPtr<ID3D12Resource> transformResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> objectCameraResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> textureCameraResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	TransformationMatrix* transformationMatrixData_ = nullptr;
	CameraForGpu* objectCameraData_ = nullptr;
	PortalTextureCameraForGpu* textureCameraData_ = nullptr;
	Material* materialData_ = nullptr;
};