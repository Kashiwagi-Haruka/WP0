#include "PortalMesh.h"

#include "Camera.h"
#include "Function.h"
#include "Object3d/Object3dCommon.h"
#include "Engine/Texture/TextureManager.h"
#include "WinApp.h"
#include "Engine/base/DirectXCommon.h"
#include "SrvManager/SrvManager.h"
#include <cstring>

void PortalMesh::Initialize(const std::string& texturePath) {
	textureIndex_ = TextureManager::GetInstance()->GetTextureIndexByfilePath(texturePath);
	worldMatrix_ = Function::MakeIdentity4x4();

	VertexData vertices[4] = {
	    {{-0.5f, 0.5f, 0.0f, 1.0f},  {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
	    {{0.5f, 0.5f, 0.0f, 1.0f},   {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
	    {{-0.5f, -0.5f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},
	    {{0.5f, -0.5f, 0.0f, 1.0f},  {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},
	};
	uint32_t indices[6] = {0, 1, 2, 2, 1, 3};

	vertexResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(vertices));
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(vertices);
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	VertexData* mappedVertices = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedVertices));
	std::memcpy(mappedVertices, vertices, sizeof(vertices));
	vertexResource_->Unmap(0, nullptr);

	indexResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(indices));
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(indices);
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
	uint32_t* mappedIndices = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedIndices));
	std::memcpy(mappedIndices, indices, sizeof(indices));
	indexResource_->Unmap(0, nullptr);

	transformResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(TransformationMatrix));
	objectCameraResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(CameraForGpu));
	textureCameraResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(PortalTextureCameraForGpu));
	const size_t alignedMaterialSize = (sizeof(Material) + 0xFF) & ~0xFF;
	materialResource_ = Object3dCommon::GetInstance()->CreateBufferResource(alignedMaterialSize);

	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = {1.0f, 1.0f, 1.0f, 1.0f};
	materialData_->enableLighting = false;
	materialData_->uvTransform = Function::MakeIdentity4x4();
	materialData_->shininess = 20.0f;
	materialData_->environmentCoefficient = 0.0f;
	materialData_->grayscaleEnabled = false;
	materialData_->sepiaEnabled = false;
	materialData_->distortionStrength = 0.1f;
	materialData_->distortionFalloff = 1.0f;
	materialResource_->Unmap(0, nullptr);
}

void PortalMesh::Update() {
	Camera* activeObjectCamera = Object3dCommon::GetInstance()->GetDefaultCamera();
	Camera* activeTextureCamera = textureCamera_;
	if (!activeObjectCamera) {
		return;
	}

	if (!useWorldMatrix_) {
		worldMatrix_ = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	}
	const bool hasTextureCamera = (activeTextureCamera != nullptr);
	const Matrix4x4 worldViewProjectionMatrix = Function::Multiply(Function::Multiply(worldMatrix_, activeObjectCamera->GetViewMatrix()), activeObjectCamera->GetProjectionMatrix());

	transformResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	transformationMatrixData_->WVP = worldViewProjectionMatrix;
	transformationMatrixData_->World = worldMatrix_;
	transformationMatrixData_->LightWVP = Function::Multiply(worldMatrix_, Object3dCommon::GetInstance()->GetDirectionalLightViewProjectionMatrix());
	transformationMatrixData_->WorldInverseTranspose = Function::Inverse(worldMatrix_);
	transformResource_->Unmap(0, nullptr);

	textureCameraResource_->Map(0, nullptr, reinterpret_cast<void**>(&textureCameraData_));
	textureCameraData_->textureViewProjection = hasTextureCamera ? activeTextureCamera->GetViewProjectionMatrix() : Function::MakeIdentity4x4();
	textureCameraData_->portalCameraWorld = hasTextureCamera ? activeTextureCamera->GetWorldMatrix() : Function::MakeIdentity4x4();
	textureCameraData_->textureWorldViewProjection =
	    hasTextureCamera ? Function::Multiply(Function::Multiply(worldMatrix_, activeTextureCamera->GetViewMatrix()), activeTextureCamera->GetProjectionMatrix()) : Function::MakeIdentity4x4();
	textureCameraData_->textureWorldPosition = hasTextureCamera ? activeTextureCamera->GetWorldTranslate() : Vector3{0.0f, 0.0f, 0.0f};
	textureCameraData_->usePortalProjection = hasTextureCamera ? 1 : 0;

	textureCameraResource_->Unmap(0, nullptr);

	objectCameraResource_->Map(0, nullptr, reinterpret_cast<void**>(&objectCameraData_));
	objectCameraData_->worldPosition = activeObjectCamera->GetWorldTranslate();
	objectCameraData_->screenSize = {static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight)};
	objectCameraData_->fullscreenGrayscaleEnabled = Object3dCommon::GetInstance()->IsFullScreenGrayscaleEnabled() ? 1 : 0;
	objectCameraData_->fullscreenSepiaEnabled = Object3dCommon::GetInstance()->IsFullScreenSepiaEnabled() ? 1 : 0;
	objectCameraData_->usePortalProjection = 1;
	objectCameraResource_->Unmap(0, nullptr);
}

void PortalMesh::Draw() {
	ID3D12DescriptorHeap* descriptorHeaps[] = {SrvManager::GetInstance()->GetDescriptorHeap().Get()};
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView_);
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformResource_->GetGPUVirtualAddress());
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(4, objectCameraResource_->GetGPUVirtualAddress());
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(5, textureCameraResource_->GetGPUVirtualAddress());
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex_));
	SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(8, Object3dCommon::GetInstance()->GetPointLightSrvIndex());
	SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(9, Object3dCommon::GetInstance()->GetSpotLightSrvIndex());
	SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(10, Object3dCommon::GetInstance()->GetAreaLightSrvIndex());
	if (!Object3dCommon::GetInstance()->IsShadowMapPassActive()) {
		SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(12, Object3dCommon::GetInstance()->GetShadowMapSrvIndex());
	}
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void PortalMesh::SetUvTransform(const Matrix4x4& uvTransform) {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->uvTransform = uvTransform;
	materialResource_->Unmap(0, nullptr);
}

void PortalMesh::SetColor(const Vector4& color) {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = color;
	materialResource_->Unmap(0, nullptr);
}