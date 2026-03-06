#pragma once
#include "Windows.h"
#include "wrl.h"
#include <d3d12.h>
#include <string>
#include "BlendMode/BlendModeManager.h"
class DirectXCommon;
class SpriteCreatePSO {

	DirectXCommon* dxCommon_;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_[6];

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob_;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob_;

	HRESULT hr_;

	std::wstring pixelShaderPath_;

	BlendMode blendMode_ = BlendMode::kBlendModeAlpha;
	BlendModeManager blendModeManager_;

	void CreateRootSignature();
	void CreateGraphicsPipeline();

public:
	explicit SpriteCreatePSO(DirectXCommon* dxCommom);
	void Create(
	    const std::wstring& pixelShaderPath = L"Resources/shader/Sprite/Sprite.PS.hlsl");
	Microsoft::WRL::ComPtr<ID3D12RootSignature> GetRootSignature() { return rootSignature_; };
	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetGraphicsPipelineState(BlendMode blendMode) { return graphicsPipelineState_[blendMode]; };
};
