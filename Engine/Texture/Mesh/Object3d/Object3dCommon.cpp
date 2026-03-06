#define NOMINMAX
#include "Object3d/Object3dCommon.h"
#include "DirectXCommon.h"
#include "Function.h"
#include "Logger.h"
#include "SrvManager/SrvManager.h"
#include "TextureManager.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>

std::unique_ptr<Object3dCommon> Object3dCommon::instance = nullptr;

Object3dCommon::Object3dCommon() {}
Object3dCommon::~Object3dCommon() {}

Object3dCommon* Object3dCommon::GetInstance() {

	if (instance == nullptr) {
		instance = std::make_unique<Object3dCommon>();
	}
	return instance.get();
}

void Object3dCommon::Finalize() { instance.reset(); }

void Object3dCommon::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;
	pso_ = std::make_unique<CreatePSO>(dxCommon_);
	pso_->Create(D3D12_CULL_MODE_BACK);

	psoToon_ = std::make_unique<CreatePSO>(dxCommon_);
	psoToon_->Create(D3D12_CULL_MODE_BACK, true, D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Resources/shader/Object3d/PS_Shader/Object3dToon.PS.hlsl");

	psoEmissive_ = std::make_unique<CreatePSO>(dxCommon_);
	psoEmissive_->Create(D3D12_CULL_MODE_NONE, true, D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Resources/shader/Object3d/PS_Shader/Object3dEmissive.PS.hlsl");

	psoNoCull_ = std::make_unique<CreatePSO>(dxCommon_);
	psoNoCull_->Create(D3D12_CULL_MODE_NONE);

	psoNoDepth_ = std::make_unique<CreatePSO>(dxCommon_);
	psoNoDepth_->Create(D3D12_CULL_MODE_BACK, false);

	psoNoCullDepth_ = std::make_unique<CreatePSO>(dxCommon_);
	psoNoCullDepth_->Create(D3D12_CULL_MODE_NONE, false);

	psoWireframe_ = std::make_unique<CreatePSO>(dxCommon_);
	psoWireframe_->Create(D3D12_CULL_MODE_NONE, true, D3D12_FILL_MODE_WIREFRAME);

	psoWireframeNoDepth_ = std::make_unique<CreatePSO>(dxCommon_);
	psoWireframeNoDepth_->Create(D3D12_CULL_MODE_NONE, false, D3D12_FILL_MODE_WIREFRAME);

	psoLine_ = std::make_unique<CreatePSO>(dxCommon_);
	psoLine_->Create(D3D12_CULL_MODE_NONE, true, D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);

	psoLineNoDepth_ = std::make_unique<CreatePSO>(dxCommon_);
	psoLineNoDepth_->Create(D3D12_CULL_MODE_NONE, false, D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);

	psoEditorGrid_ = std::make_unique<CreatePSO>(dxCommon_);
	psoEditorGrid_->Create(D3D12_CULL_MODE_NONE, true, D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Resources/shader/Object3d/PS_Shader/Object3dGrid.PS.hlsl");

	psoSkinning_ = std::make_unique<CreatePSO>(dxCommon_, true);
	psoSkinning_->Create(D3D12_CULL_MODE_BACK);

	psoSkinningToon_ = std::make_unique<CreatePSO>(dxCommon_, true);
	psoSkinningToon_->Create(D3D12_CULL_MODE_BACK, true, D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Resources/shader/Object3d/PS_Shader/Object3dToon.PS.hlsl");

	SetEnvironmentMapTexture("Resources/3d/skydome.png");

	psoMirror_ = std::make_unique<CreatePSO>(dxCommon_);
	psoMirror_->Create(D3D12_CULL_MODE_BACK, true, D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Resources/shader/Object3d/PS_Shader/Object3dMirror.PS.hlsl");

	psoPortal_ = std::make_unique<CreatePSO>(dxCommon_);
	psoPortal_->Create(
	    D3D12_CULL_MODE_BACK, true, D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Resources/shader/Object3d/PS_Shader/Object3dPortal.PS.hlsl",
	    L"Resources/shader/Object3d/VS_Shader/Object3dPortal.VS.hlsl");

	psoShadow_ = std::make_unique<CreatePSO>(dxCommon_);
	psoShadow_->CreateShadow();

	// Directional Light の共通バッファ作成
	directionalLightResource_ = CreateBufferResource(sizeof(DirectionalLight));
	assert(directionalLightResource_);
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	assert(directionalLightData_);

	*directionalLightData_ = {
	    {1.0f, 1.0f, 1.0f, 1.0f},
        {0.0f, -1.0f, 0.0f},
        1.0f
    };
	directionalLightResource_->Unmap(0, nullptr);
	pointLightResource_ = CreateBufferResource(sizeof(PointLight) * kMaxPointLights);
	assert(pointLightResource_);
	pointLightCountResource_ = CreateBufferResource(sizeof(PointLightCount));
	assert(pointLightCountResource_);

	pointLightSrvIndex_ = SrvManager::GetInstance()->Allocate();
	SrvManager::GetInstance()->CreateSRVforStructuredBuffer(pointLightSrvIndex_, pointLightResource_.Get(), static_cast<UINT>(kMaxPointLights), sizeof(PointLight));

	spotLightResource_ = CreateBufferResource(sizeof(SpotLight) * kMaxSpotLights);
	assert(spotLightResource_);
	spotLightCountResource_ = CreateBufferResource(sizeof(SpotLightCount));
	assert(spotLightCountResource_);

	spotLightSrvIndex_ = SrvManager::GetInstance()->Allocate();
	SrvManager::GetInstance()->CreateSRVforStructuredBuffer(spotLightSrvIndex_, spotLightResource_.Get(), static_cast<UINT>(kMaxSpotLights), sizeof(SpotLight));

	areaLightResource_ = CreateBufferResource(sizeof(AreaLight) * kMaxAreaLights);
	assert(areaLightResource_);
	areaLightCountResource_ = CreateBufferResource(sizeof(AreaLightCount));
	assert(areaLightCountResource_);

	areaLightSrvIndex_ = SrvManager::GetInstance()->Allocate();
	SrvManager::GetInstance()->CreateSRVforStructuredBuffer(areaLightSrvIndex_, areaLightResource_.Get(), static_cast<UINT>(kMaxAreaLights), sizeof(AreaLight));
	editorDirectionalLight_ = *directionalLightData_;
	editorPointLightCount_ = 0;
	editorSpotLightCount_ = 0;
	editorAreaLightCount_ = 0;
	std::fill(editorPointLights_.begin(), editorPointLights_.end(), PointLight{});
	std::fill(editorSpotLights_.begin(), editorSpotLights_.end(), SpotLight{});
	std::fill(editorAreaLights_.begin(), editorAreaLights_.end(), AreaLight{});
	D3D12_HEAP_PROPERTIES heapProps{};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC shadowDesc{};
	shadowDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	shadowDesc.Width = kShadowMapSize_;
	shadowDesc.Height = kShadowMapSize_;
	shadowDesc.DepthOrArraySize = 1;
	shadowDesc.MipLevels = 1;
	shadowDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	shadowDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;

	HRESULT shadowHr =
	    dxCommon_->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &shadowDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearValue, IID_PPV_ARGS(&shadowMapResource_));
	assert(SUCCEEDED(shadowHr));

	shadowDsvHeap_ = dxCommon_->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
	D3D12_DEPTH_STENCIL_VIEW_DESC shadowDsvDesc{};
	shadowDsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dxCommon_->GetDevice()->CreateDepthStencilView(shadowMapResource_.Get(), &shadowDsvDesc, shadowDsvHeap_->GetCPUDescriptorHandleForHeapStart());

	shadowMapSrvIndex_ = SrvManager::GetInstance()->Allocate();
	SrvManager::GetInstance()->CreateSRVforTexture2D(shadowMapSrvIndex_, shadowMapResource_.Get(), DXGI_FORMAT_R32_FLOAT, 1);

	shadowViewport_.TopLeftX = 0.0f;
	shadowViewport_.TopLeftY = 0.0f;
	shadowViewport_.Width = static_cast<float>(kShadowMapSize_);
	shadowViewport_.Height = static_cast<float>(kShadowMapSize_);
	shadowViewport_.MinDepth = 0.0f;
	shadowViewport_.MaxDepth = 1.0f;
	shadowScissorRect_.left = 0;
	shadowScissorRect_.top = 0;
	shadowScissorRect_.right = static_cast<LONG>(kShadowMapSize_);
	shadowScissorRect_.bottom = static_cast<LONG>(kShadowMapSize_);
}
void Object3dCommon::SetEnvironmentMapTexture(const std::string& filePath) {
	environmentMapPath_ = filePath;
	TextureManager::GetInstance()->LoadTextureName(environmentMapPath_);
	environmentMapSrvIndex_ = TextureManager::GetInstance()->GetTextureIndexByfilePath(environmentMapPath_);
}
void Object3dCommon::SetEnvironmentMapTextureResource(ID3D12Resource* resource, DXGI_FORMAT format) {
	if (!resource) {
		return;
	}
	environmentMapSrvIndex_ = SrvManager::GetInstance()->Allocate();
	SrvManager::GetInstance()->CreateSRVforTexture2D(environmentMapSrvIndex_, resource, format, 1);
}
void Object3dCommon::DrawSet(){
	if (useEditorLights_) {
		SetDirectionalLight(editorDirectionalLight_);
		SetPointLights(editorPointLights_.data(), editorPointLightCount_);
		SetSpotLights(editorSpotLights_.data(), editorSpotLightCount_);
		SetAreaLights(editorAreaLights_.data(), editorAreaLightCount_);
	}
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(3, Object3dCommon::GetInstance()->GetDirectionalLightResource()->GetGPUVirtualAddress());
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(5, Object3dCommon::GetInstance()->GetPointLightCountResource()->GetGPUVirtualAddress());
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(6, Object3dCommon::GetInstance()->GetSpotLightCountResource()->GetGPUVirtualAddress());
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(7, Object3dCommon::GetInstance()->GetAreaLightCountResource()->GetGPUVirtualAddress());
}
void Object3dCommon::DrawCommon() {

	dxCommon_->GetCommandList()->SetGraphicsRootSignature(pso_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(pso_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommon(Camera* camera) {
	Camera* previousCamera = defaultCamera;
	if (camera) {
		defaultCamera = camera;
	}
	DrawCommon();
	defaultCamera = previousCamera;
}
void Object3dCommon::DrawCommonEmissive() {
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoEmissive_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoEmissive_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonNoCull() {

	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoNoCull_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoNoCull_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonNoDepth() {
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoNoDepth_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoNoDepth_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonToon() {

	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoToon_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoToon_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonNoCullDepth() {
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoNoCullDepth_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoNoCullDepth_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonWireframeNoDepth() {
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoWireframeNoDepth_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoWireframeNoDepth_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonLineNoDepth() {
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoLineNoDepth_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoLineNoDepth_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
}
void Object3dCommon::DrawCommonEditorGrid() {
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoEditorGrid_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoEditorGrid_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonSkinning() {

	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoSkinning_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoSkinning_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonSkinningToon() {

	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoSkinningToon_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoSkinningToon_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonMirror() {

	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoMirror_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoMirror_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonPortal() {

	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoPortal_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoPortal_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonPortal(Camera* camera) {
	Camera* previousCamera = defaultCamera;
	if (camera) {
		defaultCamera = camera;
	}
	DrawCommonPortal();
	defaultCamera = previousCamera;
}
void Object3dCommon::DrawCommonShadow() {
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoShadow_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoShadow_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Object3dCommon::BeginShadowMapPass() {
	if (!shadowMapResource_) {
		return;
	}
	isShadowMapPassActive_ = true;
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = shadowMapResource_.Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = shadowDsvHeap_->GetCPUDescriptorHandleForHeapStart();
	dxCommon_->GetCommandList()->OMSetRenderTargets(0, nullptr, false, &dsvHandle);
	dxCommon_->GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	dxCommon_->GetCommandList()->RSSetViewports(1, &shadowViewport_);
	dxCommon_->GetCommandList()->RSSetScissorRects(1, &shadowScissorRect_);
}

void Object3dCommon::EndShadowMapPass() {
	if (!shadowMapResource_) {
		return;
	}
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = shadowMapResource_.Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);
	isShadowMapPassActive_ = false;
}
void Object3dCommon::SetDirectionalLight(DirectionalLight& light) {
	*directionalLightData_ = light;
	directionalLightData_->direction = Function::Normalize(directionalLightData_->direction);
}

Matrix4x4 Object3dCommon::GetDirectionalLightViewProjectionMatrix() const {
	Vector3 lightDirection = Function::Normalize(directionalLightData_->direction);
	if (Function::Length(lightDirection) < 1.0e-5f) {
		lightDirection = {0.0f, -1.0f, 0.0f};
	}
	const Vector3 lightPosition = shadowLightPosition_ - lightDirection * 120.0f;
	const Vector3 up = (std::abs(lightDirection.y) > 0.99f) ? Vector3{0.0f, 0.0f, 1.0f} : Vector3{0.0f, 1.0f, 0.0f};
	const Vector3 right = Function::Normalize(Function::Cross(up, lightDirection));
	const Vector3 cameraUp = Function::Cross(lightDirection, right);

	Matrix4x4 lightView = Function::MakeIdentity4x4();
	lightView.m[0][0] = right.x;
	lightView.m[1][0] = right.y;
	lightView.m[2][0] = right.z;
	lightView.m[0][1] = cameraUp.x;
	lightView.m[1][1] = cameraUp.y;
	lightView.m[2][1] = cameraUp.z;
	lightView.m[0][2] = lightDirection.x;
	lightView.m[1][2] = lightDirection.y;
	lightView.m[2][2] = lightDirection.z;
	lightView.m[3][0] = -Function::Dot(lightPosition, right);
	lightView.m[3][1] = -Function::Dot(lightPosition, cameraUp);
	lightView.m[3][2] = -Function::Dot(lightPosition, lightDirection);

	Matrix4x4 lightProjection = Function::MakeOrthographicMatrix(-shadowOrthoHalfWidth_, shadowOrthoHalfHeight_, shadowOrthoHalfWidth_, -shadowOrthoHalfHeight_, shadowCameraNear_, shadowCameraFar_);
	return Function::Multiply(lightView, lightProjection);
}
void Object3dCommon::SetBlendMode(BlendMode blendMode) {
	blendMode_ = blendMode;
	dxCommon_->GetCommandList()->SetPipelineState(pso_->GetGraphicsPipelineState(blendMode_).Get());
}
void Object3dCommon::SetPointLights(const PointLight* pointLights, uint32_t count) {
	uint32_t clampedCount = std::min(count, static_cast<uint32_t>(kMaxPointLights));
	pointLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&pointlightData_));
	std::memset(pointlightData_, 0, sizeof(PointLight) * kMaxPointLights);
	if (pointLights && clampedCount > 0) {
		std::memcpy(pointlightData_, pointLights, sizeof(PointLight) * clampedCount);
	}
	pointLightResource_->Unmap(0, nullptr);

	pointLightCountResource_->Map(0, nullptr, reinterpret_cast<void**>(&pointLightCountData_));
	pointLightCountData_->count = clampedCount;
	pointLightCountResource_->Unmap(0, nullptr);
}
void Object3dCommon::SetSpotLights(const SpotLight* spotLights, uint32_t count) {
	uint32_t clampedCount = std::min(count, static_cast<uint32_t>(kMaxSpotLights));
	spotLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData_));
	std::memset(spotLightData_, 0, sizeof(SpotLight) * kMaxSpotLights);
	if (spotLights && clampedCount > 0) {
		std::memcpy(spotLightData_, spotLights, sizeof(SpotLight) * clampedCount);
	}
	spotLightResource_->Unmap(0, nullptr);

	spotLightCountResource_->Map(0, nullptr, reinterpret_cast<void**>(&spotLightCountData_));
	spotLightCountData_->count = clampedCount;
	spotLightCountResource_->Unmap(0, nullptr);
}
void Object3dCommon::SetAreaLights(const AreaLight* areaLights, uint32_t count) {
	uint32_t clampedCount = std::min(count, static_cast<uint32_t>(kMaxAreaLights));
	areaLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&areaLightData_));
	std::memset(areaLightData_, 0, sizeof(AreaLight) * kMaxAreaLights);
	if (areaLights && clampedCount > 0) {
		std::memcpy(areaLightData_, areaLights, sizeof(AreaLight) * clampedCount);
	}
	areaLightResource_->Unmap(0, nullptr);

	areaLightCountResource_->Map(0, nullptr, reinterpret_cast<void**>(&areaLightCountData_));
	areaLightCountData_->count = clampedCount;
	areaLightCountResource_->Unmap(0, nullptr);
}
void Object3dCommon::SetEditorLights(
    const DirectionalLight& directionalLight, const PointLight* pointLights, uint32_t pointCount, const SpotLight* spotLights, uint32_t spotCount, const AreaLight* areaLights, uint32_t areaCount) {
	editorDirectionalLight_ = directionalLight;
	editorDirectionalLight_.direction = Function::Normalize(editorDirectionalLight_.direction);
	if (Function::Length(editorDirectionalLight_.direction) < 1.0e-5f) {
		editorDirectionalLight_.direction = {0.0f, -1.0f, 0.0f};
	}

	editorPointLightCount_ = std::min(pointCount, static_cast<uint32_t>(kMaxPointLights));
	std::fill(editorPointLights_.begin(), editorPointLights_.end(), PointLight{});
	if (pointLights && editorPointLightCount_ > 0) {
		std::copy_n(pointLights, editorPointLightCount_, editorPointLights_.begin());
	}

	editorSpotLightCount_ = std::min(spotCount, static_cast<uint32_t>(kMaxSpotLights));
	std::fill(editorSpotLights_.begin(), editorSpotLights_.end(), SpotLight{});
	if (spotLights && editorSpotLightCount_ > 0) {
		std::copy_n(spotLights, editorSpotLightCount_, editorSpotLights_.begin());
	}

	editorAreaLightCount_ = std::min(areaCount, static_cast<uint32_t>(kMaxAreaLights));
	std::fill(editorAreaLights_.begin(), editorAreaLights_.end(), AreaLight{});
	if (areaLights && editorAreaLightCount_ > 0) {
		std::copy_n(areaLights, editorAreaLightCount_, editorAreaLights_.begin());
	}
}
void Object3dCommon::SetVignetteStrength(float strength) { dxCommon_->SetVignetteStrength(strength); }

void Object3dCommon::SetRandomNoiseEnabled(bool enabled) { dxCommon_->SetRandomNoiseEnabled(enabled); }

void Object3dCommon::SetRandomNoiseScale(float scale) { dxCommon_->SetRandomNoiseScale(scale); }

void Object3dCommon::SetRandomNoiseBlendMode(int blendMode) { dxCommon_->SetRandomNoiseBlendMode(blendMode); }

Microsoft::WRL::ComPtr<ID3D12Resource> Object3dCommon::CreateBufferResource(size_t sizeInBytes) {
	// バッファの設定(UPLOAD用に変更)
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	Microsoft::WRL::ComPtr<ID3D12Resource> bufferResource = nullptr;

	HRESULT hr_ = dxCommon_->GetDevice()->CreateCommittedResource(
	    &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
	    D3D12_RESOURCE_STATE_GENERIC_READ, // Uploadならこれ
	    nullptr, IID_PPV_ARGS(&bufferResource));

	if (FAILED(hr_)) {
		return nullptr;
	}

	return bufferResource;
}