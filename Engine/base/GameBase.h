#pragma once
#include "Audio.h"
#include "BlendMode/BlendModeManager.h"
#include "Camera.h"
#include "DirectXCommon.h"
#include "Function.h"

#include "TextureManager.h"
#include "Vector4.h"
#include "VertexData.h"
#include "WinApp.h"
#include <memory>
class SrvManager;
class ImGuiManager;

class GameBase {

private:
	static std::unique_ptr<GameBase> instance;

	std::unique_ptr<WinApp> winApp_ = nullptr;

	std::unique_ptr<DirectXCommon> dxCommon_ = nullptr;

	std::unique_ptr<ImGuiManager> imguiM_ = nullptr;

	HRESULT hr_;


public:
	static GameBase* GetInstance();

	void Finalize();

	void Initialize(const wchar_t* TitleName, int32_t WindowWidth, int32_t WindowHeight);

	void BeginFlame(); // フレームの開始処理(commandListリセットなど)
	void EndFlame();   // フレームの終了処理(Present、フェンス待ちなど)

	bool ProcessMessage();

	float GetDeltaTime();
	ID3D12Device* GetD3D12Device();
	static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception);


};