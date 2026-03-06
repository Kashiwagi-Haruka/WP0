#pragma once
#include <array>
#include <cstdint>
#include <d3d12.h>
#include <wrl.h>

class DirectXCommon;

class RenderTexture2D {

public:
	void Initialize(uint32_t width, uint32_t height, DXGI_FORMAT format, const std::array<float, 4>& clearColor);
	bool IsReady() const { return initialized_; }
	uint32_t GetSrvIndex() const { return srvIndex_; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetRtvHandle() const { return rtvHandle_; }
	ID3D12Resource* GetResource() const { return resource_.Get(); }

	void TransitionToRenderTarget(ID3D12GraphicsCommandList* commandList);
	void TransitionToShaderResource();
	void BeginRender();

private:
	DirectXCommon* dxCommon_ = nullptr;

	uint32_t width_ = 0;
	uint32_t height_ = 0;

	DXGI_FORMAT format_ = DXGI_FORMAT_R8G8B8A8_UNORM;
	std::array<float, 4> clearColor_ = {0.0f, 0.0f, 0.0f, 1.0f};
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_ = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_{};
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle_{};
	uint32_t srvIndex_ = 0;
	D3D12_RESOURCE_STATES currentState_ = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	bool initialized_ = false;
};