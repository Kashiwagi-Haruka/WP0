#include "Camera.h"
#include "Function.h"

#include "Engine/Loadfile/JSON/JsonManager.h"
#ifdef USE_IMGUI
#include "imgui.h"
#endif
namespace {
// カメラTransformからビュー行列を作成する補助関数
Matrix4x4 MakeCameraViewMatrix(const Transform& transform) {
	const Matrix4x4 inverseTranslate = Function::MakeTranslateMatrix(-transform.translate.x, -transform.translate.y, -transform.translate.z);
	const Matrix4x4 inverseRotateY = Function::MakeRotateYMatrix(-transform.rotate.y);
	const Matrix4x4 inverseRotateX = Function::MakeRotateXMatrix(-transform.rotate.x);
	const Matrix4x4 inverseRotateZ = Function::MakeRotateZMatrix(-transform.rotate.z);
	return Function::Multiply(Function::Multiply(Function::Multiply(inverseTranslate, inverseRotateY), inverseRotateX), inverseRotateZ);
}
} // namespace

int Camera::nextEditorId_ = 0;

Camera::Camera()
    : transform_({
          {1.0f, 1.0f, 1.0f},
          {0.0f, 0.0f, 0.0f},
          {0.0f, 0.0f, 0.0f}
}),
      fovY(0.45f * 3.14159265f), aspectRatio(float(WinApp::kClientWidth) / float(WinApp::kClientHeight)), nearZ(0.1f), farZ(10000.0f),
      worldMatrix_(Function::MakeAffineMatrix({1.0f, 1.0f, 1.0f}, transform_.rotate, transform_.translate)), viewMatrix_(MakeCameraViewMatrix(transform_)),
      projectionMatrix_(Function::MakePerspectiveFovMatrix(fovY, aspectRatio, nearZ, farZ)), viewProjectionMatrix_(Function::Multiply(viewMatrix_, projectionMatrix_)) {
	editorId_ = nextEditorId_++;
	if (editorId_ == 0) {
		LoadEditorData();
	}
}

void Camera::Update() {
	// 現在のTransformと投影設定をもとに各行列を更新
	worldMatrix_ = Function::MakeAffineMatrix({1.0f, 1.0f, 1.0f}, transform_.rotate, transform_.translate);
	viewMatrix_ = MakeCameraViewMatrix(transform_);
	projectionMatrix_ = Function::MakePerspectiveFovMatrix(fovY, aspectRatio, nearZ, farZ);
	viewProjectionMatrix_ = Function::Multiply(viewMatrix_, projectionMatrix_);
}

void Camera::SetViewProjectionMatrix(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix) {
	// 受け取った行列を採用し、派生行列と位置情報を同期
	viewMatrix_ = viewMatrix;
	projectionMatrix_ = projectionMatrix;
	viewProjectionMatrix_ = Function::Multiply(viewMatrix_, projectionMatrix_);
	worldMatrix_ = Function::Inverse(viewMatrix_);
	transform_.translate = {worldMatrix_.m[3][0], worldMatrix_.m[3][1], worldMatrix_.m[3][2]};
}

void Camera::LoadEditorData() {
	JsonManager* jsonManager = JsonManager::GetInstance();
	if (!jsonManager->LoadJson("cameraEditor.json")) {
		editorStatusMessage_ = "Load failed: Resources/JSON/cameraEditor.json";
		return;
	}

	const nlohmann::json& root = jsonManager->GetData();
	if (!root.contains("camera") || !root["camera"].is_object()) {
		editorStatusMessage_ = "Load failed: invalid camera data";
		return;
	}

	const nlohmann::json& cameraJson = root["camera"];
	if (cameraJson.contains("scale") && cameraJson["scale"].is_array() && cameraJson["scale"].size() == 3) {
		transform_.scale = {
		    cameraJson["scale"][0].get<float>(),
		    cameraJson["scale"][1].get<float>(),
		    cameraJson["scale"][2].get<float>(),
		};
	}
	if (cameraJson.contains("rotate") && cameraJson["rotate"].is_array() && cameraJson["rotate"].size() == 3) {
		transform_.rotate = {
		    cameraJson["rotate"][0].get<float>(),
		    cameraJson["rotate"][1].get<float>(),
		    cameraJson["rotate"][2].get<float>(),
		};
	}
	if (cameraJson.contains("translate") && cameraJson["translate"].is_array() && cameraJson["translate"].size() == 3) {
		transform_.translate = {
		    cameraJson["translate"][0].get<float>(),
		    cameraJson["translate"][1].get<float>(),
		    cameraJson["translate"][2].get<float>(),
		};
	}
	if (cameraJson.contains("fovY") && cameraJson["fovY"].is_number()) {
		fovY = cameraJson["fovY"].get<float>();
	}
	if (cameraJson.contains("aspectRatio") && cameraJson["aspectRatio"].is_number()) {
		aspectRatio = cameraJson["aspectRatio"].get<float>();
	}
	if (cameraJson.contains("nearZ") && cameraJson["nearZ"].is_number()) {
		nearZ = cameraJson["nearZ"].get<float>();
	}
	if (cameraJson.contains("farZ") && cameraJson["farZ"].is_number()) {
		farZ = cameraJson["farZ"].get<float>();
	}

	editorStatusMessage_ = "Loaded camera editor settings";
}

void Camera::SaveEditorData() {
	JsonManager* jsonManager = JsonManager::GetInstance();
	nlohmann::json root;
	root["camera"] = {
	    {"scale",       {transform_.scale.x, transform_.scale.y, transform_.scale.z}            },
	    {"rotate",      {transform_.rotate.x, transform_.rotate.y, transform_.rotate.z}         },
	    {"translate",   {transform_.translate.x, transform_.translate.y, transform_.translate.z}},
	    {"fovY",        fovY	                                                                },
	    {"aspectRatio", aspectRatio	                                                         },
	    {"nearZ",       nearZ	                                                               },
	    {"farZ",        farZ	                                                                },
	};

	jsonManager->SetData(root);
	const bool saved = jsonManager->SaveJson("cameraEditor.json");
	editorStatusMessage_ = saved ? "Saved: Resources/JSON/cameraEditor.json" : "Save failed: Resources/JSON/cameraEditor.json";
}

void Camera::DrawEditorInHierarchy() {
#ifdef USE_IMGUI


	if (ImGui::TreeNode("Camera Transform")) {
		ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f);
		ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
		ImGui::DragFloat3("Translate", &transform_.translate.x, 0.01f);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Camera Projection")) {
		ImGui::DragFloat("FovY", &fovY, 0.01f, 0.01f, 3.13f);
		ImGui::DragFloat("AspectRatio", &aspectRatio, 0.01f, 0.1f, 8.0f);
		ImGui::DragFloat("NearZ", &nearZ, 0.01f, 0.001f, 1000.0f);
		ImGui::DragFloat("FarZ", &farZ, 1.0f, 1.0f, 100000.0f);
		ImGui::TreePop();
	}

	if (ImGui::Button("Save Camera Editor")) {
		SaveEditorData();
	}
	ImGui::SameLine();
	if (ImGui::Button("Load Camera Editor")) {
		LoadEditorData();
	}
	if (!editorStatusMessage_.empty()) {
		ImGui::Text("%s", editorStatusMessage_.c_str());
	}
#endif
}
