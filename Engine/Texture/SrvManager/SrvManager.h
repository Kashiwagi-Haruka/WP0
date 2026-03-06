#pragma once
#include <Windows.h>
#include <cstdint>
#include <d3d12.h>
#include <memory>
#include <wrl.h>
class DirectXCommon;
class SrvManager {
	static std::unique_ptr<SrvManager> instance;

	DirectXCommon* directXCommon_ = nullptr;

	uint32_t descriptorSize_;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;

	uint32_t useIndex = 0;
	SrvManager(SrvManager&) = delete;
	SrvManager& operator=(SrvManager&) = delete;

public:
	SrvManager() = default;
	~SrvManager() = default;

	static SrvManager* GetInstance();
	void Finalize();

	void Initialize(DirectXCommon* dxCommon);

	uint32_t Allocate();

	static const uint32_t kMaxSRVCount_;

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

	void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT format, UINT MipLevels);
	void CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride);
	void CreateUAVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride);

	void PreDraw();

	void SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);
	bool CanAllocate() const;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() { return descriptorHeap_; };
};