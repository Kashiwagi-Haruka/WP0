#define NOMINMAX
#include "ParticleManager.h"
#include "Camera.h"
#include "DirectXCommon.h"
#include "Function.h"
#include "Logger.h"
#include "SrvManager/SrvManager.h"
#include "TextureManager.h"
#include "VertexData.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <numbers>

namespace {
constexpr uint32_t kParticleThreadGroupSize = 256;
constexpr uint32_t kParticleDispatchCount = 4096 / kParticleThreadGroupSize;
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
} // namespace

std::unique_ptr<ParticleManager> ParticleManager::instance = nullptr;

ParticleManager* ParticleManager::GetInstance() {
	if (instance == nullptr) {
		instance = std::make_unique<ParticleManager>();
	}
	return instance.get();
}

void ParticleManager::SetBlendMode(BlendMode mode) { currentBlendMode_ = mode; }

void ParticleManager::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;
	srvManager_ = SrvManager::GetInstance();

	srand((unsigned int)time(nullptr));

	CreateGraphicsPipeline();
	CreateComputePipeline();

	VertexData vertices[6] = {
	    {{-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
        {{-1.0f, 1.0f, 0.0f, 1.0f},  {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{1.0f, 1.0f, 0.0f, 1.0f},   {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
	    {{-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
        {{1.0f, 1.0f, 0.0f, 1.0f},   {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{1.0f, -1.0f, 0.0f, 1.0f},  {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}
    };

	vertexBuffer_ = dxCommon_->CreateBufferResource(sizeof(vertices));
	vbView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vbView_.StrideInBytes = sizeof(VertexData);
	vbView_.SizeInBytes = sizeof(vertices);

	void* mapped = nullptr;
	vertexBuffer_->Map(0, nullptr, &mapped);
	memcpy(mapped, vertices, sizeof(vertices));
	vertexBuffer_->Unmap(0, nullptr);

	particleResource_ = CreateDefaultBufferResource(dxCommon_, sizeof(float) * 20 * kMaxParticles_, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
	particleResourceState_ = D3D12_RESOURCE_STATE_COMMON;
	particleSrvIndex_ = srvManager_->Allocate();
	srvManager_->CreateSRVforStructuredBuffer(particleSrvIndex_, particleResource_.Get(), kMaxParticles_, sizeof(float) * 20);
	particleUavIndex_ = srvManager_->Allocate();
	srvManager_->CreateUAVforStructuredBuffer(particleUavIndex_, particleResource_.Get(), kMaxParticles_, sizeof(float) * 20);
	freeListIndexResource_ = CreateDefaultBufferResource(dxCommon_, sizeof(int32_t), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
	freeListIndexResourceState_ = D3D12_RESOURCE_STATE_COMMON;
	freeListIndexUavIndex_ = srvManager_->Allocate();
	srvManager_->CreateUAVforStructuredBuffer(freeListIndexUavIndex_, freeListIndexResource_.Get(), 1, sizeof(int32_t));

	freeListResource_ = CreateDefaultBufferResource(dxCommon_, sizeof(uint32_t) * kMaxParticles_, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
	freeListResourceState_ = D3D12_RESOURCE_STATE_COMMON;
	freeListUavIndex_ = srvManager_->Allocate();
	srvManager_->CreateUAVforStructuredBuffer(freeListUavIndex_, freeListResource_.Get(), kMaxParticles_, sizeof(uint32_t));
	assert(freeListIndexUavIndex_ == particleUavIndex_ + 1);
	assert(freeListUavIndex_ == freeListIndexUavIndex_ + 1);

	emitterResource_ = dxCommon_->CreateBufferResource(sizeof(EmitterSphere));
	emitterResource_->Map(0, nullptr, reinterpret_cast<void**>(&emitterData_));
	*emitterData_ = EmitterSphere{};

	perFrameResource_ = dxCommon_->CreateBufferResource(sizeof(PerFrame));
	perFrameResource_->Map(0, nullptr, reinterpret_cast<void**>(&perFrameData_));
	*perFrameData_ = PerFrame{};

		updateEmitterResource_ = dxCommon_->CreateBufferResource(sizeof(EmitterSphere));
	updateEmitterResource_->Map(0, nullptr, reinterpret_cast<void**>(&updateEmitterData_));
	*updateEmitterData_ = EmitterSphere{};

	updatePerFrameResource_ = dxCommon_->CreateBufferResource(sizeof(PerFrame));
	updatePerFrameResource_->Map(0, nullptr, reinterpret_cast<void**>(&updatePerFrameData_));
	*updatePerFrameData_ = PerFrame{};

	initEmitterResource_ = dxCommon_->CreateBufferResource(sizeof(EmitterSphere));
	initEmitterResource_->Map(0, nullptr, reinterpret_cast<void**>(&initEmitterData_));
	*initEmitterData_ = EmitterSphere{};
	initEmitterData_->emit = 2;

	initPerFrameResource_ = dxCommon_->CreateBufferResource(sizeof(PerFrame));
	initPerFrameResource_->Map(0, nullptr, reinterpret_cast<void**>(&initPerFrameData_));
	*initPerFrameData_ = PerFrame{};
	initPerFrameData_->deltaTime = 1.0f / 60.0f;
}

void ParticleManager::CreateParticleGroup(const std::string& name, const std::string& textureFilePath) {
	assert(particleGroups.find(name) == particleGroups.end() && "ParticleGroup name already exists!");
	ParticleGroup newGroup{};
	newGroup.textureFilePath = textureFilePath;
	newGroup.textureSrvIndex = TextureManager::GetInstance()->GetTextureIndexByfilePath(textureFilePath);
	particleGroups[name] = std::move(newGroup);
}

void ParticleManager::SetCamera(Camera* camera) { camera_ = camera; }

void ParticleManager::Update(Camera* camera) {
	camera_ = camera;
	if (!isParticleInitialized_) {
		InitializeParticlesByCompute();
	}

	if (!isParticleInitialized_) {
		return;
	}
	UpdateParticlesByCompute();
}

void ParticleManager::Draw(const std::string& name) {
	struct alignas(256) MaterialCB {
		float color[4];
		int enableLighting;
		float pad[3];
		float uvTransform[16];
	};

	auto it = particleGroups.find(name);
	if (it == particleGroups.end()) {
		return;
	}

	if (!isParticleInitialized_) {
		InitializeParticlesByCompute();
	}

	if (!cbResource_) {
		cbResource_ = dxCommon_->CreateBufferResource(sizeof(MaterialCB));
	}
	if (!perViewCB_) {
		perViewCB_ = dxCommon_->CreateBufferResource(sizeof(PerView));
	}

	{
		void* p = nullptr;
		cbResource_->Map(0, nullptr, &p);
		auto* m = reinterpret_cast<MaterialCB*>(p);
		m->color[0] = 1.0f;
		m->color[1] = 1.0f;
		m->color[2] = 1.0f;
		m->color[3] = 1.0f;
		m->enableLighting = 0;
		m->pad[0] = m->pad[1] = m->pad[2] = 0.0f;
		for (int i = 0; i < 16; i++) {
			m->uvTransform[i] = (i % 5 == 0) ? 1.0f : 0.0f;
		}
		cbResource_->Unmap(0, nullptr);
	}

	PerView perView{};
	if (camera_) {
		const Matrix4x4& view = camera_->GetViewMatrix();
		const Matrix4x4& proj = camera_->GetProjectionMatrix();
		perView.viewProjection = Function::Multiply(view, proj);
		Matrix4x4 billboard = Function::Inverse(view);
		billboard.m[3][0] = billboard.m[3][1] = billboard.m[3][2] = 0.0f;
		perView.billboardMatrix = billboard;
	} else {
		perView.viewProjection = Function::MakeIdentity4x4();
		perView.billboardMatrix = Function::MakeIdentity4x4();
	}
	{
		void* p = nullptr;
		perViewCB_->Map(0, nullptr, &p);
		memcpy(p, &perView, sizeof(perView));
		perViewCB_->Unmap(0, nullptr);
	}

	if (particleResourceState_ != D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = particleResource_.Get();
		barrier.Transition.StateBefore = particleResourceState_;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);
		particleResourceState_ = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
	}

	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	ID3D12DescriptorHeap* heaps[] = {srvManager_->GetDescriptorHeap().Get()};
	dxCommon_->GetCommandList()->SetDescriptorHeaps(1, heaps);
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, cbResource_->GetGPUVirtualAddress());
	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(1, srvManager_->GetGPUDescriptorHandle(it->second.textureSrvIndex));
	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, srvManager_->GetGPUDescriptorHandle(particleSrvIndex_));
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(3, perViewCB_->GetGPUVirtualAddress());
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vbView_);
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineState_[(int)currentBlendMode_].Get());
	dxCommon_->GetCommandList()->DrawInstanced(6, kMaxParticles_, 0, 0);
}

void ParticleManager::Emit(
    const std::string& name, const Transform& transform, uint32_t count, const Vector3& accel, const AABB& area, float life, const Vector4& beforeColor, const Vector4& afterColor,
    float emissionAngle) {
	(void)name;
	if (!isParticleInitialized_) {
		InitializeParticlesByCompute();
	}

	if (!emitterData_ || !perFrameData_ || !updateEmitterData_) {
		return;
	}

	emitterData_->translate = transform.translate;
	emitterData_->radius = std::max({std::abs(area.min.x), std::abs(area.min.y), std::abs(area.min.z), std::abs(area.max.x), std::abs(area.max.y), std::abs(area.max.z)});
	emitterData_->count = std::min(count, kMaxParticles_);
	emitterData_->lifeTime = std::max(life, 1.0f / 60.0f);
	emitterData_->acceleration = accel;
	emitterData_->particleScale = transform.scale;
	emitterData_->beforeColor = beforeColor;
	emitterData_->afterColor = afterColor;
	emitterData_->emissionAngle = std::max(emissionAngle, 0.0f);
	emitterData_->emit = 1;

	perFrameData_->time = 0.0f;
	perFrameData_->deltaTime = dxCommon_->GetDeltaTime();
	if (perFrameData_->deltaTime <= 0.0f) {
		perFrameData_->deltaTime = 1.0f / 60.0f;
	}
	perFrameData_->time += perFrameData_->deltaTime;

	if (particleResourceState_ != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = particleResource_.Get();
		barrier.Transition.StateBefore = particleResourceState_;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);
		particleResourceState_ = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	}

	if (freeListIndexResourceState_ != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = freeListIndexResource_.Get();
		barrier.Transition.StateBefore = freeListIndexResourceState_;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);
		freeListIndexResourceState_ = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	}

	if (freeListResourceState_ != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = freeListResource_.Get();
		barrier.Transition.StateBefore = freeListResourceState_;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);
		freeListResourceState_ = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	}

	ID3D12DescriptorHeap* heaps[] = {srvManager_->GetDescriptorHeap().Get()};
	dxCommon_->GetCommandList()->SetDescriptorHeaps(1, heaps);
	dxCommon_->GetCommandList()->SetPipelineState(emitComputePipelineState_.Get());
	dxCommon_->GetCommandList()->SetComputeRootSignature(computeRootSignature_.Get());
	dxCommon_->GetCommandList()->SetComputeRootDescriptorTable(0, srvManager_->GetGPUDescriptorHandle(particleUavIndex_));
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(1, emitterResource_->GetGPUVirtualAddress());
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(2, perFrameResource_->GetGPUVirtualAddress());
	dxCommon_->GetCommandList()->Dispatch(kParticleDispatchCount, 1, 1);

	D3D12_RESOURCE_BARRIER uavBarriers[3]{};
	uavBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarriers[0].UAV.pResource = particleResource_.Get();
	uavBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarriers[1].UAV.pResource = freeListIndexResource_.Get();
	uavBarriers[2].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarriers[2].UAV.pResource = freeListResource_.Get();
	dxCommon_->GetCommandList()->ResourceBarrier(3, uavBarriers);

}

void ParticleManager::Finalize() {
	Clear();
	instance = nullptr;
}

void ParticleManager::Clear() {
	particleGroups.clear();
	isParticleInitialized_ = false;
}

void ParticleManager::CreateRootsignature() {
	HRESULT hr_;
	D3D12_ROOT_PARAMETER rootParameters[4] = {};

	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	D3D12_DESCRIPTOR_RANGE rangeTexture{};
	rangeTexture.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangeTexture.NumDescriptors = 1;
	rangeTexture.BaseShaderRegister = 0;
	rangeTexture.RegisterSpace = 0;
	rangeTexture.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[1].DescriptorTable.pDescriptorRanges = &rangeTexture;

	D3D12_DESCRIPTOR_RANGE rangeParticle{};
	rangeParticle.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangeParticle.NumDescriptors = 1;
	rangeParticle.BaseShaderRegister = 1;
	rangeParticle.RegisterSpace = 0;
	rangeParticle.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[2].DescriptorTable.pDescriptorRanges = &rangeParticle;

	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[3].Descriptor.ShaderRegister = 1;

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = sampler.AddressV = sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_DESC descRootSig{};
	descRootSig.pParameters = rootParameters;
	descRootSig.NumParameters = _countof(rootParameters);
	descRootSig.pStaticSamplers = &sampler;
	descRootSig.NumStaticSamplers = 1;
	descRootSig.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	hr_ = D3D12SerializeRootSignature(&descRootSig, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob_, &errorBlob_);
	if (FAILED(hr_)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob_->GetBufferPointer()));
		assert(false);
	}

	hr_ = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob_->GetBufferPointer(), signatureBlob_->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr_));
}

void ParticleManager::CreateGraphicsPipeline() {
	CreateRootsignature();
	HRESULT hr_;
	D3D12_INPUT_ELEMENT_DESC inputElements[3] = {};
	inputElements[0] = {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
	inputElements[1] = {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
	inputElements[2] = {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};

	D3D12_INPUT_LAYOUT_DESC inputLayout{};
	inputLayout.pInputElementDescs = inputElements;
	inputLayout.NumElements = _countof(inputElements);

	D3D12_DEPTH_STENCIL_DESC depthDesc{};
	depthDesc.DepthEnable = true;
	depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	auto vsBlob = dxCommon_->CompileShader(L"Resources/shader/Particle/Particle.VS.hlsl", L"vs_6_0");
	auto psBlob = dxCommon_->CompileShader(L"Resources/shader/Particle/Particle.PS.hlsl", L"ps_6_0");

	for (int i = 0; i < BlendMode::kCountOfBlendMode; i++) {
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
		psoDesc.pRootSignature = rootSignature_.Get();
		psoDesc.InputLayout = inputLayout;
		psoDesc.VS = {vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()};
		psoDesc.PS = {psBlob->GetBufferPointer(), psBlob->GetBufferSize()};
		D3D12_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		psoDesc.RasterizerState = rasterizerDesc;
		psoDesc.BlendState = blendModeManager_.SetBlendMode(static_cast<BlendMode>(i));
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		psoDesc.DepthStencilState = depthDesc;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

		hr_ = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&graphicsPipelineState_[i]));
		assert(SUCCEEDED(hr_));
	}
}

void ParticleManager::CreateComputePipeline() {
	D3D12_DESCRIPTOR_RANGE uavRanges[1] = {};
	uavRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	uavRanges[0].NumDescriptors = 3;
	uavRanges[0].BaseShaderRegister = 0;
	uavRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[3] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[0].DescriptorTable.pDescriptorRanges = uavRanges;
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[1].Descriptor.ShaderRegister = 0;
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[2].Descriptor.ShaderRegister = 1;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters = _countof(rootParameters);
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	assert(SUCCEEDED(hr));

	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&computeRootSignature_));
	assert(SUCCEEDED(hr));

auto emitCsBlob = dxCommon_->CompileShader(L"Resources/shader/Particle/Particle.CS.hlsl", L"cs_6_0");
	D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineDesc{};
	computePipelineDesc.pRootSignature = computeRootSignature_.Get();
	computePipelineDesc.CS = {emitCsBlob->GetBufferPointer(), emitCsBlob->GetBufferSize()};

	hr = dxCommon_->GetDevice()->CreateComputePipelineState(&computePipelineDesc, IID_PPV_ARGS(&emitComputePipelineState_));
	assert(SUCCEEDED(hr));

	auto updateCsBlob = dxCommon_->CompileShader(L"Resources/shader/Particle/UpdateParticle.CS.hlsl", L"cs_6_0");
	computePipelineDesc.CS = {updateCsBlob->GetBufferPointer(), updateCsBlob->GetBufferSize()};

	hr = dxCommon_->GetDevice()->CreateComputePipelineState(&computePipelineDesc, IID_PPV_ARGS(&updateComputePipelineState_));
	assert(SUCCEEDED(hr));
}

void ParticleManager::UpdateParticlesByCompute() {
	if (particleResourceState_ != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = particleResource_.Get();
		barrier.Transition.StateBefore = particleResourceState_;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);
		particleResourceState_ = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	}

	if (freeListIndexResourceState_ != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = freeListIndexResource_.Get();
		barrier.Transition.StateBefore = freeListIndexResourceState_;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);
		freeListIndexResourceState_ = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	}

	if (freeListResourceState_ != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = freeListResource_.Get();
		barrier.Transition.StateBefore = freeListResourceState_;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);
		freeListResourceState_ = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	}

		if (!updateEmitterData_ || !updatePerFrameData_) {
		return;
	}

	updateEmitterData_->emit = 0;
	updatePerFrameData_->deltaTime = dxCommon_->GetDeltaTime();
	if (updatePerFrameData_->deltaTime <= 0.0f) {
		updatePerFrameData_->deltaTime = 1.0f / 60.0f;
	}
	updatePerFrameData_->time += updatePerFrameData_->deltaTime;

	ID3D12DescriptorHeap* heaps[] = {srvManager_->GetDescriptorHeap().Get()};
	dxCommon_->GetCommandList()->SetDescriptorHeaps(1, heaps);
	dxCommon_->GetCommandList()->SetPipelineState(updateComputePipelineState_.Get());
	dxCommon_->GetCommandList()->SetComputeRootSignature(computeRootSignature_.Get());
	dxCommon_->GetCommandList()->SetComputeRootDescriptorTable(0, srvManager_->GetGPUDescriptorHandle(particleUavIndex_));
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(1, updateEmitterResource_->GetGPUVirtualAddress());
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(2, updatePerFrameResource_->GetGPUVirtualAddress());
	dxCommon_->GetCommandList()->Dispatch(kParticleDispatchCount, 1, 1);

	D3D12_RESOURCE_BARRIER uavBarriers[3]{};
	uavBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarriers[0].UAV.pResource = particleResource_.Get();
	uavBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarriers[1].UAV.pResource = freeListIndexResource_.Get();
	uavBarriers[2].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarriers[2].UAV.pResource = freeListResource_.Get();
	dxCommon_->GetCommandList()->ResourceBarrier(3, uavBarriers);
}

void ParticleManager::InitializeParticlesByCompute() {
	if (isParticleInitialized_) {
		return;
	}

	if (particleResourceState_ != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = particleResource_.Get();
		barrier.Transition.StateBefore = particleResourceState_;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);
		particleResourceState_ = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	}

	if (freeListIndexResourceState_ != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = freeListIndexResource_.Get();
		barrier.Transition.StateBefore = freeListIndexResourceState_;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);
		freeListIndexResourceState_ = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	}

	if (freeListResourceState_ != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = freeListResource_.Get();
		barrier.Transition.StateBefore = freeListResourceState_;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);
		freeListResourceState_ = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	}

	if (!initEmitterData_ || !initPerFrameData_) {
		return;
	}

	initEmitterData_->emit = 2;
	initPerFrameData_->time = 0.0f;
	initPerFrameData_->deltaTime = 1.0f / 60.0f;

	ID3D12DescriptorHeap* heaps[] = {srvManager_->GetDescriptorHeap().Get()};
	dxCommon_->GetCommandList()->SetDescriptorHeaps(1, heaps);
	dxCommon_->GetCommandList()->SetPipelineState(emitComputePipelineState_.Get());
	dxCommon_->GetCommandList()->SetComputeRootSignature(computeRootSignature_.Get());
	dxCommon_->GetCommandList()->SetComputeRootDescriptorTable(0, srvManager_->GetGPUDescriptorHandle(particleUavIndex_));
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(1, initEmitterResource_->GetGPUVirtualAddress());
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(2, initPerFrameResource_->GetGPUVirtualAddress());
	dxCommon_->GetCommandList()->Dispatch(kParticleDispatchCount, 1, 1);

	D3D12_RESOURCE_BARRIER uavBarriers[3]{};
	uavBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarriers[0].UAV.pResource = particleResource_.Get();
	uavBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarriers[1].UAV.pResource = freeListIndexResource_.Get();
	uavBarriers[2].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarriers[2].UAV.pResource = freeListResource_.Get();
	dxCommon_->GetCommandList()->ResourceBarrier(3, uavBarriers);

	// init リソースは固定値のまま維持し、GPU 実行タイミング競合を避ける。
	isParticleInitialized_ = true;
}