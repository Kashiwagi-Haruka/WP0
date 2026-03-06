#pragma once
#include "CameraForGPU.h"
#include "Matrix4x4.h"
#include "Material.h"
#include "Transform.h"
#include "Vector2.h"
#include "Vector4.h"
#include "VertexData.h"
#include <cstdint>
#include <d3d12.h>
#include <string>
#include <vector>
#include <wrl.h>
class Camera;

class Primitive {

public:
	// 描画可能なプリミティブ形状の種類
	enum PrimitiveName {
		Plane,    // X-Y 平面の四角形
		Circle,   // 中心から三角形分割した円板
		Ring,     // 内径ありのリング
		Sphere,   // UV 展開の球
		Torus,    // ドーナツ形状
		Cylinder, // 上下面付きの円柱
		Cone,     // 底面付きの円錐
		Line,     // 2 点を結ぶ線分
		Triangle, // 単純な三角形
		Box,      // 6 面で構成した直方体
		Band,     // 表裏を持つ帯ポリゴン
	};

private:
	PrimitiveName primitiveName_;
	struct alignas(256) TransformationMatrix {
		Matrix4x4 WVP;                   // 64 バイト
		Matrix4x4 LightWVP;              // 64 バイト
		Matrix4x4 World;                 // 64 バイト
		Matrix4x4 WorldInverseTranspose; // 64 バイト
	};

	Transform transform_ = {
	    {1.0f, 1.0f, 1.0f},
	    {0.0f, 0.0f, 0.0f},
	    {0.0f, 0.0f, 0.0f},
	};

	Camera* camera_;

	TransformationMatrix* transformationMatrixData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformResource_;
	CameraForGpu* cameraData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	Material* materialData_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
	std::vector<VertexData> vertices_;
	std::vector<uint32_t> indices_;
	uint32_t textureIndex_ = 0;
	uint32_t secondaryTextureIndex_ = UINT32_MAX;
	Matrix4x4 worldMatrix;
	Matrix4x4 worldViewProjectionMatrix;
	bool isUseSetWorld;
	Vector3 uvScale_ = {1.0f, 1.0f, 1.0f};
	Vector3 uvRotate_ = {0.0f, 0.0f, 0.0f};
	Vector3 uvTranslate_ = {0.0f, 0.0f, 0.0f};
	Vector2 uvAnchor_ = {0.0f, 0.0f};
	Matrix4x4 textureViewProjection0_{};
	Matrix4x4 textureViewProjection1_{};
	Matrix4x4 portalCameraWorld0_{};
	Matrix4x4 portalCameraWorld1_{};
	bool usePortalProjection_ = false;

public:
	~Primitive();
	// 指定した形状を既定テクスチャで初期化
	void Initialize(PrimitiveName name);
	// 指定した形状を分割数付きで既定テクスチャ初期化
	void Initialize(PrimitiveName name, uint32_t slices);
	// 指定した形状を任意テクスチャで初期化
	void Initialize(PrimitiveName name, const std::string& texturePath);
	// 指定した形状を分割数付きで任意テクスチャ初期化
	void Initialize(PrimitiveName name, const std::string& texturePath, uint32_t slices);
	// 行列・マテリアルなど GPU に渡す定数を更新
	void Update();
	// 現在のワールド行列を使ってカメラ依存の定数だけ更新
	void UpdateCameraMatrices();
	// 現在の設定で描画
	void Draw();

	// 使用するカメラを設定
	void SetCamera(Camera* camera);
	// 平行移動を設定
	void SetTranslate(Vector3 translate);
	// 回転を設定
	void SetRotate(Vector3 rotate);
	// 拡大縮小を設定
	void SetScale(Vector3 scale);
	// Transform をまとめて設定
	void SetTransform(Transform transform);
	// 外部で計算したワールド行列を直接設定
	void SetWorldMatrix(Matrix4x4 matrix);
	// Line 用の始点・終点を設定
	void SetLinePositions(const Vector3& start, const Vector3& end);
	// Ring 用の内径・外径を X/Y ごとに設定
	void SetRingDiameterXY(const Vector2& innerDiameter, const Vector2& outerDiameter);
	// メッシュデータを外部から差し替え
	void SetMeshData(const std::vector<VertexData>& vertices, const std::vector<uint32_t>& indices);
	// ベースカラーを設定
	void SetColor(Vector4 color);
	// ライティング有効/無効
	void SetEnableLighting(bool enable);
	// UV 変換行列を設定
	void SetUvTransform(const Matrix4x4& uvTransform);
	void SetUvTransform(Vector3 scale, Vector3 rotate, Vector3 translate, Vector2 anchor = {0.0f, 0.0f});
	void SetUvAnchor(Vector2 anchor);
	// スペキュラの鋭さを設定
	void SetShininess(float shininess);
	// 環境マップ反射係数を設定
	void SetEnvironmentCoefficient(float coefficient);
	// グレースケール有効/無効
	void SetGrayscaleEnabled(bool enable);
	// セピア有効/無効
	void SetSepiaEnabled(bool enable);
	// 歪み回転の強さを設定
	void SetDistortionStrength(float strength);
	// 歪み回転の外側への効き方を設定
	void SetDistortionFalloff(float falloff);
	// 使用テクスチャの SRV インデックスを直接設定
	void SetTextureIndex(uint32_t textureIndex);
	// サブテクスチャの SRV インデックスを設定 (portal shader の t4)
	void SetSecondaryTextureIndex(uint32_t textureIndex);
	void ClearSecondaryTextureIndex();
	void SetEditorRegistrationEnabled(bool enable) { editorRegistrationEnabled_ = enable; }
	// ポータル投影 UV 用の 2 つのカメラ ViewProjection を設定
	void SetPortalProjectionMatrices(const Matrix4x4& textureViewProjection0, const Matrix4x4& textureViewProjection1, const Matrix4x4& portalCameraWorld0, const Matrix4x4& portalCameraWorld1);
	void SetPortalProjectionEnabled(bool enabled);
	// 現在の Transform を取得
	Transform GetTransform() const { return transform_; }
	// 現在のマテリアル値を取得
	Vector4 GetColor() const;
	bool IsLightingEnabled() const;
	float GetShininess() const;
	float GetEnvironmentCoefficient() const;
	bool IsGrayscaleEnabled() const;
	bool IsSepiaEnabled() const;
	float GetDistortionStrength() const;
	float GetDistortionFalloff() const;
	Vector2 GetUvAnchor() const { return uvAnchor_; }

private:
	// Line 描画時に使う始点
	Vector3 lineStart_ = {0.0f, 0.0f, 0.0f};
	// Line 描画時に使う終点
	Vector3 lineEnd_ = {0.0f, 0.0f, 0.0f};
	// true の場合は lineStart_/lineEnd_ を優先して線を更新
	bool useLinePositions_ = false;
	bool editorRegistrationEnabled_ = true;
	// 曲面系プリミティブの分割数
	uint32_t slices_ = 32;
	// Sphere/Torus の縦分割数
	uint32_t stacks_ = 16;
};