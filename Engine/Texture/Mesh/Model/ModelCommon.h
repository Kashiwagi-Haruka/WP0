#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <wrl.h>
#include <memory>
class DirectXCommon;

class ModelCommon {

	static std::unique_ptr<ModelCommon> instance;
	
	DirectXCommon* dxCommon_;

public:
	static ModelCommon* GetInstance();
	void Initialize(DirectXCommon* dxCommon);
	DirectXCommon* GetDxCommon() const { return dxCommon_; };
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);
};
