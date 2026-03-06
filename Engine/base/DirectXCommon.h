#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

// DXC (DirectX Shader Compiler)
#include <dxcapi.h>

// SAL マクロ用
#include <sal.h>

#include <array>
#include <chrono>
#include <string>
#include <vector>

#include "WinApp.h"

#include "BlendMode/BlendModeManager.h"
#include "Function.h"
#include "Light/DirectionalLight.h"
#include "Matrix4x4.h"
#include "Transform.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "VertexData.h"

class BlendModeManager;
class SrvManager;

class DirectXCommon {

	HRESULT hr_;
	Microsoft::WRL::ComPtr<ID3D12Device> device_ = nullptr;
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc_;

	static const UINT kFrameCount = 2; // スワップチェインのバッファ数に合わせる
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocators_[kFrameCount];
	UINT frameIndex_ = 0;

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_ = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_;
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController_ = nullptr;
	// これから書き込むバックバッファのインデックスを取得
	UINT backBufferIndex_;
	uint32_t descriptorSizeSRV_ = 0;
	uint32_t descriptorSizeRTV_ = 0;
	uint32_t descriptorSizeDSV_ = 0;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_ = nullptr;
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources_;
	Microsoft::WRL::ComPtr<ID3D12Resource> sceneColorResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> sceneSrvDescriptorHeap_ = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE sceneRtvHandle_{};
	D3D12_CPU_DESCRIPTOR_HANDLE sceneSrvHandleCPU_{};
	D3D12_GPU_DESCRIPTOR_HANDLE sceneSrvHandleGPU_{};

	Microsoft::WRL::ComPtr<ID3D12RootSignature> copyRootSignature_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> copyPipelineState_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> postEffectParameterResource_ = nullptr;
	struct PostEffectParameters {
		float vignetteStrength;
		float randomNoiseEnabled;
		float randomNoiseScale;
		float randomNoiseTime;
		float randomNoiseBlendMode;
		float padding[3];
	};
	PostEffectParameters* postEffectParameterMappedData_ = nullptr;
	float vignetteStrength_ = 0.0f;
	bool randomNoiseEnabled_ = false;
	float randomNoiseScale_ = 512.0f;
	float randomNoiseTime_ = 0.0f;
	int randomNoiseBlendMode_ = 0;
	bool editorLayoutEnabled_ = false;
	// RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};
	// RTVを2つ作るのでディスクリプタを2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];

	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;

	Microsoft::WRL::ComPtr<ID3D12Resource> depthStenicilResource_ = nullptr;

	// 初期値0でFenceをつくる
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_ = nullptr;
	uint64_t fenceValue_ = 0;
	// FenceのSignalを持つためのイベントを作成する
	HANDLE fenceEvent_;

	D3D12_VIEWPORT viewport_;
	// シザー矩形
	D3D12_RECT scissorRect_;

	// transitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier_{};

	float deltaTime_ = 1.0f / 60.0f;
	WinApp* winApp_ = nullptr;
	std::chrono::steady_clock::time_point reference_;

public:
	void initialize(WinApp* winApp);
	void InitializeFixFPS();

	void UpdateFixFPS();
	void PreDraw();
	void PostDraw();
	void SetMainRenderTarget();
	void ExecuteCommandListAndWait();
	void DrawSceneTextureToBackBuffer();
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);
	void Finalize();
	float GetDeltaTime() const { return deltaTime_; }
	void SetVignetteStrength(float strength);
	float GetVignetteStrength() const { return vignetteStrength_; }
	void SetRandomNoiseEnabled(bool enabled);
	bool GetRandomNoiseEnabled() const { return randomNoiseEnabled_; }
	void SetRandomNoiseScale(float scale);
	float GetRandomNoiseScale() const { return randomNoiseScale_; }
	void SetRandomNoiseBlendMode(int blendMode);
	int GetRandomNoiseBlendMode() const { return randomNoiseBlendMode_; }
	void SetEditorLayoutEnabled(bool enabled) { editorLayoutEnabled_ = enabled; }

	ID3D12Device* GetDevice() { return device_.Get(); };
	ID3D12GraphicsCommandList* GetCommandList() { return commandList_.Get(); };
	D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilViewHandle() const { return dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart(); }

	D3D12_RENDER_TARGET_VIEW_DESC GetRtvDesc() { return rtvDesc_; }

	size_t GetSwapChainResourcesNum() const { return swapChainResources_.size(); }
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() { return commandQueue_; }

	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader( // CompilerするShaderファイルへのパス
	    const std::wstring& filePath,
	    // Compilerに使用するProfile
	    const wchar_t* profile);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

private:
	void DebugLayer();
	void DeviceInitialize();
	void DebugError();
	void CommandListInitialize();
	void SwapChainInitialize();
	void DepthBufferCreate();
	void DescriptorHeapCreate();
	void SceneColorResourceCreate();
	void SceneColorViewCreate();
	void SceneCopyPipelineCreate();

	void RenderTargetViewInitialize();
	void DepthStencilViewInitialize();
	void FenceCreate();
	void ViewportRectInitialize();
	void ScissorRectInitialize();
	void DXCCompilerCreate();

	void FrameStart();      // フレーム最初の準備
	void DrawCommandList(); // 描画コマンドリスト

	void CrtvTransitionBarrier();
};
