#pragma once
#include "BlendMode/BlendModeManager.h"
#include <Windows.h>
#include <memory>
#include <wrl.h>
#include "PSO/SpriteCreatePSO.h"
class DirectXCommon;

class SpriteCommon {

private:
	static std::unique_ptr<SpriteCommon> instance_;

	DirectXCommon* dxCommon_;

	BlendMode blendMode_ = BlendMode::kBlendModeAlpha;
	BlendModeManager blendModeManager_;

	std::unique_ptr<SpriteCreatePSO> pso_;
	std::unique_ptr<SpriteCreatePSO> psoFont_;

	HRESULT hr_;

public:
	static SpriteCommon* GetInstance();
	void Initialize(DirectXCommon* dxCommon);
	void Finalize();
	void DrawCommon();
	void DrawCommonFont();

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	DirectXCommon* GetDxCommon() const { return dxCommon_; };
};
