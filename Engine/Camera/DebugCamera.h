#pragma once
#include "Matrix4x4.h"
#include "Transform.h"

class DebugCamera {
	// デバッグ操作対象となるカメラのTransform
	Transform transform_ = {
	    {1.0f,  1.0f,  1.0f  },
	    {0.0f,  0.0f,  0.0f  },
	    {50.0f, 50.0f, -30.0f},
	};
	// transform_ から算出される行列群
	Matrix4x4 worldMatrix_{};
	Matrix4x4 viewMatrix_{};
	Matrix4x4 projectionMatrix_{};
	Matrix4x4 viewProjectionMatrix_{};

	// 投影パラメータ
	float fovY_ = 0.45f;
	float aspectRatio_ = 1280.0f / 720.0f;
	float nearZ_ = 0.1f;
	float farZ_ = 10000.0f;

	// Pivot 操作系（従来挙動を維持）
	Vector3 pivot_ = {0.0f, 0.0f, 0.0f};
	Matrix4x4 matRot_{};
	Vector3 scale_ = {1.0f, 1.0f, 1.0f};
	Vector3 translation_ = {50.0f, 50.0f, -30.0f};

public:
	// 画面サイズに応じた投影設定と内部状態を初期化
	void Initialize();
	// マウス入力に応じて回転・パン・ズームを更新
	void Update();

	const Matrix4x4& GetWorldMatrix() const { return worldMatrix_; }
	const Matrix4x4& GetViewMatrix() const { return viewMatrix_; }
	const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix_; }
	const Matrix4x4& GetViewProjectionMatrix() const { return viewProjectionMatrix_; }

	const Transform& GetTransform() const { return transform_; }
	// 外部指定のTransformを反映し、Pivot操作系の基準も再同期
	void SetTransform(const Transform& transform);
	// 回転のみ差し替え、内部回転行列を再構築
	void SetRotation(const Vector3& rotation);
	// 注視点（Pivot）を明示的に設定
	void SetPivot(const Vector3& pivot) { pivot_ = pivot; }
	// Pivot空間でのカメラオフセットを設定
	void SetTranslation(const Vector3& translation) {
		translation_ = translation;
		transform_.translate = translation;
	}
};