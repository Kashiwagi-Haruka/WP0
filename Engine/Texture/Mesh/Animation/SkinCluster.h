#pragma once
#include "Matrix4x4.h"
#include "Model/Model.h"
#include "Model/ModelCommon.h"
#include "Skeleton.h"
#include <array>
#include <cstdint>
#include <d3d12.h>
#include <span>
#include <utility>
#include <vector>
#include <wrl.h>

constexpr uint32_t kNumMaxInfluence = 4;

struct VertexInfluence {
	std::array<float, kNumMaxInfluence> weights{};
	std::array<int32_t, kNumMaxInfluence> jointIndices{};
};

struct WellForGPU {
	Matrix4x4 skeletonSpaceMatrix;
	Matrix4x4 skeletonSpaceInverseTransposeMatrix;
};

struct SkinCluster {
	struct SkinningInformation {
		int32_t numVertices = 0;
	};

	std::vector<Matrix4x4> inverseBindPoseMatrices;
	Microsoft::WRL::ComPtr<ID3D12Resource> influenceResource;
	std::span<VertexInfluence> mappedInfluence;
	Microsoft::WRL::ComPtr<ID3D12Resource> paletteResource;
	std::span<WellForGPU> mappedPalette;
	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> paletteSrvHandle{};
	uint32_t paletteSrvIndex = 0;

	Microsoft::WRL::ComPtr<ID3D12Resource> inputVertexResource;
	uint32_t inputVertexSrvIndex = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource> outputVertexResource;
	D3D12_VERTEX_BUFFER_VIEW outputVertexBufferView{};
	uint32_t outputVertexSrvIndex = 0;
	uint32_t outputVertexUavIndex = 0;
	D3D12_RESOURCE_STATES outputVertexCurrentState = D3D12_RESOURCE_STATE_COMMON;
	uint32_t influenceSrvIndex = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource> skinningInformationResource;
	std::span<SkinningInformation> mappedSkinningInformation;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> computeRootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> computePipelineState;
};

SkinCluster CreateSkinCluster(const Skeleton& skeleton, const Model& model);
void UpdateSkinCluster(SkinCluster& skinCluster, const Skeleton& skeleton);