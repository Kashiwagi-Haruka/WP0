#include "SpriteCommon.h"
#include "DirectXCommon.h"
#include "Logger.h"
#include <cassert>

std::unique_ptr<SpriteCommon> SpriteCommon::instance_ = nullptr;

SpriteCommon* SpriteCommon::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = std::make_unique<SpriteCommon>();
	}
	return instance_.get();
}

void SpriteCommon::Initialize(DirectXCommon* dxCommon) {

	dxCommon_ = dxCommon;

	pso_ = std::make_unique<SpriteCreatePSO>(dxCommon_);
	pso_->Create();
	psoFont_ = std::make_unique<SpriteCreatePSO>(dxCommon_);
	psoFont_->Create(L"Resources/shader/Sprite/Font.PS.hlsl");
}
void SpriteCommon::Finalize() { instance_.reset(); }
void SpriteCommon::DrawCommon() {

	dxCommon_->GetCommandList()->SetGraphicsRootSignature(pso_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(pso_->GetGraphicsPipelineState(blendMode_).Get()); // 通常

	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void SpriteCommon::DrawCommonFont(){
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(pso_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(pso_->GetGraphicsPipelineState(blendMode_).Get()); // 通常

	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
Microsoft::WRL::ComPtr<ID3D12Resource> SpriteCommon::CreateBufferResource(size_t sizeInBytes) {
	// バッファの設定（UPLOAD用に変更）
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
