#include "SkinCluster.h"
#include "DirectXCommon.h"
#include "Function.h"
#include "Logger.h"
#include "Model/ModelCommon.h"
#include "SrvManager/SrvManager.h"
#include "TextureManager.h"
#include <algorithm>
#include <cassert>
#include <cstring>

namespace {
Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBufferResource(DirectXCommon* dxCommon, size_t sizeInBytes, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initialState) {
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = flags;

	Microsoft::WRL::ComPtr<ID3D12Resource> bufferResource = nullptr;
	HRESULT hr = dxCommon->GetDevice()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, initialState, nullptr, IID_PPV_ARGS(&bufferResource));
	assert(SUCCEEDED(hr));
	return bufferResource;
}

void CreateSkinningComputePipeline(SkinCluster& skinCluster) {
	auto* dxCommon = ModelCommon::GetInstance()->GetDxCommon();
	assert(dxCommon);

	D3D12_DESCRIPTOR_RANGE descriptorRanges[4]{};
	descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRanges[0].NumDescriptors = 1;
	descriptorRanges[0].BaseShaderRegister = 0;
	descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRanges[1].NumDescriptors = 1;
	descriptorRanges[1].BaseShaderRegister = 1;
	descriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRanges[2].NumDescriptors = 1;
	descriptorRanges[2].BaseShaderRegister = 2;
	descriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRanges[3].NumDescriptors = 1;
	descriptorRanges[3].BaseShaderRegister = 0;
	descriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[5]{};
	for (uint32_t i = 0; i < 4; ++i) {
		rootParameters[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameters[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		rootParameters[i].DescriptorTable.pDescriptorRanges = &descriptorRanges[i];
		rootParameters[i].DescriptorTable.NumDescriptorRanges = 1;
	}
	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[4].Descriptor.ShaderRegister = 0;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters = _countof(rootParameters);
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	assert(SUCCEEDED(hr));

	hr = dxCommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&skinCluster.computeRootSignature));
	assert(SUCCEEDED(hr));

		auto csBlob = dxCommon->CompileShader(L"Resources/shader/Object3d/CS_Shader/Skinning.CS.hlsl", L"cs_6_0");
	D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineDesc{};
	computePipelineDesc.pRootSignature = skinCluster.computeRootSignature.Get();
	computePipelineDesc.CS = {csBlob->GetBufferPointer(), csBlob->GetBufferSize()};

	hr = dxCommon->GetDevice()->CreateComputePipelineState(&computePipelineDesc, IID_PPV_ARGS(&skinCluster.computePipelineState));
	assert(SUCCEEDED(hr));
}
} // namespace

SkinCluster CreateSkinCluster(const Skeleton& skeleton, const Model& model) {
	SkinCluster skinCluster{};

	const auto& joints = skeleton.GetJoints();
	if (joints.empty()) {
		Logger::Log("CreateSkinCluster skipped: skeleton has no joints.\n");
		return skinCluster;
	}

	const auto& modelData = model.GetModelData();

	// palette用Resourceを確保
	skinCluster.paletteResource = ModelCommon::GetInstance()->CreateBufferResource(sizeof(WellForGPU) * joints.size());
	WellForGPU* mappedPalette = nullptr;
	skinCluster.paletteResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedPalette));
	skinCluster.mappedPalette = {mappedPalette, joints.size()};

	// palette用SRVを作成
	assert(SrvManager::GetInstance()->CanAllocate());
	skinCluster.paletteSrvIndex = SrvManager::GetInstance()->Allocate();
	skinCluster.paletteSrvHandle.first = SrvManager::GetInstance()->GetCPUDescriptorHandle(skinCluster.paletteSrvIndex);
	skinCluster.paletteSrvHandle.second = SrvManager::GetInstance()->GetGPUDescriptorHandle(skinCluster.paletteSrvIndex);
	SrvManager::GetInstance()->CreateSRVforStructuredBuffer(skinCluster.paletteSrvIndex, skinCluster.paletteResource.Get(), static_cast<UINT>(joints.size()), sizeof(WellForGPU));

	// Influence用Resourceを確保
	skinCluster.influenceResource = ModelCommon::GetInstance()->CreateBufferResource(sizeof(VertexInfluence) * modelData.vertices.size());
	VertexInfluence* mappedInfluence = nullptr;
	skinCluster.influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluence));
	std::memset(mappedInfluence, 0, sizeof(VertexInfluence) * modelData.vertices.size());
	skinCluster.mappedInfluence = {mappedInfluence, modelData.vertices.size()};

	assert(SrvManager::GetInstance()->CanAllocate());
	skinCluster.influenceSrvIndex = SrvManager::GetInstance()->Allocate();
	SrvManager::GetInstance()->CreateSRVforStructuredBuffer(skinCluster.influenceSrvIndex, skinCluster.influenceResource.Get(), static_cast<UINT>(modelData.vertices.size()), sizeof(VertexInfluence));

	// Compute shader input/output vertex
	skinCluster.inputVertexResource = model.GetVertexResource();
	assert(SrvManager::GetInstance()->CanAllocate());
	skinCluster.inputVertexSrvIndex = SrvManager::GetInstance()->Allocate();
	SrvManager::GetInstance()->CreateSRVforStructuredBuffer(skinCluster.inputVertexSrvIndex, skinCluster.inputVertexResource.Get(), static_cast<UINT>(modelData.vertices.size()), sizeof(VertexData));

		skinCluster.outputVertexResource =
	    CreateDefaultBufferResource(ModelCommon::GetInstance()->GetDxCommon(), sizeof(VertexData) * modelData.vertices.size(), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
	skinCluster.outputVertexCurrentState = D3D12_RESOURCE_STATE_COMMON;

	skinCluster.outputVertexBufferView.BufferLocation = skinCluster.outputVertexResource->GetGPUVirtualAddress();
	skinCluster.outputVertexBufferView.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * modelData.vertices.size());
	skinCluster.outputVertexBufferView.StrideInBytes = sizeof(VertexData);

	assert(SrvManager::GetInstance()->CanAllocate());
	skinCluster.outputVertexSrvIndex = SrvManager::GetInstance()->Allocate();
	SrvManager::GetInstance()->CreateSRVforStructuredBuffer(skinCluster.outputVertexSrvIndex, skinCluster.outputVertexResource.Get(), static_cast<UINT>(modelData.vertices.size()), sizeof(VertexData));

	assert(SrvManager::GetInstance()->CanAllocate());
	skinCluster.outputVertexUavIndex = SrvManager::GetInstance()->Allocate();
	SrvManager::GetInstance()->CreateUAVforStructuredBuffer(skinCluster.outputVertexUavIndex, skinCluster.outputVertexResource.Get(), static_cast<UINT>(modelData.vertices.size()), sizeof(VertexData));

	// SkinningInformation (CBV)
	const size_t skinningInformationSize = (sizeof(SkinCluster::SkinningInformation) + 0xFF) & ~0xFF;
	skinCluster.skinningInformationResource = ModelCommon::GetInstance()->CreateBufferResource(skinningInformationSize);
	SkinCluster::SkinningInformation* mappedSkinningInformation = nullptr;
	skinCluster.skinningInformationResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedSkinningInformation));
	skinCluster.mappedSkinningInformation = {mappedSkinningInformation, 1};
	skinCluster.mappedSkinningInformation[0].numVertices = static_cast<int32_t>(modelData.vertices.size());

	CreateSkinningComputePipeline(skinCluster);

	// InverseBindPoseMatrixを格納する場所を作成
	skinCluster.inverseBindPoseMatrices.resize(joints.size());
	std::generate(skinCluster.inverseBindPoseMatrices.begin(), skinCluster.inverseBindPoseMatrices.end(), Function::MakeIdentity4x4);

	// ModelDataのSkinCluster情報を解析してInfluenceの中身を埋める
	for (const auto& [jointName, jointWeight] : modelData.skinClusterData) {
		auto jointIndex = skeleton.FindJointIndex(jointName);
		if (!jointIndex) {
			continue;
		}

		skinCluster.inverseBindPoseMatrices[*jointIndex] = jointWeight.inverseBindPoseMatrix;
		for (const auto& vertexWeight : jointWeight.vertexWeights) {
			auto& currentInfluence = skinCluster.mappedInfluence[vertexWeight.vertexIndex];
			uint32_t targetIndex = kNumMaxInfluence;
			for (uint32_t index = 0; index < kNumMaxInfluence; ++index) {
				if (currentInfluence.weights[index] == 0.0f) {
					targetIndex = index;
					break;
				}
			}

			if (targetIndex == kNumMaxInfluence) {
				float minWeight = currentInfluence.weights[0];
				targetIndex = 0;
				for (uint32_t index = 1; index < kNumMaxInfluence; ++index) {
					if (currentInfluence.weights[index] < minWeight) {
						minWeight = currentInfluence.weights[index];
						targetIndex = index;
					}
				}
				if (vertexWeight.weight <= minWeight) {
					continue;
				}
			}

			currentInfluence.weights[targetIndex] = vertexWeight.weight;
			currentInfluence.jointIndices[targetIndex] = *jointIndex;
		}
	}

	for (auto& influence : skinCluster.mappedInfluence) {
		float weightSum = 0.0f;
		for (float weight : influence.weights) {
			weightSum += weight;
		}
		if (weightSum <= 0.0f) {
			continue;
		}
		for (float& weight : influence.weights) {
			weight /= weightSum;
		}
	}

	return skinCluster;
}

void UpdateSkinCluster(SkinCluster& skinCluster, const Skeleton& skeleton) {
	const auto& joints = skeleton.GetJoints();
	for (size_t jointIndex = 0; jointIndex < joints.size(); ++jointIndex) {
		assert(jointIndex < skinCluster.inverseBindPoseMatrices.size());
		skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix = Function::Multiply(skinCluster.inverseBindPoseMatrices[jointIndex], joints[jointIndex].skeletonSpaceMatrix);
		skinCluster.mappedPalette[jointIndex].skeletonSpaceInverseTransposeMatrix = Function::Transpose(Function::Inverse(skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix));
	}
	auto* dxCommon = ModelCommon::GetInstance()->GetDxCommon();
	auto* commandList = dxCommon->GetCommandList();
	assert(commandList && SrvManager::GetInstance());

	if (skinCluster.outputVertexCurrentState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
		D3D12_RESOURCE_BARRIER toUavBarrier{};
		toUavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		toUavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		toUavBarrier.Transition.pResource = skinCluster.outputVertexResource.Get();
		toUavBarrier.Transition.StateBefore = skinCluster.outputVertexCurrentState;
		toUavBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		toUavBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		commandList->ResourceBarrier(1, &toUavBarrier);
		skinCluster.outputVertexCurrentState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	}

	ID3D12DescriptorHeap* descriptorHeaps[] = {SrvManager::GetInstance()->GetDescriptorHeap().Get()};
	commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	commandList->SetComputeRootSignature(skinCluster.computeRootSignature.Get());
	commandList->SetPipelineState(skinCluster.computePipelineState.Get());
	commandList->SetComputeRootDescriptorTable(0, SrvManager::GetInstance()->GetGPUDescriptorHandle(skinCluster.paletteSrvIndex));
	commandList->SetComputeRootDescriptorTable(1, SrvManager::GetInstance()->GetGPUDescriptorHandle(skinCluster.inputVertexSrvIndex));
	commandList->SetComputeRootDescriptorTable(2, SrvManager::GetInstance()->GetGPUDescriptorHandle(skinCluster.influenceSrvIndex));
	commandList->SetComputeRootDescriptorTable(3, SrvManager::GetInstance()->GetGPUDescriptorHandle(skinCluster.outputVertexUavIndex));
	commandList->SetComputeRootConstantBufferView(4, skinCluster.skinningInformationResource->GetGPUVirtualAddress());
	const UINT threadGroupCountX = (static_cast<UINT>(skinCluster.mappedSkinningInformation[0].numVertices) + 1023u) / 1024u;
	commandList->Dispatch(threadGroupCountX, 1, 1);

	D3D12_RESOURCE_BARRIER toVertexBufferBarrier{};
	toVertexBufferBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	toVertexBufferBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	toVertexBufferBarrier.Transition.pResource = skinCluster.outputVertexResource.Get();
	toVertexBufferBarrier.Transition.StateBefore = skinCluster.outputVertexCurrentState;
	toVertexBufferBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	toVertexBufferBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commandList->ResourceBarrier(1, &toVertexBufferBarrier);
	skinCluster.outputVertexCurrentState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
}