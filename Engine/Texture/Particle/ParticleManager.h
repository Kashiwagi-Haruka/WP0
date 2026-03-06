#pragma once
#include "BlendMode/BlendModeManager.h"
#include "Matrix4x4.h"
#include "RigidBody.h"
#include "Transform.h"
#include "Vector3.h"
#include "Vector4.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <d3d12.h>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <wrl.h>

struct Particle {
	Transform transform_{
	    .scale = {1, 1, 1},
          .rotate{0, 0, 0},
          .translate{0, 0, 0}
    };

	Vector3 vel{};
	float life{};
	Vector4 beforeColor = {1, 1, 1, 1};
	Vector4 afterColor = {1, 1, 1, 0};
	float fadeSpeed = 0.02f;

	BlendMode blendmode = BlendMode::kBlendModeAlpha;

	Vector3 accel;
	AABB area;
	bool visible = true;
};

class SrvManager;
class DirectXCommon;
class Camera;

class ParticleManager {

public:
	struct ParticleGroup {
		std::string textureFilePath;
		uint32_t textureSrvIndex = 0;
		std::list<Particle> particles;
		uint32_t drawCount = 1024;
	};

public:
	static ParticleManager* GetInstance();
	void Initialize(DirectXCommon* dxCommon);
	void CreateParticleGroup(const std::string& name, const std::string& textureFilePath);
	void Emit(
	    const std::string& name, const Transform& transform, uint32_t count, const Vector3& accel, const AABB& area, float life, const Vector4& beforeColor, const Vector4& afterColor,
	    float emissionAngle);
	void SetCamera(Camera* camera);
	void SetBlendMode(BlendMode mode);

	void Update(Camera* camera);
	void Draw(const std::string& name);
	void Clear();
	void Finalize();

private:
	struct EmitterSphere {
		Vector3 translate{0.0f, 0.0f, 0.0f};
		float radius = 1.0f;
		uint32_t count = 10;
		float frequency = 0.5f;
		float frequencyTime = 0.0f;
		uint32_t emit = 0;
		float lifeTime = 120.0f;
		Vector3 acceleration{0.0f, 0.0f, 0.0f};
		float pad = 0.0f;
		Vector3 particleScale{1.0f, 1.0f, 1.0f};
		Vector4 beforeColor{1.0f, 1.0f, 1.0f, 1.0f};
		Vector4 afterColor{1.0f, 1.0f, 1.0f, 0.0f};
		float emissionAngle = 6.283185307f;
		float emissionAnglePadding[3] = {0.0f, 0.0f, 0.0f};
	};
	static_assert(offsetof(EmitterSphere, beforeColor) == 64, "EmitterSphere.beforeColor layout mismatch with shader cbuffer");
	static_assert(offsetof(EmitterSphere, afterColor) == 80, "EmitterSphere.afterColor layout mismatch with shader cbuffer");
	static_assert(sizeof(EmitterSphere) == 112, "EmitterSphere size mismatch with shader cbuffer");

	struct PerFrame {
		float time = 0.0f;
		float deltaTime = 1.0f / 60.0f;
		float pad[2] = {0.0f, 0.0f};
	};

	struct PerView {
		Matrix4x4 viewProjection;
		Matrix4x4 billboardMatrix;
	};

	static constexpr uint32_t kMaxParticles_ = 4096;

	static std::unique_ptr<ParticleManager> instance;
	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;

	std::unordered_map<std::string, ParticleGroup> particleGroups;
	Camera* camera_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbView_{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_[6];
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob_;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob_;
	BlendModeManager blendModeManager_;
	BlendMode currentBlendMode_ = BlendMode::kBlendModeAlpha;

	Microsoft::WRL::ComPtr<ID3D12Resource> cbResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> perViewCB_;

	Microsoft::WRL::ComPtr<ID3D12Resource> particleResource_;
	uint32_t particleSrvIndex_ = 0;
	uint32_t particleUavIndex_ = 0;
	D3D12_RESOURCE_STATES particleResourceState_ = D3D12_RESOURCE_STATE_COMMON;

	Microsoft::WRL::ComPtr<ID3D12Resource> freeListIndexResource_;
	uint32_t freeListIndexUavIndex_ = 0;
	D3D12_RESOURCE_STATES freeListIndexResourceState_ = D3D12_RESOURCE_STATE_COMMON;

	Microsoft::WRL::ComPtr<ID3D12Resource> freeListResource_;
	uint32_t freeListUavIndex_ = 0;
	D3D12_RESOURCE_STATES freeListResourceState_ = D3D12_RESOURCE_STATE_COMMON;

	// Emit dispatch 用（Emit() で書き換える）
	Microsoft::WRL::ComPtr<ID3D12Resource> emitterResource_;
	EmitterSphere* emitterData_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> perFrameResource_;
	PerFrame* perFrameData_ = nullptr;

	// Update dispatch 用（UpdateParticlesByCompute() で書き換える）
	Microsoft::WRL::ComPtr<ID3D12Resource> updateEmitterResource_;
	EmitterSphere* updateEmitterData_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> updatePerFrameResource_;
	PerFrame* updatePerFrameData_ = nullptr;

	// Initialize dispatch 用（固定値）
	Microsoft::WRL::ComPtr<ID3D12Resource> initEmitterResource_;
	EmitterSphere* initEmitterData_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> initPerFrameResource_;
	PerFrame* initPerFrameData_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> computeRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> emitComputePipelineState_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> updateComputePipelineState_;
	bool isParticleInitialized_ = false;

	void CreateRootsignature();
	void CreateGraphicsPipeline();
	void CreateComputePipeline();
	void InitializeParticlesByCompute();
	void UpdateParticlesByCompute();
};