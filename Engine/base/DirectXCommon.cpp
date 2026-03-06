#define NOMINMAX
#include "DirectXCommon.h"
#include "Logger.h"
#include "ParticleManager.h"
#include "SrvManager/SrvManager.h"
#include "StringUtility.h"
#include <algorithm>
#include <cassert>
#include <dxcapi.h>
#include <format>
#include <thread>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")

using namespace Microsoft::WRL;

void DirectXCommon::initialize(WinApp* winApp) {
	assert(winApp);
	winApp_ = winApp;

	InitializeFixFPS();

	// デバイスの初期化
	DeviceInitialize();
	// コマンドリストの初期化
	CommandListInitialize();
	// スワップチェインの初期化
	SwapChainInitialize();
	// 深度バッファの生成
	DepthBufferCreate();
	// ディスクリプタヒープの生成
	DescriptorHeapCreate();
	// DXCコンパイラの生成
	DXCCompilerCreate();
	// RTVの初期化
	RenderTargetViewInitialize();
	// シーンカラーテクスチャの生成
	SceneColorResourceCreate();
	// シーンカラー用RTV/SRVの初期化
	SceneColorViewCreate();
	// シーンカラーをバックバッファにコピーするためのPSO作成
	SceneCopyPipelineCreate();
	SetVignetteStrength(vignetteStrength_);
	SetRandomNoiseEnabled(randomNoiseEnabled_);
	SetRandomNoiseScale(randomNoiseScale_);
	SetRandomNoiseBlendMode(randomNoiseBlendMode_);
	// DSVの初期化
	DepthStencilViewInitialize();
	// フェンスの生成
	FenceCreate();
	// ビューポートとシザー矩形の設定
	ViewportRectInitialize();
	ScissorRectInitialize();

}

#pragma region FixFPS
void DirectXCommon::InitializeFixFPS() { reference_ = std::chrono::steady_clock::now(); }
void DirectXCommon::UpdateFixFPS() {

	const std::chrono::microseconds kMinTime(uint64_t(1000000.0f / 60.0f));

	const std::chrono::microseconds kMinCheckTime(uint64_t(1000000.0f / 65.0f));

	// 　現在時刻を取得する

	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

	// 　前回時刻からの経過時間を取得する
	std::chrono::microseconds elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);

	// 1/60秒　(よりわずかに短い時間)経ってない場合
	if (elapsed < kMinCheckTime) {
		// 1/60秒になるまでスリープする
		while (std::chrono::steady_clock::now() - reference_ < kMinTime) {
			// 1マイクロスリープ
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}

	now = std::chrono::steady_clock::now();
	deltaTime_ = std::chrono::duration<float>(now - reference_).count();
	reference_ = now;
}
#pragma endregion

#pragma region DeviceInitialize
void DirectXCommon::DebugLayer() {

#ifdef _DEBUG

	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController_)))) {
		// デバッグレイヤー
		debugController_->EnableDebugLayer();
		// 更にGPU側でもチェックを行うようにする
		debugController_->SetEnableGPUBasedValidation(FALSE);
	}

#endif // DEBUG
}
void DirectXCommon::DeviceInitialize() {

	/// Debuglayer
	DebugLayer();

	// DXGIファクトリーの生成
	dxgiFactory_ = nullptr;
	// HRESULTはWindows系のエラーコードであり、
	// 関数が成功したかどうかをSUCCEEDEDマクロで判定できる

	hr_ = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));

	// 初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr_));

	// 良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter_)) != DXGI_ERROR_NOT_FOUND; ++i) {

		// アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr_ = useAdapter_->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr_)); // 取得できないのは一大事
		// ソフトウェアアダプタでなければ採用!
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {

			// 採用したアダプタの情報をログに出力。wstringの方なので注意
			Logger::Log(StringUtility::ConvertString_(std::format(L"Use Adapater: {}\n", adapterDesc.Description)));
			break;
		}
		useAdapter_ = nullptr; // ソフトウェアアダプタの場合は見なかった。
	}
	// 適切なアダプタが見つからなかったので起動できない。
	assert(useAdapter_ != nullptr);
	// 機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {

	    D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0};
	const char* featureLevelStrings[] = {"12.2", "12.1", "12.0"};
	// 高い順に生成できるか試していく
	for (size_t i = 0; i < _countof(featureLevels); ++i) {

		// 採用したアダプターでデバイスを生成
		hr_ = D3D12CreateDevice(useAdapter_.Get(), featureLevels[i], IID_PPV_ARGS(&device_));

		// 指定した機能レベルでデバイスが生成できたかを確認
		if (SUCCEEDED(hr_)) {
			// 生成できたのでログ出力を行ってループを抜ける
			Logger::Log(std::format("FeatureLevel: {}\n", featureLevelStrings[i]));
			break;
		}
	}
	// デバイスの生成がうまくいかなかったので起動できない
	assert(device_ != nullptr);
	Logger::Log("Complete create D3D12Device!!!\n"); // 初期化完了のログをだす
	DebugError();
}
void DirectXCommon::DebugError() {

#ifdef _DEBUG

	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		// ヤバいエラーの時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		// エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		// 警告時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		// 抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {// Windows11でのDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用バグによるエラーメッセージ
		                              // https://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-window-11
		                              D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE};
		// 抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = {D3D12_MESSAGE_SEVERITY_INFO};
		D3D12_INFO_QUEUE_FILTER filter{};

		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;

		// 指定したメッセージの表示を抑制させる
		infoQueue->PushStorageFilter(&filter);

		// 解放
		infoQueue.Reset();
	}

#endif // _DEBUG
}
void DirectXCommon::CommandListInitialize() {

	// コマンドキューを生成する
	commandQueue_ = nullptr;
	commandQueueDesc_ = {};
	commandQueueDesc_.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc_.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // 主に描画用途
	hr_ = device_->CreateCommandQueue(&commandQueueDesc_, IID_PPV_ARGS(&commandQueue_));

	// コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr_));

	// 初期化時 (CreateDevice後)
	for (UINT i = 0; i < kFrameCount; i++) {
		hr_ = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators_[i]));
		assert(SUCCEEDED(hr_));
	}

	// commandAllocators_[0] を使って最初に作る
	hr_ = device_->CreateCommandList(
	    0, D3D12_COMMAND_LIST_TYPE_DIRECT,
	    commandAllocators_[0].Get(), // ★最初の1つを渡す
	    nullptr, IID_PPV_ARGS(&commandList_));
	assert(SUCCEEDED(hr_));
}
void DirectXCommon::SwapChainInitialize() {

	swapChain_ = nullptr;
	swapChainDesc_ = {};

	swapChainDesc_.Width = WinApp::kClientWidth;
	swapChainDesc_.Height = WinApp::kClientHeight;
	swapChainDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc_.SampleDesc.Count = 1;
	swapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc_.BufferCount = 2;
	swapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// コマンドキュー、ウィンドウハンドル、設定を渡して生成する

	hr_ = dxgiFactory_->CreateSwapChainForHwnd(commandQueue_.Get(), winApp_->GetHwnd(), &swapChainDesc_, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));
	assert(SUCCEEDED(hr_));

	backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();
}
void DirectXCommon::DepthBufferCreate() {
	// ================================
	// 1. 深度バッファリソースの設定
	// ================================
	D3D12_RESOURCE_DESC depthDesc{};
	depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthDesc.Width = WinApp::kClientWidth;
	depthDesc.Height = WinApp::kClientHeight;
	depthDesc.DepthOrArraySize = 1;
	depthDesc.MipLevels = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 深度24bit + ステンシル8bit
	depthDesc.SampleDesc.Count = 1;
	depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// ================================
	// 2. 利用するヒープの設定
	// ================================
	D3D12_HEAP_PROPERTIES heapProps{};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	// ================================
	// 3. 深度値のクリア設定
	// ================================
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	clearValue.DepthStencil.Depth = 1.0f; // デフォルトは最大深度
	clearValue.DepthStencil.Stencil = 0;  // ステンシルは0クリア

	// ================================
	// 4. 深度バッファリソースの生成
	// ================================
	hr_ = device_->CreateCommittedResource(
	    &heapProps, D3D12_HEAP_FLAG_NONE, &depthDesc,
	    D3D12_RESOURCE_STATE_DEPTH_WRITE, // 初期状態
	    &clearValue, IID_PPV_ARGS(&depthStenicilResource_));
	assert(SUCCEEDED(hr_));
}
void DirectXCommon::DescriptorHeapCreate() {

	descriptorSizeRTV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	descriptorSizeDSV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	// ディスクリプタヒープの生成

		rtvDescriptorHeap_ = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 3, false);
	sceneSrvDescriptorHeap_ = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, true);

	dsvDescriptorHeap_ = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
}
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DirectXCommon::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr_ = device_->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr_));

	return descriptorHeap;
}
void DirectXCommon::RenderTargetViewInitialize() {

	hr_ = swapChain_->GetBuffer(0, IID_PPV_ARGS(&swapChainResources_[0]));
	// 上手く取得できなければ起動できない
	assert(SUCCEEDED(hr_));
	hr_ = swapChain_->GetBuffer(1, IID_PPV_ARGS(&swapChainResources_[1]));
	assert(SUCCEEDED(hr_));
	assert(backBufferIndex_ < 2);
	//
	assert(swapChainResources_[0] != nullptr);
	assert(swapChainResources_[1] != nullptr);
	assert(swapChainResources_[backBufferIndex_] != nullptr);
	//

	rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;      // 出力結果をSRGBに変換して書き込む
	rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; // 2dテクスチャとして書き込む
	// ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();

	// まず1つ目を作る。1つ目は最初のところに作る。作る場所をこちらで指定してあげる必要がある
	rtvHandles_[0] = rtvStartHandle;
	device_->CreateRenderTargetView(swapChainResources_[0].Get(), &rtvDesc_, rtvHandles_[0]);
	// 2つ目のディスクリプタハンドルを得る(自力で)
	rtvHandles_[1].ptr = rtvHandles_[0].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	// 2つ目を作る
	device_->CreateRenderTargetView(swapChainResources_[1].Get(), &rtvDesc_, rtvHandles_[1]);
}
void DirectXCommon::SceneColorResourceCreate() {
	D3D12_RESOURCE_DESC textureDesc{};
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureDesc.Width = WinApp::kClientWidth;
	textureDesc.Height = WinApp::kClientHeight;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.MipLevels = 1;
	// sRGB で描画する各 PSO と整合するように typeless で確保し、
	// RTV/SRV で用途に応じたフォーマットを指定できるようにする
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_TYPELESS;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_HEAP_PROPERTIES heapProps{};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE clearValue{};
	// typeless リソースのため、最適化クリアは実際に使う RTV フォーマットを指定する
	clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	clearValue.Color[0] = 0.1f;
	clearValue.Color[1] = 0.25f;
	clearValue.Color[2] = 0.5f;
	clearValue.Color[3] = 1.0f;

	hr_ = device_->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, IID_PPV_ARGS(&sceneColorResource_));
	assert(SUCCEEDED(hr_));
}
void DirectXCommon::SceneColorViewCreate() {
	// RTV
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStart = rtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	sceneRtvHandle_.ptr = rtvStart.ptr + descriptorSizeRTV_ * 2;
	D3D12_RENDER_TARGET_VIEW_DESC sceneRtvDesc{};
	sceneRtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	sceneRtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	device_->CreateRenderTargetView(sceneColorResource_.Get(), &sceneRtvDesc, sceneRtvHandle_);

	// SRV
	sceneSrvHandleCPU_ = sceneSrvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	sceneSrvHandleGPU_ = sceneSrvDescriptorHeap_->GetGPUDescriptorHandleForHeapStart();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	// SceneColor は sRGB で書かれているため、サンプリング時に線形化する
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	device_->CreateShaderResourceView(sceneColorResource_.Get(), &srvDesc, sceneSrvHandleCPU_);
}
void DirectXCommon::SceneCopyPipelineCreate() {
	D3D12_DESCRIPTOR_RANGE range{};
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range.NumDescriptors = 1;
	range.BaseShaderRegister = 0;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[2]{};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[0].DescriptorTable.pDescriptorRanges = &range;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[1].Descriptor.ShaderRegister = 0;
	rootParameters[1].Descriptor.RegisterSpace = 0;

	D3D12_STATIC_SAMPLER_DESC staticSampler{};
	staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSampler.ShaderRegister = 0;
	staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	staticSampler.MaxLOD = D3D12_FLOAT32_MAX;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.NumParameters = _countof(rootParameters);
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.NumStaticSamplers = 1;
	rootSignatureDesc.pStaticSamplers = &staticSampler;

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	hr_ = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr_)) {
		if (errorBlob) {
			Logger::Log(reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
		}
		assert(false);
	}

	hr_ = device_->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&copyRootSignature_));
	assert(SUCCEEDED(hr_));

	// FullScreenCopy pass は SV_VertexID を使った全画面三角形で描画する
	auto vsBlob = CompileShader(L"Resources/shader/FullScreenCopy/CopyImage.VS.hlsl", L"vs_6_0");
	auto psBlob = CompileShader(L"Resources/shader/FullScreenCopy/CopyImage.PS.hlsl", L"ps_6_0");

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.pRootSignature = copyRootSignature_.Get();
	psoDesc.VS = {vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()};
	psoDesc.PS = {psBlob->GetBufferPointer(), psBlob->GetBufferSize()};
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	psoDesc.BlendState = blendDesc;

	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	psoDesc.RasterizerState = rasterizerDesc;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	psoDesc.DepthStencilState = depthStencilDesc;
	psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.InputLayout.pInputElementDescs = nullptr;
	psoDesc.InputLayout.NumElements = 0;

	hr_ = device_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&copyPipelineState_));
	assert(SUCCEEDED(hr_));
	postEffectParameterResource_ = CreateBufferResource(sizeof(PostEffectParameters));
	hr_ = postEffectParameterResource_->Map(0, nullptr, reinterpret_cast<void**>(&postEffectParameterMappedData_));
	assert(SUCCEEDED(hr_));
	postEffectParameterMappedData_->vignetteStrength = vignetteStrength_;
	postEffectParameterMappedData_->randomNoiseEnabled = randomNoiseEnabled_ ? 1.0f : 0.0f;
	postEffectParameterMappedData_->randomNoiseScale = randomNoiseScale_;
	postEffectParameterMappedData_->randomNoiseTime = randomNoiseTime_;
	postEffectParameterMappedData_->randomNoiseBlendMode = static_cast<float>(randomNoiseBlendMode_);
}
void DirectXCommon::DepthStencilViewInitialize() {

	// depthStenicilResource_ は DepthBufferCreate() で作成済みのはず
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	device_->CreateDepthStencilView(depthStenicilResource_.Get(), &dsvDesc, dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart());
}
void DirectXCommon::DXCCompilerCreate() {

	// dxcCompilerを初期化
	dxcUtils_ = nullptr;
	dxcCompiler_ = nullptr;
	hr_ = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	assert(SUCCEEDED(hr_));
	hr_ = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	assert(SUCCEEDED(hr_));

	// 現時点でincludeはしないが、includeに対応するための設定を行っておく
	includeHandler_ = nullptr;
	hr_ = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr_));
}
void DirectXCommon::FenceCreate() {
	// Fenceを作る
	fenceValue_ = 0;
	hr_ = device_->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hr_));

	fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	assert(fenceEvent_ != nullptr);

	// ここ重要：コマンドキューが必ず有効か確認
	assert(commandQueue_ != nullptr);

	fenceValue_++;
	hr_ = commandQueue_->Signal(fence_.Get(), fenceValue_);
	assert(SUCCEEDED(hr_));
}
void DirectXCommon::ViewportRectInitialize() {

	// ビューポートとシザー設定
	viewport_ = {};
	viewport_.Width = float(WinApp::kClientWidth);
	viewport_.Height = float(WinApp::kClientHeight);
	viewport_.TopLeftX = 0;
	viewport_.TopLeftY = 0;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;
}
void DirectXCommon::ScissorRectInitialize() {

	scissorRect_ = {};
	scissorRect_.left = 0;
	scissorRect_.right = WinApp::kClientWidth;
	scissorRect_.top = 0;
	scissorRect_.bottom = WinApp::kClientHeight;
}
#pragma endregion
void DirectXCommon::SetVignetteStrength(float strength) {
	vignetteStrength_ = std::clamp(strength, 0.0f, 1.0f);
	if (postEffectParameterMappedData_) {
		postEffectParameterMappedData_->vignetteStrength = vignetteStrength_;
	}
}

void DirectXCommon::SetRandomNoiseEnabled(bool enabled) {
	randomNoiseEnabled_ = enabled;
	if (postEffectParameterMappedData_) {
		postEffectParameterMappedData_->randomNoiseEnabled = randomNoiseEnabled_ ? 1.0f : 0.0f;
	}
}

void DirectXCommon::SetRandomNoiseScale(float scale) {
	randomNoiseScale_ = std::max(scale, 1.0f);
	if (postEffectParameterMappedData_) {
		postEffectParameterMappedData_->randomNoiseScale = randomNoiseScale_;
	}
}
void DirectXCommon::SetRandomNoiseBlendMode(int blendMode) {
	randomNoiseBlendMode_ = std::clamp(blendMode, 0, 4);
	if (postEffectParameterMappedData_) {
		postEffectParameterMappedData_->randomNoiseBlendMode = static_cast<float>(randomNoiseBlendMode_);
	}
}
void DirectXCommon::PreDraw() {
	randomNoiseTime_ += deltaTime_;
	if (postEffectParameterMappedData_) {
		postEffectParameterMappedData_->randomNoiseTime = randomNoiseTime_;
	}
	// ① 現在のバックバッファをフレーム毎に更新
	backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();

	// --- 安全チェック ---
	assert(backBufferIndex_ < 2);
	assert(swapChainResources_[backBufferIndex_] != nullptr); // 安全強化！

	// ③ コマンドリストのリセット
	FrameStart();

	// ④ バックバッファへのバリア & RTV 設定 & クリア
	DrawCommandList();
}
void DirectXCommon::PostDraw() {
	// Present前にバックバッファをRTV状態からPresent状態へ戻す
	D3D12_RESOURCE_BARRIER presentBarrier{};
	presentBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	presentBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	presentBarrier.Transition.pResource = swapChainResources_[backBufferIndex_].Get();
	presentBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	presentBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	presentBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commandList_->ResourceBarrier(1, &presentBarrier);

	// コマンドリストをクローズして実行
	hr_ = commandList_->Close();
	assert(SUCCEEDED(hr_));
	Microsoft::WRL::ComPtr<ID3D12CommandList> lists[] = {commandList_.Get()};
	commandQueue_->ExecuteCommandLists(1, lists->GetAddressOf());

	// 画面を切り替え
	swapChain_->Present(1, 0);

	// フェンスで CPU/GPU 同期
	fenceValue_++;
	commandQueue_->Signal(fence_.Get(), fenceValue_);
	if (fence_->GetCompletedValue() < fenceValue_) {
		fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		WaitForSingleObject(fenceEvent_, INFINITE);
	}
	UpdateFixFPS();
	hr_ = commandAllocators_[frameIndex_]->Reset();
	assert(SUCCEEDED(hr_));
	hr_ = commandList_->Reset(commandAllocators_[frameIndex_].Get(), nullptr);
	assert(SUCCEEDED(hr_));
}
void DirectXCommon::ExecuteCommandListAndWait() {
	hr_ = commandList_->Close();
	assert(SUCCEEDED(hr_));
	Microsoft::WRL::ComPtr<ID3D12CommandList> lists[] = {commandList_.Get()};
	commandQueue_->ExecuteCommandLists(1, lists->GetAddressOf());

	fenceValue_++;
	commandQueue_->Signal(fence_.Get(), fenceValue_);
	if (fence_->GetCompletedValue() < fenceValue_) {
		fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		WaitForSingleObject(fenceEvent_, INFINITE);
	}

	hr_ = commandAllocators_[frameIndex_]->Reset();
	assert(SUCCEEDED(hr_));
	hr_ = commandList_->Reset(commandAllocators_[frameIndex_].Get(), nullptr);
	assert(SUCCEEDED(hr_));
}
void DirectXCommon::DrawSceneTextureToBackBuffer() {
	D3D12_RESOURCE_BARRIER barriers[2]{};
	barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barriers[0].Transition.pResource = sceneColorResource_.Get();
	barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commandList_->ResourceBarrier(1, barriers);

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	commandList_->OMSetRenderTargets(1, &rtvHandles_[backBufferIndex_], false, &dsvHandle);
	float backBufferClearColor[] = {0.06f, 0.06f, 0.08f, 1.0f};
	commandList_->ClearRenderTargetView(rtvHandles_[backBufferIndex_], backBufferClearColor, 0, nullptr);

	D3D12_VIEWPORT gameViewport = viewport_;
	D3D12_RECT gameScissor = scissorRect_;
	if (editorLayoutEnabled_) {
		const float kTopToolbarHeight = 44.0f;
		const float kLeftPanelRatio = 0.22f;
		const float kRightPanelRatio = 0.24f;
		const float kGameAspect = 16.0f / 9.0f;
		const float availableWidth = viewport_.Width * (1.0f - kLeftPanelRatio - kRightPanelRatio);
		const float availableHeight = std::max(1.0f, viewport_.Height - kTopToolbarHeight);
		const float availableStartX = viewport_.Width * kLeftPanelRatio;
		const float availableStartY = kTopToolbarHeight;

		float gameWidth = availableWidth;
		float gameHeight = gameWidth / kGameAspect;
		if (gameHeight > availableHeight) {
			gameHeight = availableHeight;
			gameWidth = gameHeight * kGameAspect;
		}

		gameViewport.Width = std::max(1.0f, gameWidth);
		gameViewport.Height = std::max(1.0f, gameHeight);
		gameViewport.TopLeftX = availableStartX + (availableWidth - gameViewport.Width) * 0.5f;
		gameViewport.TopLeftY = availableStartY + (availableHeight - gameViewport.Height) * 0.5f;
		gameScissor.left = static_cast<LONG>(gameViewport.TopLeftX);
		gameScissor.top = static_cast<LONG>(gameViewport.TopLeftY);
		gameScissor.right = gameScissor.left + static_cast<LONG>(gameViewport.Width);
		gameScissor.bottom = gameScissor.top + static_cast<LONG>(gameViewport.Height);
	}
	commandList_->RSSetViewports(1, &gameViewport);
	commandList_->RSSetScissorRects(1, &gameScissor);

	ID3D12DescriptorHeap* descriptorHeaps[] = {sceneSrvDescriptorHeap_.Get()};
	commandList_->SetDescriptorHeaps(1, descriptorHeaps);
	commandList_->SetGraphicsRootSignature(copyRootSignature_.Get());
	commandList_->SetPipelineState(copyPipelineState_.Get());
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList_->SetGraphicsRootDescriptorTable(0, sceneSrvHandleGPU_);
	commandList_->SetGraphicsRootConstantBufferView(1, postEffectParameterResource_->GetGPUVirtualAddress());
	commandList_->DrawInstanced(3, 1, 0, 0);
	commandList_->RSSetViewports(1, &viewport_);
	commandList_->RSSetScissorRects(1, &scissorRect_);

	barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barriers[1].Transition.pResource = sceneColorResource_.Get();
	barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commandList_->ResourceBarrier(1, &barriers[1]);
}
void DirectXCommon::FrameStart() {

	// FrameStart
	frameIndex_ = swapChain_->GetCurrentBackBufferIndex();
}
void DirectXCommon::DrawCommandList() {

	// TransitionBarrierの設定
	// 今回のバリアはTransition
	barrier_.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにしておく
	barrier_.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを張る対象のリソース。現在のバックアップに対して行う
	barrier_.Transition.pResource = swapChainResources_[backBufferIndex_].Get();
	// 遷移前(現在)のResourceState
	barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	// 遷移後のResourceState
	barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	assert(commandList_ != nullptr);

	// TransitionBarrierを張る
	commandList_->ResourceBarrier(1, &barrier_);

	// まずはSceneColor(RenderTexture)を描画先にする
	SetMainRenderTarget();

	commandList_->RSSetViewports(1, &viewport_);       // Viewportを設定
	commandList_->RSSetScissorRects(1, &scissorRect_); // Scissorを設定

	// Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = {srvDescriptorHeap_.Get()};
	// commandList_->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());
}
void DirectXCommon::SetMainRenderTarget() {
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	commandList_->OMSetRenderTargets(1, &sceneRtvHandle_, false, &dsvHandle);
	commandList_->RSSetViewports(1, &viewport_);
	commandList_->RSSetScissorRects(1, &scissorRect_);
	float clearColor[] = {0.1f, 0.25f, 0.5f, 1.0f};
	commandList_->ClearRenderTargetView(sceneRtvHandle_, clearColor, 0, nullptr);
	commandList_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void DirectXCommon::CrtvTransitionBarrier() {
	// 画面に描く処理はすべて終わり、画面に映すので、状態を遷移
	// 今回はRenderTargetからPresentにする。
	barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	// TransitionBarrierを張る
	commandList_->ResourceBarrier(1, &barrier_);
}

Microsoft::WRL::ComPtr<IDxcBlob> DirectXCommon::CompileShader(
    /* CompilerするShaderファイルへのパス*/ const std::wstring& filePath,
    // Compilerに使用するProfile
    const wchar_t* profile) {
	// ここの中身をこの後書いていく
	// 1. hlslファイルを読む
	// // これからシェーダーをコンパイルする旨をログに出す
	Logger::Log(StringUtility::ConvertString_(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));

	// hlslファイルを読む
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource = nullptr;
	HRESULT hr_ = dxcUtils_->LoadFile(filePath.c_str(), nullptr, &shaderSource);

	// 読めなかったら止める
	assert(SUCCEEDED(hr_));

	// 読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8; // UTF8の文字コードであることを通知
	// 2. Compileする
	LPCWSTR arguments[] = {
	    filePath.c_str(), // コンパイル対象のhlslファイル名
	    L"-E",
	    L"main", // エントリーポイントの指定。基本的にmain以外は指定しない
	    L"-T",
	    profile, // ShaderProfileの設定
	    L"-Zi",
	    L"-Qembed_debug", // デバッグ用の情報を埋め込む
	    L"-Od",           // 最適化を外しておく
	    L"-Zpr",          // メモリレイアウトは行優先
	};

	// 実際にShaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcResult> shaderResult = nullptr;
	hr_ = dxcCompiler_->Compile(&shaderSourceBuffer, arguments, _countof(arguments), includeHandler_.Get(), IID_PPV_ARGS(&shaderResult));

	// コンパイルエラーではなくdxcが起動できないなど致命的な状況
	assert(SUCCEEDED(hr_));
	// 3. 警告・エラーがでていないか確認する
	// // 警告・エラーが出たらログに出して止める
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Logger::Log(shaderError->GetStringPointer());
		// 警告・エラーダメゼッタイ
		assert(false);
	}
	// 4. Compile結果を受け取って返す
	// コンパイル結果から実行用のバイナリ部分を取得
	Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob = nullptr;
	hr_ = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr_));

	// 成功したログを出す
	Logger::Log(StringUtility::ConvertString_(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));

	// もう使わないリソースを解放
	shaderSource.Reset();
	shaderResult.Reset();

	// 実行用のバイナリを返却
	return shaderBlob;
}
Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateBufferResource(size_t sizeInBytes) {
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

	HRESULT hr_ = device_->CreateCommittedResource(
	    &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
	    D3D12_RESOURCE_STATE_GENERIC_READ, // Uploadならこれ
	    nullptr, IID_PPV_ARGS(&bufferResource));

	if (FAILED(hr_)) {
		return nullptr;
	}

	return bufferResource;
}

void DirectXCommon::Finalize() {

	// --- GPU 完了待ち ---
	if (commandQueue_ && fence_ && fenceEvent_) {
		commandQueue_->Signal(fence_.Get(), fenceValue_);
		if (fence_->GetCompletedValue() < fenceValue_) {
			fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
			WaitForSingleObject(fenceEvent_, INFINITE);
		}
		fenceValue_++;
	}

	// --- Fence Event ---
	if (fenceEvent_) {
		CloseHandle(fenceEvent_);
		fenceEvent_ = nullptr;
	}

	// --- PostEffect ---
	postEffectParameterMappedData_ = nullptr;
	postEffectParameterResource_.Reset();

	// --- Scene color / copy pipeline ---
	sceneColorResource_.Reset();
	sceneSrvDescriptorHeap_.Reset();
	copyRootSignature_.Reset();
	copyPipelineState_.Reset();

	// --- SwapChain Resources ---
	for (auto& bb : swapChainResources_) {
		bb.Reset();
	}

	// --- DepthStencil ---
	depthStenicilResource_.Reset();

	// --- Descriptor Heaps ---
	rtvDescriptorHeap_.Reset();
	srvDescriptorHeap_.Reset();
	dsvDescriptorHeap_.Reset();

	// --- DXC ---
	includeHandler_.Reset();
	dxcCompiler_.Reset();
	dxcUtils_.Reset();

	// --- Command ---
	commandList_.Reset();
	for (auto& alloc : commandAllocators_) {
		alloc.Reset();
	}
	commandQueue_.Reset();

	// --- Synchronization ---
	fence_.Reset();

	// --- Swap chain ---
	swapChain_.Reset();

	// --- Device ---
	device_.Reset();
	debugController_.Reset();
	dxgiFactory_.Reset();
	useAdapter_.Reset();
}
