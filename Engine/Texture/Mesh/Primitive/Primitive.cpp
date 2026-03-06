#define NOMINMAX
#include "Primitive.h"
#include "Camera.h"
#include "DirectXCommon.h"
#include "Function.h"
#include "Engine/Editor/Hierarchy.h"
#include "Object3d/Object3dCommon.h"
#include "SrvManager/SrvManager.h"
#include "TextureManager.h"
#include "WinApp.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace {
constexpr uint32_t kDefaultSlices = 32;
constexpr float kHalfSize = 0.5f;
constexpr float kBandWidth = 0.2f;
struct MeshData {
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
};

// 4頂点の四角形を 2 三角形としてメッシュへ追加
void AppendQuad(MeshData& mesh, const VertexData& v0, const VertexData& v1, const VertexData& v2, const VertexData& v3) {
	uint32_t baseIndex = static_cast<uint32_t>(mesh.vertices.size());
	mesh.vertices.push_back(v0);
	mesh.vertices.push_back(v1);
	mesh.vertices.push_back(v2);
	mesh.vertices.push_back(v3);
	mesh.indices.push_back(baseIndex + 0);
	mesh.indices.push_back(baseIndex + 2);
	mesh.indices.push_back(baseIndex + 1);
	mesh.indices.push_back(baseIndex + 2);
	mesh.indices.push_back(baseIndex + 3);
	mesh.indices.push_back(baseIndex + 1);
}

// 正方形の板ポリゴンを生成
MeshData BuildPlane() {
	MeshData mesh;
	VertexData v0 = {
	    {-kHalfSize, -kHalfSize, 0.0f, 1.0f},
        {0.0f, 1.0f},
        {0.0f, 0.0f, -1.0f}
    };
	VertexData v1 = {
	    {kHalfSize, -kHalfSize, 0.0f, 1.0f},
        {1.0f, 1.0f},
        {0.0f, 0.0f, -1.0f}
    };
	VertexData v2 = {
	    {-kHalfSize, kHalfSize, 0.0f, 1.0f},
        {0.0f, 0.0f},
        {0.0f, 0.0f, -1.0f}
    };
	VertexData v3 = {
	    {kHalfSize, kHalfSize, 0.0f, 1.0f},
        {1.0f, 0.0f},
        {0.0f, 0.0f, -1.0f}
    };
	AppendQuad(mesh, v0, v1, v2, v3);
	return mesh;
}

// 単純な三角形を生成
MeshData BuildTriangle() {
	MeshData mesh;
	mesh.vertices = {
	    {{-kHalfSize, -kHalfSize, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},
	    {{0.0f, kHalfSize, 0.0f, 1.0f},        {0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}},
	    {{kHalfSize, -kHalfSize, 0.0f, 1.0f},  {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},
	};
	mesh.indices = {0, 1, 2};
	return mesh;
}

// 扇形分割で円板を生成
MeshData BuildCircle(uint32_t slices) {
	MeshData mesh;
	mesh.vertices.reserve(slices + 1);
	mesh.indices.reserve(slices * 3);
	mesh.vertices.push_back({
	    {0.0f, 0.0f, 0.0f, 1.0f},
        {0.5f, 0.5f},
        {0.0f, 0.0f, -1.0f}
    });

	for (uint32_t i = 0; i <= slices; ++i) {
		float angle = (static_cast<float>(i) / static_cast<float>(slices)) * Function::kPi * 2.0f;
		float x = std::cos(angle) * kHalfSize;
		float y = std::sin(angle) * kHalfSize;
		mesh.vertices.push_back({
		    {x, y, 0.0f, 1.0f},
            {(x / (kHalfSize * 2.0f)) + 0.5f, 0.5f - (y / (kHalfSize * 2.0f))},
            {0.0f, 0.0f, -1.0f}
        });
	}

	for (uint32_t i = 1; i <= slices; ++i) {
		mesh.indices.push_back(0);
		mesh.indices.push_back(i);
		mesh.indices.push_back(i + 1);
	}

	return mesh;
}
// 原点中心の線分を生成
MeshData BuildLine() {
	MeshData mesh;
	mesh.vertices = {
	    {{-kHalfSize, 0.0f, 0.0f, 1.0f}, {0.0f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	    {{kHalfSize, 0.0f, 0.0f, 1.0f},  {1.0f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	};
	mesh.indices = {0, 1};
	return mesh;
}

// 内径と外径を持つリングを生成
MeshData BuildRing(uint32_t slices, const Vector2& innerDiameter, const Vector2& outerDiameter) {
	MeshData mesh;
	const float innerRadiusX = std::max(innerDiameter.x * 0.5f, 0.0f);
	const float innerRadiusY = std::max(innerDiameter.y * 0.5f, 0.0f);
	const float outerRadiusX = std::max(outerDiameter.x * 0.5f, 0.0f);
	const float outerRadiusY = std::max(outerDiameter.y * 0.5f, 0.0f);
	mesh.vertices.reserve((slices + 1) * 2);
	mesh.indices.reserve(slices * 6);

	for (uint32_t i = 0; i <= slices; ++i) {
		float angle = (static_cast<float>(i) / static_cast<float>(slices)) * Function::kPi * 2.0f;
		float cosA = std::cos(angle);
		float sinA = std::sin(angle);
		Vector3 normal = {0.0f, 0.0f, -1.0f};
		mesh.vertices.push_back({
		    {outerRadiusX * cosA, outerRadiusY * sinA, 0.0f, 1.0f},
            {0.5f + cosA * 0.5f, 0.5f - sinA * 0.5f},
            normal
        });
		mesh.vertices.push_back({
		    {innerRadiusX * cosA, innerRadiusY * sinA, 0.0f, 1.0f},
            {0.5f + cosA * 0.3f, 0.5f - sinA * 0.3f},
            normal
        });
	}

	for (uint32_t i = 0; i < slices; ++i) {
		uint32_t outer0 = i * 2;
		uint32_t inner0 = outer0 + 1;
		uint32_t outer1 = outer0 + 2;
		uint32_t inner1 = outer0 + 3;
		mesh.indices.push_back(outer0);
		mesh.indices.push_back(inner0);
		mesh.indices.push_back(outer1);
		mesh.indices.push_back(outer1);
		mesh.indices.push_back(inner0);
		mesh.indices.push_back(inner1);
	}

	return mesh;
}

// 緯度経度分割で球を生成
MeshData BuildSphere(uint32_t slices, uint32_t stacks) {
	MeshData mesh;
	mesh.vertices.reserve((slices + 1) * (stacks + 1));
	mesh.indices.reserve(slices * stacks * 6);

	for (uint32_t y = 0; y <= stacks; ++y) {
		float v = static_cast<float>(y) / static_cast<float>(stacks);
		float theta = v * Function::kPi;
		float sinTheta = std::sin(theta);
		float cosTheta = std::cos(theta);
		for (uint32_t x = 0; x <= slices; ++x) {
			float u = static_cast<float>(x) / static_cast<float>(slices);
			float phi = u * Function::kPi * 2.0f;
			float sinPhi = std::sin(phi);
			float cosPhi = std::cos(phi);
			Vector3 normal = {sinTheta * cosPhi, cosTheta, sinTheta * sinPhi};
			Vector4 position = {normal.x * kHalfSize, normal.y * kHalfSize, normal.z * kHalfSize, 1.0f};
			mesh.vertices.push_back({
			    position, {u, 1.0f - v},
                 normal
            });
		}
	}

	for (uint32_t y = 0; y < stacks; ++y) {
		for (uint32_t x = 0; x < slices; ++x) {
			uint32_t i0 = y * (slices + 1) + x;
			uint32_t i1 = i0 + 1;
			uint32_t i2 = i0 + (slices + 1);
			uint32_t i3 = i2 + 1;
			mesh.indices.push_back(i0);
			mesh.indices.push_back(i2);
			mesh.indices.push_back(i1);
			mesh.indices.push_back(i1);
			mesh.indices.push_back(i2);
			mesh.indices.push_back(i3);
		}
	}

	return mesh;
}

// 主半径・副半径でトーラスを生成
MeshData BuildTorus(uint32_t slices, uint32_t stacks) {
	MeshData mesh;
	const float majorRadius = kHalfSize * 0.8f;
	const float minorRadius = kHalfSize * 0.3f;
	mesh.vertices.reserve((slices + 1) * (stacks + 1));
	mesh.indices.reserve(slices * stacks * 6);

	for (uint32_t y = 0; y <= stacks; ++y) {
		float v = static_cast<float>(y) / static_cast<float>(stacks);
		float theta = v * Function::kPi * 2.0f;
		float cosTheta = std::cos(theta);
		float sinTheta = std::sin(theta);
		for (uint32_t x = 0; x <= slices; ++x) {
			float u = static_cast<float>(x) / static_cast<float>(slices);
			float phi = u * Function::kPi * 2.0f;
			float cosPhi = std::cos(phi);
			float sinPhi = std::sin(phi);
			float radial = majorRadius + minorRadius * cosTheta;
			Vector3 normal = {cosPhi * cosTheta, sinTheta, sinPhi * cosTheta};
			Vector4 position = {radial * cosPhi, minorRadius * sinTheta, radial * sinPhi, 1.0f};
			mesh.vertices.push_back({
			    position, {u, 1.0f - v},
                 normal
            });
		}
	}

	for (uint32_t y = 0; y < stacks; ++y) {
		for (uint32_t x = 0; x < slices; ++x) {
			uint32_t i0 = y * (slices + 1) + x;
			uint32_t i1 = i0 + 1;
			uint32_t i2 = i0 + (slices + 1);
			uint32_t i3 = i2 + 1;
			mesh.indices.push_back(i0);
			mesh.indices.push_back(i2);
			mesh.indices.push_back(i1);
			mesh.indices.push_back(i1);
			mesh.indices.push_back(i2);
			mesh.indices.push_back(i3);
		}
	}

	return mesh;
}

// 側面+上下蓋付き円柱を生成
MeshData BuildCylinder(uint32_t slices) {
	MeshData mesh;
	const float height = kHalfSize;
	const float radius = kHalfSize;

	for (uint32_t i = 0; i <= slices; ++i) {
		float u = static_cast<float>(i) / static_cast<float>(slices);
		float angle = u * Function::kPi * 2.0f;
		float cosA = std::cos(angle);
		float sinA = std::sin(angle);
		Vector3 normal = {cosA, 0.0f, sinA};
		mesh.vertices.push_back({
		    {radius * cosA, -height, radius * sinA, 1.0f},
            {u, 1.0f},
            normal
        });
		mesh.vertices.push_back({
		    {radius * cosA, height, radius * sinA, 1.0f},
            {u, 0.0f},
            normal
        });
	}

	for (uint32_t i = 0; i < slices; ++i) {
		uint32_t base = i * 2;
		mesh.indices.push_back(base);
		mesh.indices.push_back(base + 1);
		mesh.indices.push_back(base + 2);
		mesh.indices.push_back(base + 2);
		mesh.indices.push_back(base + 1);
		mesh.indices.push_back(base + 3);
	}

	uint32_t baseIndex = static_cast<uint32_t>(mesh.vertices.size());
	mesh.vertices.push_back({
	    {0.0f, -height, 0.0f, 1.0f},
        {0.5f, 0.5f},
        {0.0f, -1.0f, 0.0f}
    });
	mesh.vertices.push_back({
	    {0.0f, height, 0.0f, 1.0f},
        {0.5f, 0.5f},
        {0.0f, 1.0f, 0.0f}
    });

	for (uint32_t i = 0; i <= slices; ++i) {
		float u = static_cast<float>(i) / static_cast<float>(slices);
		float angle = u * Function::kPi * 2.0f;
		float cosA = std::cos(angle);
		float sinA = std::sin(angle);
		mesh.vertices.push_back({
		    {radius * cosA, -height, radius * sinA, 1.0f},
            {0.5f + cosA * 0.5f, 0.5f - sinA * 0.5f},
            {0.0f, -1.0f, 0.0f}
        });
		mesh.vertices.push_back({
		    {radius * cosA, height, radius * sinA, 1.0f},
            {0.5f + cosA * 0.5f, 0.5f - sinA * 0.5f},
            {0.0f, 1.0f, 0.0f}
        });
	}

	for (uint32_t i = 0; i < slices; ++i) {
		uint32_t bottom0 = baseIndex + 2 + i * 2;
		uint32_t bottom1 = bottom0 + 2;
		mesh.indices.push_back(baseIndex);
		mesh.indices.push_back(bottom1);
		mesh.indices.push_back(bottom0);

		uint32_t top0 = baseIndex + 3 + i * 2;
		uint32_t top1 = top0 + 2;
		mesh.indices.push_back(baseIndex + 1);
		mesh.indices.push_back(top0);
		mesh.indices.push_back(top1);
	}

	return mesh;
}
// 側面+底面付き円錐を生成
MeshData BuildCone(uint32_t slices) {
	MeshData mesh;
	const float height = kHalfSize;
	const float radius = kHalfSize;
	Vector4 tip = {0.0f, height, 0.0f, 1.0f};

	for (uint32_t i = 0; i <= slices; ++i) {
		float u = static_cast<float>(i) / static_cast<float>(slices);
		float angle = u * Function::kPi * 2.0f;
		float cosA = std::cos(angle);
		float sinA = std::sin(angle);
		Vector3 normal = Function::Normalize({cosA, radius / height, sinA});
		mesh.vertices.push_back({
		    {radius * cosA, -height, radius * sinA, 1.0f},
            {u, 1.0f},
            normal
        });
		mesh.vertices.push_back({
		    tip, {u, 0.0f},
             normal
        });
	}

	for (uint32_t i = 0; i < slices; ++i) {
		uint32_t base = i * 2;
		mesh.indices.push_back(base);
		mesh.indices.push_back(base + 1);
		mesh.indices.push_back(base + 2);
	}

	uint32_t centerIndex = static_cast<uint32_t>(mesh.vertices.size());
	mesh.vertices.push_back({
	    {0.0f, -height, 0.0f, 1.0f},
        {0.5f, 0.5f},
        {0.0f, -1.0f, 0.0f}
    });
	for (uint32_t i = 0; i <= slices; ++i) {
		float u = static_cast<float>(i) / static_cast<float>(slices);
		float angle = u * Function::kPi * 2.0f;
		float cosA = std::cos(angle);
		float sinA = std::sin(angle);
		mesh.vertices.push_back({
		    {radius * cosA, -height, radius * sinA, 1.0f},
            {0.5f + cosA * 0.5f, 0.5f - sinA * 0.5f},
            {0.0f, -1.0f, 0.0f}
        });
	}

	for (uint32_t i = 0; i < slices; ++i) {
		uint32_t ring0 = centerIndex + 1 + i;
		uint32_t ring1 = ring0 + 1;
		mesh.indices.push_back(centerIndex);
		mesh.indices.push_back(ring1);
		mesh.indices.push_back(ring0);
	}

	return mesh;
}
// 6面それぞれの法線を持つボックスを生成
MeshData BuildBox() {
	MeshData mesh;
	const float s = kHalfSize;
	Vector3 normals[6] = {
	    {0.0f,  0.0f,  -1.0f},
        {0.0f,  0.0f,  1.0f },
        {-1.0f, 0.0f,  0.0f },
        {1.0f,  0.0f,  0.0f },
        {0.0f,  1.0f,  0.0f },
        {0.0f,  -1.0f, 0.0f },
	};

	Vector4 positions[6][4] = {
	    {{-s, -s, -s, 1.0f}, {s, -s, -s, 1.0f},  {-s, s, -s, 1.0f},  {s, s, -s, 1.0f} }, // front
	    {{s, -s, s, 1.0f},   {-s, -s, s, 1.0f},  {s, s, s, 1.0f},    {-s, s, s, 1.0f} }, // back
	    {{-s, -s, s, 1.0f},  {-s, -s, -s, 1.0f}, {-s, s, s, 1.0f},   {-s, s, -s, 1.0f}}, // left
	    {{s, -s, -s, 1.0f},  {s, -s, s, 1.0f},   {s, s, -s, 1.0f},   {s, s, s, 1.0f}  }, // right
	    {{-s, s, -s, 1.0f},  {s, s, -s, 1.0f},   {-s, s, s, 1.0f},   {s, s, s, 1.0f}  }, // top
	    {{-s, -s, s, 1.0f},  {s, -s, s, 1.0f},   {-s, -s, -s, 1.0f}, {s, -s, -s, 1.0f}}, // bottom
	};

	Vector2 uvs[4] = {
	    {0.0f, 1.0f},
        {1.0f, 1.0f},
        {0.0f, 0.0f},
        {1.0f, 0.0f}
    };

	for (int face = 0; face < 6; ++face) {
		VertexData v0 = {positions[face][0], uvs[0], normals[face]};
		VertexData v1 = {positions[face][1], uvs[1], normals[face]};
		VertexData v2 = {positions[face][2], uvs[2], normals[face]};
		VertexData v3 = {positions[face][3], uvs[3], normals[face]};
		AppendQuad(mesh, v0, v1, v2, v3);
	}

	return mesh;
}
// 表裏を持つ細長い帯を生成
MeshData BuildBand(uint32_t segments) {
	MeshData mesh;
	const float length = kHalfSize * 2.0f;
	const float halfWidth = kBandWidth * 0.5f;
	mesh.vertices.reserve((segments + 1) * 2);
	mesh.indices.reserve(segments * 12);

	for (uint32_t i = 0; i <= segments; ++i) {
		float t = static_cast<float>(i) / static_cast<float>(segments);
		float x = -kHalfSize + length * t;
		Vector3 normal = {0.0f, 0.0f, -1.0f};
		mesh.vertices.push_back({
		    {x, -halfWidth, 0.0f, 1.0f},
            {t, 1.0f},
            normal
        });
		mesh.vertices.push_back({
		    {x, halfWidth, 0.0f, 1.0f},
            {t, 0.0f},
            normal
        });
	}

	for (uint32_t i = 0; i < segments; ++i) {
		uint32_t base = i * 2;
		mesh.indices.push_back(base);
		mesh.indices.push_back(base + 2);
		mesh.indices.push_back(base + 1);
		mesh.indices.push_back(base + 1);
		mesh.indices.push_back(base + 2);
		mesh.indices.push_back(base + 3);
		mesh.indices.push_back(base + 1);
		mesh.indices.push_back(base + 2);
		mesh.indices.push_back(base);
		mesh.indices.push_back(base + 3);
		mesh.indices.push_back(base + 2);
		mesh.indices.push_back(base + 1);
	}

	return mesh;
}

uint32_t ClampSlices(uint32_t slices) { return std::max(3u, slices); }

uint32_t ComputeStacksFromSlices(uint32_t slices) { return std::max(2u, slices / 2); }

MeshData BuildMeshByPrimitiveName(Primitive::PrimitiveName primitiveName, uint32_t slices, uint32_t stacks) {
	switch (primitiveName) {
	case Primitive::Plane:
		return BuildPlane();
	case Primitive::Circle:
		return BuildCircle(slices);
	case Primitive::Ring:
		return BuildRing(slices, {kHalfSize * 1.2f, kHalfSize * 1.2f}, {kHalfSize * 2.0f, kHalfSize * 2.0f});
	case Primitive::Sphere:
		return BuildSphere(slices, stacks);
	case Primitive::Torus:
		return BuildTorus(slices, stacks);
	case Primitive::Cylinder:
		return BuildCylinder(slices);
	case Primitive::Cone:
		return BuildCone(slices);
	case Primitive::Triangle:
		return BuildTriangle();
	case Primitive::Line:
		return BuildLine();
	case Primitive::Box:
		return BuildBox();
	case Primitive::Band:
		return BuildBand(slices);
	default:
		return BuildPlane();
	}
}
} // namespace
Primitive::~Primitive() { Hierarchy::GetInstance()->UnregisterPrimitive(this); }
// 既定テクスチャでプリミティブを初期化
void Primitive::Initialize(PrimitiveName name) { Initialize(name, kDefaultSlices); }
// 指定分割数で既定テクスチャ初期化
void Primitive::Initialize(PrimitiveName name, uint32_t slices) {
	primitiveName_ = name;
	slices_ = ClampSlices(slices);
	stacks_ = ComputeStacksFromSlices(slices_);
	camera_ = nullptr;
	transformResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(TransformationMatrix));
	cameraResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(CameraForGpu));
	textureViewProjection0_ = Function::MakeIdentity4x4();
	textureViewProjection1_ = Function::MakeIdentity4x4();
	usePortalProjection_ = false;
	MeshData mesh = BuildMeshByPrimitiveName(primitiveName_, slices_, stacks_);
	vertices_ = std::move(mesh.vertices);
	indices_ = std::move(mesh.indices);

	vertexResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(VertexData) * vertices_.size());
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertices_.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, vertices_.data(), sizeof(VertexData) * vertices_.size());
	vertexResource_->Unmap(0, nullptr);

	indexResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(uint32_t) * indices_.size());
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * indices_.size());
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
	void* mappedIndex = nullptr;
	indexResource_->Map(0, nullptr, &mappedIndex);
	std::memcpy(mappedIndex, indices_.data(), sizeof(uint32_t) * indices_.size());
	indexResource_->Unmap(0, nullptr);

	size_t alignedSize = (sizeof(Material) + 0xFF) & ~0xFF;
	materialResource_ = Object3dCommon::GetInstance()->CreateBufferResource(alignedSize);
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData_->enableLighting = 1;
	materialData_->uvTransform = Function::MakeIdentity4x4();
	materialData_->shininess = 40.0f;
	materialData_->environmentCoefficient = 0.0f;
	materialData_->grayscaleEnabled = 0;
	materialData_->sepiaEnabled = 0;
	materialData_->distortionStrength = 0.0f;
	materialData_->distortionFalloff = 1.0f;
	materialResource_->Unmap(0, nullptr);

	const std::string texturePath = "Resources/3d/uvChecker.png";
	TextureManager::GetInstance()->LoadTextureName(texturePath);
	textureIndex_ = TextureManager::GetInstance()->GetTextureIndexByfilePath(texturePath);

	isUseSetWorld = false;
	if (editorRegistrationEnabled_) {
		Hierarchy::GetInstance()->RegisterPrimitive(this);
		
	}
}
// 指定テクスチャでプリミティブを初期化
void Primitive::Initialize(PrimitiveName name, const std::string& texturePath) { Initialize(name, texturePath, kDefaultSlices); }
// 指定分割数・指定テクスチャでプリミティブを初期化
void Primitive::Initialize(PrimitiveName name, const std::string& texturePath, uint32_t slices) {
	primitiveName_ = name;
	slices_ = ClampSlices(slices);
	stacks_ = ComputeStacksFromSlices(slices_);
	camera_ = nullptr;
	transformResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(TransformationMatrix));
	cameraResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(CameraForGpu));
	textureViewProjection0_ = Function::MakeIdentity4x4();
	textureViewProjection1_ = Function::MakeIdentity4x4();
	usePortalProjection_ = false;
	MeshData mesh = BuildMeshByPrimitiveName(primitiveName_, slices_, stacks_);
	vertices_ = std::move(mesh.vertices);
	indices_ = std::move(mesh.indices);

	vertexResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(VertexData) * vertices_.size());
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertices_.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, vertices_.data(), sizeof(VertexData) * vertices_.size());
	vertexResource_->Unmap(0, nullptr);

	indexResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(uint32_t) * indices_.size());
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * indices_.size());
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
	void* mappedIndex = nullptr;
	indexResource_->Map(0, nullptr, &mappedIndex);
	std::memcpy(mappedIndex, indices_.data(), sizeof(uint32_t) * indices_.size());
	indexResource_->Unmap(0, nullptr);

	size_t alignedSize = (sizeof(Material) + 0xFF) & ~0xFF;
	materialResource_ = Object3dCommon::GetInstance()->CreateBufferResource(alignedSize);
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData_->enableLighting = 1;
	materialData_->uvTransform = Function::MakeIdentity4x4();
	materialData_->shininess = 40.0f;
	materialData_->environmentCoefficient = 0.0f;
	materialData_->grayscaleEnabled = 0;
	materialData_->sepiaEnabled = 0;
	materialData_->distortionStrength = 0.0f;
	materialData_->distortionFalloff = 1.0f;
	materialResource_->Unmap(0, nullptr);

	TextureManager::GetInstance()->LoadTextureName(texturePath);
	textureIndex_ = TextureManager::GetInstance()->GetTextureIndexByfilePath(texturePath);

	isUseSetWorld = false;
	if (editorRegistrationEnabled_) {
		Hierarchy::GetInstance()->RegisterPrimitive(this);

	}
}
// 座標変換やマテリアル定数を更新
void Primitive::Update() {
	if (primitiveName_ == Primitive::Line && useLinePositions_) {
		vertices_[0].position = {lineStart_.x, lineStart_.y, lineStart_.z, 1.0f};
		vertices_[1].position = {lineEnd_.x, lineEnd_.y, lineEnd_.z, 1.0f};
		VertexData* vertexData = nullptr;
		vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
		std::memcpy(vertexData, vertices_.data(), sizeof(VertexData) * vertices_.size());
		vertexResource_->Unmap(0, nullptr);
	}

	if (!isUseSetWorld) {
		worldMatrix = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	}

	Camera* activeCamera = camera_;
	if (!activeCamera) {
		activeCamera = Object3dCommon::GetInstance()->GetDefaultCamera();
	}

	UpdateCameraMatrices();
}

void Primitive::UpdateCameraMatrices() {
	Camera* activeCamera = camera_;
	if (!activeCamera) {
		activeCamera = Object3dCommon::GetInstance()->GetDefaultCamera();
	}

	if (activeCamera) {
		worldViewProjectionMatrix = Function::Multiply(Function::Multiply(worldMatrix, activeCamera->GetViewMatrix()), activeCamera->GetProjectionMatrix());
	} else {
		// Camera が未初期化のフレームではクラッシュ回避のため単位行列を使う
		worldViewProjectionMatrix = worldMatrix;
	}

	transformResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	transformationMatrixData_->WVP = worldViewProjectionMatrix;
	transformationMatrixData_->World = worldMatrix;
	transformationMatrixData_->LightWVP = Function::Multiply(worldMatrix, Object3dCommon::GetInstance()->GetDirectionalLightViewProjectionMatrix());
	transformationMatrixData_->WorldInverseTranspose = Function::Inverse(worldMatrix);
	transformResource_->Unmap(0, nullptr);

	cameraResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));
	if (activeCamera) {
		cameraData_->worldPosition = activeCamera->GetWorldTranslate();
	} else {
		cameraData_->worldPosition = {transform_.translate};
	}
	cameraData_->screenSize = {static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight)};
	cameraData_->fullscreenGrayscaleEnabled = Object3dCommon::GetInstance()->IsFullScreenGrayscaleEnabled() ? 1 : 0;
	cameraData_->fullscreenSepiaEnabled = Object3dCommon::GetInstance()->IsFullScreenSepiaEnabled() ? 1 : 0;
	cameraData_->textureViewProjection0 = textureViewProjection0_;
	cameraData_->textureViewProjection1 = textureViewProjection1_;
	cameraData_->portalCameraWorld0 = portalCameraWorld0_;
	cameraData_->portalCameraWorld1 = portalCameraWorld1_;
	cameraData_->usePortalProjection = usePortalProjection_ ? 1 : 0;
	cameraResource_->Unmap(0, nullptr);
}
// 設定済みのメッシュを描画
void Primitive::Draw() {
	ID3D12DescriptorHeap* descriptorHeaps[] = {SrvManager::GetInstance()->GetDescriptorHeap().Get()};
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView_);

	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformResource_->GetGPUVirtualAddress());
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraResource_->GetGPUVirtualAddress());
	
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex_);
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, srvHandle);
	SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(8, Object3dCommon::GetInstance()->GetPointLightSrvIndex());
	SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(9, Object3dCommon::GetInstance()->GetSpotLightSrvIndex());
	SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(10, Object3dCommon::GetInstance()->GetAreaLightSrvIndex());
	uint32_t secondaryTexture = secondaryTextureIndex_;
	if (secondaryTexture == UINT32_MAX) {
		secondaryTexture = Object3dCommon::GetInstance()->GetEnvironmentMapSrvIndex();
	}
	SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(11, secondaryTexture);
	if (!Object3dCommon::GetInstance()->IsShadowMapPassActive()) {
		SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(12, Object3dCommon::GetInstance()->GetShadowMapSrvIndex());
	}
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(static_cast<UINT>(indices_.size()), 1, 0, 0, 0);
}

void Primitive::SetCamera(Camera* camera) { camera_ = camera; }
void Primitive::SetTranslate(Vector3 translate) {
	transform_.translate = translate;
	isUseSetWorld = false;
	useLinePositions_ = false;
}
void Primitive::SetRotate(Vector3 rotate) {
	transform_.rotate = rotate;
	isUseSetWorld = false;
	useLinePositions_ = false;
}
void Primitive::SetScale(Vector3 scale) {
	transform_.scale = scale;
	isUseSetWorld = false;
	useLinePositions_ = false;
}
void Primitive::SetTransform(Transform transform) {
	transform_ = transform;
	isUseSetWorld = false;
	useLinePositions_ = false;
}
void Primitive::SetWorldMatrix(Matrix4x4 matrix) {
	worldMatrix = matrix;
	isUseSetWorld = true;
	useLinePositions_ = false;
}
// Line の始点・終点を更新して頂点へ反映
void Primitive::SetLinePositions(const Vector3& start, const Vector3& end) {
	if (primitiveName_ != Primitive::Line) {
		return;
	}
	lineStart_ = start;
	lineEnd_ = end;
	useLinePositions_ = true;
	isUseSetWorld = true;
	worldMatrix = Function::MakeIdentity4x4();
}
// Ring の内径・外径を XY ごとに更新して頂点へ反映
void Primitive::SetRingDiameterXY(const Vector2& innerDiameter, const Vector2& outerDiameter) {
	if (primitiveName_ != Primitive::Ring) {
		return;
	}
	MeshData mesh = BuildRing(slices_, innerDiameter, outerDiameter);
	SetMeshData(mesh.vertices, mesh.indices);
}
// 外部メッシュへ置き換え、GPU バッファを再作成
void Primitive::SetMeshData(const std::vector<VertexData>& vertices, const std::vector<uint32_t>& indices) {
	if (vertices.empty() || indices.empty()) {
		return;
	}
	vertices_ = vertices;
	indices_ = indices;

	vertexResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(VertexData) * vertices_.size());
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertices_.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, vertices_.data(), sizeof(VertexData) * vertices_.size());
	vertexResource_->Unmap(0, nullptr);

	indexResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(uint32_t) * indices_.size());
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * indices_.size());
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
	void* mappedIndex = nullptr;
	indexResource_->Map(0, nullptr, &mappedIndex);
	std::memcpy(mappedIndex, indices_.data(), sizeof(uint32_t) * indices_.size());
	indexResource_->Unmap(0, nullptr);
}
void Primitive::SetColor(Vector4 color) {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = color;
	materialResource_->Unmap(0, nullptr);
}
void Primitive::SetEnableLighting(bool enable) {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->enableLighting = enable ? 1 : 0;
	materialResource_->Unmap(0, nullptr);
}
void Primitive::SetUvTransform(const Matrix4x4& uvTransform) {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->uvTransform = uvTransform;
	materialResource_->Unmap(0, nullptr);
}
void Primitive::SetUvTransform(Vector3 scale, Vector3 rotate, Vector3 translate, Vector2 anchor) {
	uvScale_ = scale;
	uvRotate_ = rotate;
	uvTranslate_ = translate;
	uvAnchor_ = anchor;
	SetUvTransform(Function::MakeAffineMatrix(uvScale_, uvRotate_, uvTranslate_, uvAnchor_));
}
void Primitive::SetUvAnchor(Vector2 anchor) {
	uvAnchor_ = anchor;
	SetUvTransform(Function::MakeAffineMatrix(uvScale_, uvRotate_, uvTranslate_, uvAnchor_));
}
void Primitive::SetShininess(float shininess) {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->shininess = shininess;
	materialResource_->Unmap(0, nullptr);
}
void Primitive::SetEnvironmentCoefficient(float coefficient) {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->environmentCoefficient = coefficient;
	materialResource_->Unmap(0, nullptr);
}
void Primitive::SetGrayscaleEnabled(bool enable) {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->grayscaleEnabled = enable ? 1 : 0;
	materialResource_->Unmap(0, nullptr);
}

void Primitive::SetSepiaEnabled(bool enable) {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->sepiaEnabled = enable ? 1 : 0;
	materialResource_->Unmap(0, nullptr);
}
void Primitive::SetDistortionStrength(float strength) {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->distortionStrength = strength;
	materialResource_->Unmap(0, nullptr);
}

void Primitive::SetDistortionFalloff(float falloff) {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->distortionFalloff = falloff;
	materialResource_->Unmap(0, nullptr);
}
void Primitive::SetTextureIndex(uint32_t textureIndex) { textureIndex_ = textureIndex; }
void Primitive::SetSecondaryTextureIndex(uint32_t textureIndex) { secondaryTextureIndex_ = textureIndex; }

void Primitive::ClearSecondaryTextureIndex() { secondaryTextureIndex_ = UINT32_MAX; }
void Primitive::SetPortalProjectionMatrices(
    const Matrix4x4& textureViewProjection0, const Matrix4x4& textureViewProjection1, const Matrix4x4& portalCameraWorld0, const Matrix4x4& portalCameraWorld1) {
	textureViewProjection0_ = textureViewProjection0;
	textureViewProjection1_ = textureViewProjection1;
	portalCameraWorld0_ = portalCameraWorld0;
	portalCameraWorld1_ = portalCameraWorld1;
}
void Primitive::SetPortalProjectionEnabled(bool enabled) { usePortalProjection_ = enabled; }
Vector4 Primitive::GetColor() const {
	if (materialData_) {
		return materialData_->color;
	}
	return {1.0f, 1.0f, 1.0f, 1.0f};
}

bool Primitive::IsLightingEnabled() const {
	if (materialData_) {
		return materialData_->enableLighting != 0;
	}
	return true;
}

float Primitive::GetShininess() const {
	if (materialData_) {
		return materialData_->shininess;
	}
	return 40.0f;
}

float Primitive::GetEnvironmentCoefficient() const {
	if (materialData_) {
		return materialData_->environmentCoefficient;
	}
	return 0.0f;
}

bool Primitive::IsGrayscaleEnabled() const {
	if (materialData_) {
		return materialData_->grayscaleEnabled != 0;
	}
	return false;
}

bool Primitive::IsSepiaEnabled() const {
	if (materialData_) {
		return materialData_->sepiaEnabled != 0;
	}
	return false;
}
float Primitive::GetDistortionStrength() const {
	if (materialData_) {
		return materialData_->distortionStrength;
	}
	return 0.0f;
}
float Primitive::GetDistortionFalloff() const {
	if (materialData_) {
		return materialData_->distortionFalloff;
	}
	return 1.0f;
}