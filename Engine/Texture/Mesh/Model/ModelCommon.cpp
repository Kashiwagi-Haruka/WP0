#include "ModelCommon.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "SrvManager/SrvManager.h"
#include "Object3d/Object3dCommon.h"
std::unique_ptr<ModelCommon> ModelCommon::instance = nullptr;

ModelCommon* ModelCommon::GetInstance() {
	if (instance == nullptr) {
		instance = std::make_unique<ModelCommon>();
	}
	return instance.get();
}
void ModelCommon::Initialize(DirectXCommon* dxCommon) { dxCommon_ = dxCommon; }

Microsoft::WRL::ComPtr<ID3D12Resource> ModelCommon::CreateBufferResource(size_t sizeInBytes) {
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