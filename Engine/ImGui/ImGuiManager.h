#pragma once

#include <cstdint>
#include <d3d12.h>
#include <dxgi1_6.h>

class WinApp;
class DirectXCommon;
class SrvManager;
class ImGuiManager {

	uint32_t srvIndex_;
	DirectXCommon* dxCommon_ = nullptr;
	WinApp* winApp_ = nullptr;
	bool prevEditorLayoutEnabled_ = false;

public:
	void Initialize(WinApp* winapp, DirectXCommon* dxCommon);
	void Begin();
	void End();
	void Draw(DirectXCommon* dxCommon);

	void Finalize();
};