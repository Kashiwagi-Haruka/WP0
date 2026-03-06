#include "SrvManager.h"
#include "DirectXCommon.h"
#include <cassert>
#include <memory>
std::unique_ptr<SrvManager> SrvManager::instance = nullptr;
SrvManager* SrvManager::GetInstance() {
	if (instance == nullptr) {
		instance = std::make_unique<SrvManager>();
	}
	return instance.get();
}

void SrvManager::Finalize() { instance.reset(); }
const uint32_t SrvManager::kMaxSRVCount_ = 512;
void SrvManager::Initialize(DirectXCommon* dxCommon) {
	directXCommon_ = dxCommon;

	// SRV用ディスクリプタヒープ作成
	descriptorHeap_ = directXCommon_->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount_, true);
	descriptorSize_ = directXCommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

uint32_t SrvManager::Allocate() {

	int index = useIndex;

	useIndex++;

	return index;
}

D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetCPUDescriptorHandle(uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize_ * index);
	return handleCPU;
}
D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetGPUDescriptorHandle(uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize_ * index);
	return handleGPU;
}

void SrvManager::CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT format, UINT MipLevels) {

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = MipLevels;
	directXCommon_->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, GetCPUDescriptorHandle(srvIndex));
}
void SrvManager::CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride) {
	assert(pResource);

	// SRV 設定構造体を初期化
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN; // StructuredBuffer はフォーマットを持たない
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	// StructuredBuffer 用設定
	srvDesc.Buffer.FirstElement = 0;                          // 最初の要素
	srvDesc.Buffer.NumElements = numElements;                 // 要素数
	srvDesc.Buffer.StructureByteStride = structureByteStride; // 要素1つのサイズ
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;        // 通常は None

	// SRV を作成
	directXCommon_->GetDevice()->CreateShaderResourceView(
	    pResource,                       // 対象リソース
	    &srvDesc,                        // 設定
	    GetCPUDescriptorHandle(srvIndex) // 作成先（ヒープ上の位置）
	);
}
void SrvManager::CreateUAVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride) {
	assert(pResource);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = numElements;
	uavDesc.Buffer.StructureByteStride = structureByteStride;
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	directXCommon_->GetDevice()->CreateUnorderedAccessView(pResource, nullptr, &uavDesc, GetCPUDescriptorHandle(srvIndex));
}
void SrvManager::PreDraw() {
	ID3D12DescriptorHeap* descriptorHeaps[] = {descriptorHeap_.Get()};
	directXCommon_->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
}

void SrvManager::SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex) {

	directXCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(RootParameterIndex, GetGPUDescriptorHandle(srvIndex));
}
bool SrvManager::CanAllocate() const {
	// useIndex が最大数 kMaxSRVCount_ に達していなければ true
	return (useIndex < kMaxSRVCount_);
}
