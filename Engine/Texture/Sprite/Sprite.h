#pragma once
#include "Matrix4x4.h"
#include "Transform.h"
#include "Vector3.h"
#include "Vector4.h"
#include "VertexData.h"
#include <Windows.h>
#include <d3d12.h>
#include <stdint.h>
#include <string>
#include <wrl.h>

class Sprite {

	/*----------------------------------*/

	struct alignas(256) TransformationMatrix {
		Matrix4x4 WVP;   // 64 バイト
		Matrix4x4 World; // 64 バイト
		                 // ここで自動的に 128 バイト分のパディングが入って、
		                 // sizeof(TransformationMatrix) == 256 になる
	};

	struct Material {

		Vector4 color;
		int enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
	};

	Transform transform_{
	    transform_.scale = {1.0f, 1.0f, 1.0f},
	    transform_.rotate = {0.0f, 0.0f, 0.0f},
	    transform_.translate = {0.0f, 0.0f, 0.0f},
	};
	Material materialData_;

	Vector2 anchorPoint = {0, 0};

	Vector2 textureLeftTop = {0, 0};
	Vector2 textureSize{};
	Vector2 textureCutSize{};

	float left = 0.0f - anchorPoint.x;
	float right = 1.0f - anchorPoint.x;
	float top = 0.0f - anchorPoint.y;
	float bottom = 1.0f - anchorPoint.y;

	bool isFlipX_ = false;
	bool isFripY_ = false;

	int handle_ = 0;

	/*----------------------------------*/

	UINT currentSpriteVertexOffset_ = 0;
	static const UINT kMaxSpriteVertices = 6 * 10000; // フレーム最大 1000 スプライト分

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;

	VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;
	Material* material = nullptr;
	TransformationMatrix* transformData = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	uint32_t textureIndex = 0;

public:
	void Initialize(uint32_t Handle);

	void Update();

	void Draw();
	void SetTextureHandle(uint32_t Handle) { textureIndex = Handle; }
	void SetPosition(const Vector2& pos) {
		transform_.translate.x = pos.x;
		transform_.translate.y = pos.y;
		transform_.translate.z = 0.0f;
	}
	void SetScale(const Vector2& scale) {
		transform_.scale.x = scale.x;
		transform_.scale.y = scale.y;
		transform_.scale.z = 1.0f;
	}
	void SetRotation(const float& rot) {

		transform_.rotate.x = 0.0f;
		transform_.rotate.y = 0.0f;
		transform_.rotate.z = rot;
	}
	void SetColor(const Vector4& color);
	void SetTextureRange(const Vector2& leftTop, const Vector2& TextureSize);
	bool GetIsFlipX() { return isFlipX_; };
	void SetIsFlipX(const bool isFlipX);
	bool GetIsFlipY() { return isFripY_; };
	void SetIsFlipY(const bool isFlipY);
	void AdjustTextureSize();
	const Vector2& GetAnchorPoint() const { return anchorPoint; };
	void SetAnchorPoint(const Vector2 anchorPoint) {
		this->anchorPoint;
		left = 0.0f - anchorPoint.x;
		right = 1.0f - anchorPoint.x;
		top = 0.0f - anchorPoint.y;
		bottom = 1.0f - anchorPoint.y;
	};
};
