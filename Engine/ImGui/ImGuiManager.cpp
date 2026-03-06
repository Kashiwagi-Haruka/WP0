#define NOMINMAX
#include "ImGuiManager.h"
#include "Engine/Editor/Hierarchy.h"
#include <dxgi1_6.h>
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#endif
#include "DirectXCommon.h"
#include "SrvManager/SrvManager.h"
#include "WinApp.h"
#include <algorithm>
#include <cfloat>
#include <format>
#include <string>

void ImGuiManager::Initialize([[maybe_unused]] WinApp* winApp, [[maybe_unused]] DirectXCommon* dxCommon) {
#ifdef USE_IMGUI
	auto* srvManager = SrvManager::GetInstance();
	dxCommon_ = dxCommon;
	winApp_ = winApp;
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	// Win32 backend（先に）
	ImGui_ImplWin32_Init(winApp->GetHwnd());

	// ---- DX12 backend（新API）----
	ImGui_ImplDX12_InitInfo init_info = {};
	init_info.Device = dxCommon->GetDevice();
	init_info.CommandQueue = dxCommon->GetCommandQueue().Get();
	init_info.NumFramesInFlight = static_cast<int>(dxCommon->GetSwapChainResourcesNum());
	init_info.RTVFormat = dxCommon->GetRtvDesc().Format; // 固定値より安全
	init_info.SrvDescriptorHeap = srvManager->GetDescriptorHeap().Get();

	init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) {
		auto* sm = reinterpret_cast<SrvManager*>(info->UserData);
		uint32_t index = sm->Allocate();
		*out_cpu_handle = sm->GetCPUDescriptorHandle(index);
		*out_gpu_handle = sm->GetGPUDescriptorHandle(index);
	};

	init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE) {
		// 今回はフリーリスト無しなので何もしない
	};

	init_info.UserData = srvManager;

	// ★これを忘れずに！
	ImGui_ImplDX12_Init(&init_info);

	// 任意のフラグ（順序はどちらでもOK）
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
#endif
}

void ImGuiManager::Begin() {
#ifdef USE_IMGUI
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();


	ImGuiIO& io = ImGui::GetIO();

	if (winApp_) {
		RECT clientRect{};
		if (GetClientRect(winApp_->GetHwnd(), &clientRect)) {
			const float clientWidth = static_cast<float>(std::max(1L, clientRect.right - clientRect.left));
			const float clientHeight = static_cast<float>(std::max(1L, clientRect.bottom - clientRect.top));
			const float renderWidth = static_cast<float>(WinApp::kClientWidth);
			const float renderHeight = static_cast<float>(WinApp::kClientHeight);

			POINT cursorPoint{};
			if (GetCursorPos(&cursorPoint) && ScreenToClient(winApp_->GetHwnd(), &cursorPoint) != 0 && cursorPoint.x >= 0 && cursorPoint.y >= 0 &&
			    cursorPoint.x < (clientRect.right - clientRect.left) && cursorPoint.y < (clientRect.bottom - clientRect.top)) {
				const float mappedMouseX = static_cast<float>(cursorPoint.x) * (renderWidth / clientWidth);
				const float mappedMouseY = static_cast<float>(cursorPoint.y) * (renderHeight / clientHeight);
				io.AddMousePosEvent(mappedMouseX, mappedMouseY);
			} else {
				io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
			}

			io.DisplaySize = ImVec2(renderWidth, renderHeight);
			io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
		}
	}
	ImGui::NewFrame();
	if (ImGui::Begin("Performance")) {
		ImGui::Text("FPS: %.1f", io.Framerate);
	}
	ImGui::End();
#endif
	Hierarchy* Hierarchy = Hierarchy::GetInstance();
#ifdef USE_IMGUI
	const bool isEditorLayoutEnabled = Hierarchy->HasRegisteredObjects();
	if (dxCommon_) {
		dxCommon_->SetEditorLayoutEnabled(isEditorLayoutEnabled);
	}
	prevEditorLayoutEnabled_ = isEditorLayoutEnabled;
#endif
	Hierarchy->DrawObjectEditors();
}

void ImGuiManager::End() {
#ifdef USE_IMGUI
	ImGui::Render();
#endif
}

void ImGuiManager::Draw([[maybe_unused]] DirectXCommon* dxCommon) {
#ifdef USE_IMGUI
	auto* srvManager = SrvManager::GetInstance();
	if (!srvManager->GetDescriptorHeap().Get() || !dxCommon->GetCommandList()) {
		OutputDebugStringA("ImGui Render Error: srvDescriptorHeap_ or commandList_ is null\n");
		return;
	}
	// 描画用のDescriptorHeapの設定
	ID3D12DescriptorHeap* descriptorHeaps[] = {srvManager->GetDescriptorHeap().Get()};
	dxCommon->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

#endif
}

void ImGuiManager::Finalize() {
#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

#endif
}