#include "CreatePSO.h"
#include "DirectXCommon.h"
#include "Logger.h"
#include <array>
#include <cassert>

CreatePSO::CreatePSO(DirectXCommon* dxCommon, bool useSkinning) : dxCommon_(dxCommon), useSkinning_(useSkinning) {}

void CreatePSO::Create(
    D3D12_CULL_MODE cullMode, bool depthEnable, D3D12_FILL_MODE fillMode, D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType, const std::wstring& pixelShaderPath, const std::wstring& vertexShaderPath) {
	isShadowPass_ = false;
	pixelShaderPath_ = pixelShaderPath;
	vertexShaderPath_ = vertexShaderPath;
	CreateRootSignature();
	CreateGraphicsPipeline(cullMode, depthEnable, fillMode, topologyType);
}
void CreatePSO::CreateShadow(D3D12_CULL_MODE cullMode, D3D12_FILL_MODE fillMode) {
	isShadowPass_ = true;
	vertexShaderPath_ = L"Resources/shader/Object3d/VS_Shader/Object3dShadowMap.VS.hlsl";
	pixelShaderPath_ = L"Resources/shader/Object3d/PS_Shader/Object3dShadowMap.PS.hlsl";
	CreateRootSignature();
	CreateGraphicsPipeline(cullMode, true, fillMode, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
}
void CreatePSO::CreateRootSignature() {
	// --- RootSignature ---
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	D3D12_ROOT_PARAMETER rootParameters[14] = {};
	D3D12_DESCRIPTOR_RANGE descriptorRange[7] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descriptorRange[1].BaseShaderRegister = 1;
	descriptorRange[1].NumDescriptors = 1;
	descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descriptorRange[2].BaseShaderRegister = 2;
	descriptorRange[2].NumDescriptors = 1;
	descriptorRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descriptorRange[3].BaseShaderRegister = 3;
	descriptorRange[3].NumDescriptors = 1;
	descriptorRange[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descriptorRange[4].BaseShaderRegister = 4;
	descriptorRange[4].NumDescriptors = 1;
	descriptorRange[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descriptorRange[5].BaseShaderRegister = 5;
	descriptorRange[5].NumDescriptors = 1;
	descriptorRange[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	if (useSkinning_) {
		descriptorRange[6].BaseShaderRegister = 0;
		descriptorRange[6].NumDescriptors = 1;
		descriptorRange[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descriptorRange[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	}

	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 1;

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRange[0];
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].Descriptor.ShaderRegister = 3;

	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[4].Descriptor.ShaderRegister = 4;

	rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[5].Descriptor.ShaderRegister = 5;

	rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[6].Descriptor.ShaderRegister = 6;

	rootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[7].Descriptor.ShaderRegister = 7;

	rootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[8].DescriptorTable.pDescriptorRanges = &descriptorRange[1];
	rootParameters[8].DescriptorTable.NumDescriptorRanges = 1;

	rootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[9].DescriptorTable.pDescriptorRanges = &descriptorRange[2];
	rootParameters[9].DescriptorTable.NumDescriptorRanges = 1;

	rootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[10].DescriptorTable.pDescriptorRanges = &descriptorRange[3];
	rootParameters[10].DescriptorTable.NumDescriptorRanges = 1;

	rootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[11].DescriptorTable.pDescriptorRanges = &descriptorRange[4];
	rootParameters[11].DescriptorTable.NumDescriptorRanges = 1;

	rootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[12].DescriptorTable.pDescriptorRanges = &descriptorRange[5];
	rootParameters[12].DescriptorTable.NumDescriptorRanges = 1;

	if (useSkinning_) {
		rootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		rootParameters[13].DescriptorTable.pDescriptorRanges = &descriptorRange[6];
		rootParameters[13].DescriptorTable.NumDescriptorRanges = 1;
	}
	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = useSkinning_ ? 14 : 13;

	D3D12_STATIC_SAMPLER_DESC staticSampler[1] = {};
	staticSampler[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSampler[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSampler[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSampler[0].ShaderRegister = 0;
	staticSampler[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	descriptionRootSignature.pStaticSamplers = staticSampler;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSampler);

	// --- RootSignature作成 ---
	signatureBlob_ = nullptr;
	errorBlob_ = nullptr;
	hr_ = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob_, &errorBlob_);
	if (FAILED(hr_)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob_->GetBufferPointer()));
		assert(false);
	}
	rootSignature_ = nullptr;
	hr_ = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob_->GetBufferPointer(), signatureBlob_->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr_));
}
void CreatePSO::CreateGraphicsPipeline(D3D12_CULL_MODE cullMode, bool depthEnable, D3D12_FILL_MODE fillMode, D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType) {

	// --- InputLayout ---
	std::array<D3D12_INPUT_ELEMENT_DESC, 5> skinningElementDescs{};
	std::array<D3D12_INPUT_ELEMENT_DESC, 3> inputElementDescs{};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	if (useSkinning_) {
		skinningElementDescs[0].SemanticName = "POSITION";
		skinningElementDescs[0].SemanticIndex = 0;
		skinningElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		skinningElementDescs[0].InputSlot = 0;
		skinningElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		skinningElementDescs[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		skinningElementDescs[0].InstanceDataStepRate = 0;

		skinningElementDescs[1].SemanticName = "TEXCOORD";
		skinningElementDescs[1].SemanticIndex = 0;
		skinningElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		skinningElementDescs[1].InputSlot = 0;
		skinningElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		skinningElementDescs[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		skinningElementDescs[1].InstanceDataStepRate = 0;

		skinningElementDescs[2].SemanticName = "NORMAL";
		skinningElementDescs[2].SemanticIndex = 0;
		skinningElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		skinningElementDescs[2].InputSlot = 0;
		skinningElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		skinningElementDescs[2].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		skinningElementDescs[2].InstanceDataStepRate = 0;

		inputLayoutDesc.pInputElementDescs = skinningElementDescs.data();
		inputLayoutDesc.NumElements = 3;
	} else {
		inputElementDescs[0].SemanticName = "POSITION";
		inputElementDescs[0].SemanticIndex = 0;
		inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		inputElementDescs[0].InputSlot = 0;
		inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElementDescs[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		inputElementDescs[0].InstanceDataStepRate = 0;

		inputElementDescs[1].SemanticName = "TEXCOORD";
		inputElementDescs[1].SemanticIndex = 0;
		inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		inputElementDescs[1].InputSlot = 0;
		inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElementDescs[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		inputElementDescs[1].InstanceDataStepRate = 0;

		inputElementDescs[2].SemanticName = "NORMAL";
		inputElementDescs[2].SemanticIndex = 0;
		inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

		inputLayoutDesc.pInputElementDescs = inputElementDescs.data();
		inputLayoutDesc.NumElements = static_cast<UINT>(inputElementDescs.size());
	}

	// --- DepthStencil ---
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = depthEnable;
	if (depthEnable) {
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL; // ★ 深度書き込みを有効
	} else {
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // ★ 深度書き込みを無効
	}
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // ★ 手前なら描画

// --- 共通設定 ---
	D3D12_GRAPHICS_PIPELINE_STATE_DESC baseDesc{};
	baseDesc.pRootSignature = rootSignature_.Get();
	baseDesc.InputLayout = inputLayoutDesc;
	baseDesc.BlendState = blendModeManager_.SetBlendMode(blendMode_);
	baseDesc.NumRenderTargets = 1;
	if (isShadowPass_) {
		baseDesc.NumRenderTargets = 0;
	} else {
		baseDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	}
	baseDesc.DepthStencilState = depthStencilDesc;
	baseDesc.DSVFormat = isShadowPass_ ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_D24_UNORM_S8_UINT;
	baseDesc.PrimitiveTopologyType = topologyType;
	baseDesc.SampleDesc.Count = 1;
	baseDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// --- シェーダーコンパイル ---
	std::wstring vsPath =
	    vertexShaderPath_.empty() ? (useSkinning_ ? L"Resources/shader/Object3d/VS_Shader/SkinningObject3d.VS.hlsl" : L"Resources/shader/Object3d/VS_Shader/Object3d.VS.hlsl") : vertexShaderPath_;
	Microsoft::WRL::ComPtr<IDxcBlob> vsBlob = dxCommon_->CompileShader(vsPath.c_str(), L"vs_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> psBlob = dxCommon_->CompileShader(pixelShaderPath_.c_str(), L"ps_6_0");
	assert(vsBlob && psBlob);

	for (int i = 0; i < BlendMode::kCountOfBlendMode; i++) {
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = baseDesc;
		psoDesc.BlendState = blendModeManager_.SetBlendMode(static_cast<BlendMode>(i));
		psoDesc.VS = {vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()};
		psoDesc.PS = {psBlob->GetBufferPointer(), psBlob->GetBufferSize()};

		D3D12_RASTERIZER_DESC rasterizerDesc{};
		// ★ 背面カリングを有効化（裏面は描画しない）
		rasterizerDesc.CullMode = cullMode;
		rasterizerDesc.FillMode = fillMode;
		// ★ 深度クリッピングを有効化
		rasterizerDesc.DepthClipEnable = TRUE;
		// ★ 反時計回りを表面に設定
		rasterizerDesc.FrontCounterClockwise = FALSE;

		psoDesc.RasterizerState = rasterizerDesc;
		hr_ = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&graphicsPipelineState_[i]));
		assert(SUCCEEDED(hr_));
	}
}
