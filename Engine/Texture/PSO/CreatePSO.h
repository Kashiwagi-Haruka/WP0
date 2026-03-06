#pragma once
#include "BlendMode/BlendModeManager.h"
#include "Windows.h"
#include "wrl.h"
#include <d3d12.h>
#include <string>
class DirectXCommon;
class CreatePSO {

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_[6];

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob_;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob_;

	DirectXCommon* dxCommon_ = nullptr;
	bool useSkinning_ = false;
	bool isShadowPass_ = false;
	std::wstring vertexShaderPath_;
	std::wstring pixelShaderPath_;

	HRESULT hr_;

	BlendMode blendMode_;
	BlendModeManager blendModeManager_;

	void CreateRootSignature();
	void CreateGraphicsPipeline(D3D12_CULL_MODE cullMode, bool depthEnable, D3D12_FILL_MODE fillMode, D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType);

public:
	explicit CreatePSO(DirectXCommon* dxCommom, bool useSkinning = false);
	void Create(
	    D3D12_CULL_MODE cullMode, bool depthEnable = true, D3D12_FILL_MODE fillMode = D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
	    const std::wstring& pixelShaderPath = L"Resources/shader/Object3d/PS_Shader/Object3d.PS.hlsl", const std::wstring& vertexShaderPath = L"");
	void CreateShadow(D3D12_CULL_MODE cullMode = D3D12_CULL_MODE_BACK, D3D12_FILL_MODE fillMode = D3D12_FILL_MODE_SOLID);
	Microsoft::WRL::ComPtr<ID3D12RootSignature> GetRootSignature() { return rootSignature_; };
	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetGraphicsPipelineState(BlendMode blendMode) { return graphicsPipelineState_[blendMode]; };
};