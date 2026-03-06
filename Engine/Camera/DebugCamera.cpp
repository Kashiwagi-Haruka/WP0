#define NOMINMAX
#include "DebugCamera.h"

#include "Function.h"
#include "Input.h"
#include "WinApp.h"
#include <algorithm>

namespace {
Matrix4x4 MakeCameraViewMatrix(const Transform& transform) {
	const Matrix4x4 inverseTranslate = Function::MakeTranslateMatrix(-transform.translate.x, -transform.translate.y, -transform.translate.z);
	const Matrix4x4 inverseRotateY = Function::MakeRotateYMatrix(-transform.rotate.y);
	const Matrix4x4 inverseRotateX = Function::MakeRotateXMatrix(-transform.rotate.x);
	const Matrix4x4 inverseRotateZ = Function::MakeRotateZMatrix(-transform.rotate.z);
	return Function::Multiply(Function::Multiply(Function::Multiply(inverseTranslate, inverseRotateY), inverseRotateX), inverseRotateZ);
}
} // namespace

void DebugCamera::Initialize() {
	// 既定値を実画面サイズに合わせて設定
	fovY_ = 0.45f * 3.14159265f;
	aspectRatio_ = float(WinApp::kClientWidth) / float(WinApp::kClientHeight);
	nearZ_ = 0.1f;
	farZ_ = 10000.0f;
	matRot_ = Function::MakeIdentity4x4();
	translation_ = transform_.translate;
	Update();
}

void DebugCamera::SetTransform(const Transform& transform) {
	// 受け取った姿勢を基準に、Pivot操作用の内部状態を再初期化
	transform_ = transform;
	pivot_ = {0.0f, 0.0f, 0.0f};
	translation_ = transform.translate;
	matRot_ = Function::MakeAffineMatrix({1.0f, 1.0f, 1.0f}, transform.rotate, {0.0f, 0.0f, 0.0f});
}

void DebugCamera::SetRotation(const Vector3& rotation) {
	// 回転角を反映し、回転行列を同期
	transform_.rotate = rotation;
	matRot_ = Function::MakeAffineMatrix({1.0f, 1.0f, 1.0f}, transform_.rotate, {0.0f, 0.0f, 0.0f});
}

void DebugCamera::Update() {
	// 左ドラッグ: 回転 / Shift+左ドラッグ: パン / ホイール: ズーム
	const float rotateSpeed = 0.005f;
	const float panSpeed = 0.02f;
	const float zoomSpeed = 0.01f;

	Input* input = Input::GetInstance();
	const Vector2 mouseMove = input->GetMouseMove();
	const bool isLeftDrag = input->PushMouseButton(Input::MouseButton::kLeft);
	const bool isShift = input->PushKey(DIK_LSHIFT) || input->PushKey(DIK_RSHIFT);

	float dPitch = 0.0f;
	float dYaw = 0.0f;
	if (isLeftDrag && !isShift) {
		dYaw = mouseMove.x * rotateSpeed;
		dPitch = mouseMove.y * rotateSpeed;
	} else if (isLeftDrag && isShift) {
		const Vector3 right = {matRot_.m[0][0], matRot_.m[1][0], matRot_.m[2][0]};
		const Vector3 up = {matRot_.m[0][1], matRot_.m[1][1], matRot_.m[2][1]};
		pivot_ += right * (-mouseMove.x * panSpeed);
		pivot_ += up * (mouseMove.y * panSpeed);
	}

	const float wheelDelta = input->GetMouseWheelDelta();
	if (wheelDelta != 0.0f) {
		translation_.z += wheelDelta * zoomSpeed;
		translation_.z = std::clamp(translation_.z, -500.0f, -1.0f);
	}

	Matrix4x4 matRotDelta = Function::MakeIdentity4x4();
	matRotDelta = Function::Multiply(matRotDelta, Function::MakeRotateXMatrix(dPitch));
	matRotDelta = Function::Multiply(matRotDelta, Function::MakeRotateYMatrix(dYaw));
	matRot_ = Function::Multiply(matRotDelta, matRot_);
	transform_.rotate.x += dPitch;
	transform_.rotate.y += dYaw;

	const Matrix4x4 pivotMat = Function::MakeTranslateMatrix(pivot_);
	const Matrix4x4 scaleMat = Function::MakeScaleMatrix(scale_);
	const Matrix4x4 offsetMat = Function::MakeTranslateMatrix(translation_);
	const Matrix4x4 pivotCameraMatrix = Function::Multiply(Function::Multiply(Function::Multiply(pivotMat, matRot_), scaleMat), offsetMat);

	transform_.translate = {pivotCameraMatrix.m[3][0], pivotCameraMatrix.m[3][1], pivotCameraMatrix.m[3][2]};
	worldMatrix_ = Function::MakeAffineMatrix({1.0f, 1.0f, 1.0f}, transform_.rotate, transform_.translate);
	viewMatrix_ = MakeCameraViewMatrix(transform_);
	projectionMatrix_ = Function::MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearZ_, farZ_);
	viewProjectionMatrix_ = Function::Multiply(viewMatrix_, projectionMatrix_);
}