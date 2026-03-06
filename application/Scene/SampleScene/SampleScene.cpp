#include "SampleScene.h"
#include "Function.h"
#include "GameBase.h"
#include "Input.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include "ParticleManager.h"
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI
#include "SceneManager.h"
#include "Sprite/SpriteCommon.h"
#include "TextureManager.h"
#include "WinApp.h"
#include <numbers>
#include <utility>
namespace {
Transform MakeOppositeSidePortalCameraTransform(const Transform& baseTransform) {
	Transform oppositeTransform = baseTransform;
	oppositeTransform.rotate.y += std::numbers::pi_v<float>;
	return oppositeTransform;
}
Transform MakePortalAnchoredTextureCameraTransform(const Transform& portalTransform, const Transform& adjustmentTransform) {
	Transform cameraTransform = portalTransform;
	cameraTransform.scale = {1.0f, 1.0f, 1.0f};
	cameraTransform = MakeOppositeSidePortalCameraTransform(cameraTransform);
	cameraTransform.rotate += adjustmentTransform.rotate;

	const Matrix4x4 cameraWorld = Function::MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	Vector3 right = {cameraWorld.m[0][0], cameraWorld.m[0][1], cameraWorld.m[0][2]};
	Vector3 up = {cameraWorld.m[1][0], cameraWorld.m[1][1], cameraWorld.m[1][2]};
	Vector3 forward = {cameraWorld.m[2][0], cameraWorld.m[2][1], cameraWorld.m[2][2]};
	if (Function::LengthSquared(right) > 0.000001f) {
		right = Function::Normalize(right);
	}
	if (Function::LengthSquared(up) > 0.000001f) {
		up = Function::Normalize(up);
	}
	if (Function::LengthSquared(forward) > 0.000001f) {
		forward = Function::Normalize(forward);
	}
	cameraTransform.translate += right * adjustmentTransform.translate.x;
	cameraTransform.translate += up * adjustmentTransform.translate.y;
	cameraTransform.translate += forward * (0.05f + adjustmentTransform.translate.z);

	return cameraTransform;
}
} // namespace
SampleScene::SampleScene() {

	uvBallObj_ = std::make_unique<Object3d>();
	fieldObj_ = std::make_unique<Object3d>();
	planeGltf_ = std::make_unique<Object3d>();
	animatedCubeObj_ = std::make_unique<Object3d>();
	humanObj_ = std::make_unique<Object3d>();
	spherePrimitive_ = std::make_unique<Primitive>();
	portalMeshA_ = std::make_unique<PortalMesh>();
	portalMeshB_ = std::make_unique<PortalMesh>();
	/*portalObjectCamera_ = std::make_unique<Camera>();*/
	portalTextureCameraA_ = std::make_unique<Camera>();
	portalTextureCameraB_ = std::make_unique<Camera>();
	cameraTransform_ = {
	    .scale{0.1f, 0.1f, 0.1f  },
        .rotate{0.0f, 0.0f, 0.0f  },
        .translate{0.0f, 5.0f, -10.0f}
    };

	camera_ = std::make_unique<Camera>();
	debugCamera_ = std::make_unique<DebugCamera>();
	camera_->SetTransform(cameraTransform_);

	ModelManager::GetInstance()->LoadModel("Resources/3d", "uvBall");
	ModelManager::GetInstance()->LoadModel("Resources/3d", "terrain");
	ModelManager::GetInstance()->LoadGltfModel("Resources/3d", "planeG");
	ModelManager::GetInstance()->LoadGltfModel("Resources/3d/AnimatedCube", "AnimatedCube");
	ModelManager::GetInstance()->LoadGltfModel("Resources/3d/human", "walk");
	ModelManager::GetInstance()->LoadGltfModel("Resources/3d/human", "sneakWalk");
	ParticleManager::GetInstance()->CreateParticleGroup("sample", "Resources/2d/defaultParticle.png");
}
void SampleScene::Initialize() {

	debugCamera_->Initialize();
	debugCamera_->SetTranslation(cameraTransform_.translate);
	uvBallObj_->Initialize();
	uvBallObj_->SetCamera(camera_.get());
	uvBallObj_->SetModel("uvBall");
	fieldObj_->Initialize();
	fieldObj_->SetCamera(camera_.get());
	fieldObj_->SetModel("terrain");
	planeGltf_->Initialize();
	planeGltf_->SetCamera(camera_.get());
	planeGltf_->SetModel("planeG");
	animatedCubeObj_->Initialize();
	animatedCubeObj_->SetCamera(camera_.get());
	animatedCubeObj_->SetModel("AnimatedCube");
	humanObj_->Initialize();
	humanObj_->SetCamera(camera_.get());
	humanObj_->SetModel("walk");

	spherePrimitive_->Initialize(Primitive::Sphere, 32);
	spherePrimitive_->SetCamera(camera_.get());
	spherePrimitive_->SetEnableLighting(true);
	portalATransform_ = {
	    .scale{1.8f,  1.8f,                      1.0f},
        .rotate{0.0f,0.0f, 0.0f},
        .translate{-3.0f, 1.5f,                      2.0f}
    };
	portalBTransform_ = {
	    .scale{1.8f,	                                1.8f, 1.0f},
        .rotate{std::numbers::pi_v<float> / 2.0f, 0.0f, 0.0f},
        .translate{3.0f,                                    3.5f, 2.0f}
    };
	portalTextureCameraAAdjust_ = {
	    .scale{1.0f, 1.0f, 1.0f},
        .rotate{0.0f, 0.0f, 0.0f},
        .translate{0.0f, 0.0f, 0.0f}
    };
	portalTextureCameraBAdjust_ = {
	    .scale{1.0f, 1.0f, 1.0f},
        .rotate{0.0f, 0.0f, 0.0f},
        .translate{0.0f, 0.0f, 0.0f}
    };
	portalTextureCameraATransform_ = MakePortalAnchoredTextureCameraTransform(portalBTransform_, portalTextureCameraAAdjust_);
	portalTextureCameraA_->SetTransform(portalTextureCameraATransform_);
	portalTextureCameraA_->Update();
	portalTextureCameraBTransform_ = MakePortalAnchoredTextureCameraTransform(portalATransform_, portalTextureCameraBAdjust_);
	portalTextureCameraB_->SetTransform(portalTextureCameraBTransform_);
	portalTextureCameraB_->Update();

	portalMeshA_->Initialize("Resources/TD3_3102/2d/atHome.jpg");
	portalMeshB_->Initialize("Resources/TD3_3102/2d/atHome.jpg");
	portalRenderTextureA_.Initialize(WinApp::kClientWidth, WinApp::kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, {0.05f, 0.05f, 0.08f, 1.0f});
	portalRenderTextureB_.Initialize(WinApp::kClientWidth, WinApp::kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, {0.05f, 0.05f, 0.08f, 1.0f});
	portalMeshA_->SetTextureIndex(portalRenderTextureB_.GetSrvIndex());
	portalMeshB_->SetTextureIndex(portalRenderTextureA_.GetSrvIndex());
	portalMeshA_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
	portalMeshB_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
	portalMeshA_->SetTextureCamera(portalTextureCameraA_.get());
	portalMeshB_->SetTextureCamera(portalTextureCameraB_.get());
	uvBallTransform_ = {
	    .scale{1.0f, 1.0f, 1.0f},
        .rotate{0.0f, 0.0f, 0.0f},
        .translate{0.0f, 0.0f, 0.0f}
    };
	planeGTransform_ = {
	    .scale{1.0f, 1.0f, 1.0f},
        .rotate{0.0f, 0.0f, 0.0f},
        .translate{0.0f, 1.0f, 0.0f}
    };
	animatedCubeTransform_ = {
	    .scale{1.0f, 1.0f, 1.0f},
        .rotate{0.0f, 0.0f, 0.0f},
        .translate{3.0f, 1.0f, 0.0f}
    };
	humanTransform_ = {
	    .scale{100.0f,	                        100.0f,                    100.0f},
        .rotate{-std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float>, 0.0f  },
        .translate{0.0f,                              1.0f,                      -3.0f }
    };
	particleTransform_ = {
	    .scale{0.1f, 0.1f, 0.1f },
        .rotate{0.0f, 0.0f, 0.0f },
        .translate{0.0f, 1.0f, -3.0f}
    };

	/*portalObjectCameraTransform_ = {
	    .scale{1.0f, 1.0f, 1.0f  },
        .rotate{0.0f, 0.0f, 0.0f  },
        .translate{0.0f, 5.0f, -10.0f}
    };
	portalObjectCamera_->SetTransform(portalObjectCameraTransform_);
	portalObjectCamera_->Update();*/
	portalMeshA_->SetTransform(portalATransform_);
	portalMeshB_->SetTransform(portalBTransform_);
	sampleParticleEmitter_ = std::make_unique<ParticleEmitter>("sample");
	sampleParticleEmitter_->SetTransform(particleTransform_);
	sampleParticleEmitter_->SetFrequency(0.1f);
	sampleParticleEmitter_->SetCount(5);
	sampleParticleEmitter_->SetAcceleration({0.0f, 0.0f, 0.0f});
	sampleParticleEmitter_->SetAreaMin({-0.5f, -0.5f, -0.5f});
	sampleParticleEmitter_->SetAreaMax({0.5f, 0.5f, 0.5f});
	planeGltf_->SetTransform(planeGTransform_);
	animatedCubeAnimation_ = Animation::LoadAnimationData("Resources/3d/AnimatedCube", "AnimatedCube");
	animatedCubeObj_->SetAnimation(&animatedCubeAnimation_, true);
	animatedCubeObj_->SetTransform(animatedCubeTransform_);
	humanAnimationClips_ = Animation::LoadAnimationClips("Resources/3d/human", "walk");
	std::vector<Animation::AnimationData> sneakClips = Animation::LoadAnimationClips("Resources/3d/human", "sneakWalk");
	humanAnimationClips_.insert(humanAnimationClips_.end(), sneakClips.begin(), sneakClips.end());
	if (!humanAnimationClips_.empty()) {
		currentHumanAnimationIndex_ = 0;
		humanObj_->SetAnimation(&humanAnimationClips_[currentHumanAnimationIndex_], true);
	}
	humanObj_->SetTransform(humanTransform_);
	if (Model* walkModel = ModelManager::GetInstance()->FindModel("walk")) {
		humanSkeleton_ = std::make_unique<Skeleton>(Skeleton().Create(walkModel->GetModelData().rootnode));
		humanSkinCluster_ = CreateSkinCluster(*humanSkeleton_, *walkModel);
		if (!humanSkinCluster_.mappedPalette.empty()) {
			humanObj_->SetSkinCluster(&humanSkinCluster_);
		}
	}

	uvSprite = std::make_unique<Sprite>();
	uvSprite->Initialize(TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/uvChecker.png"));
	uvSprite->SetScale(Vector2(100, 100));
	uvSprite->SetRotation(0);
	uvSprite->SetPosition(Vector2(0, 0));

		overlayCameraSprite_ = std::make_unique<Sprite>();
	overlayCameraSprite_->Initialize(portalRenderTextureA_.GetSrvIndex());
	overlayCameraSprite_->SetScale(Vector2(static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight)));
	overlayCameraSprite_->SetRotation(0.0f);
	overlayCameraSprite_->SetPosition(Vector2(0.0f, 0.0f));

	activePointLightCount_ = 2;
	pointLights_[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
	pointLights_[0].position = {0.0f, 5.0f, 0.0f};
	pointLights_[0].intensity = 1.0f;
	pointLights_[0].radius = 10.0f;
	pointLights_[0].decay = 1.0f;
	pointLights_[1].color = {1.0f, 0.0f, 0.0f, 1.0f};
	pointLights_[1].position = {5.0f, 5.0f, 5.0f};
	pointLights_[1].intensity = 1.0f;
	pointLights_[1].radius = 10.0f;
	pointLights_[1].decay = 1.0f;

	directionalLight_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	directionalLight_.direction = {0.0f, -1.0f, 0.0f};
	directionalLight_.intensity = 1.0f;

	activeSpotLightCount_ = 2;
	spotLights_[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
	spotLights_[0].position = {2.0f, 1.25f, 0.0f};
	spotLights_[0].direction = {-1.0f, -1.0f, 0.0f};
	spotLights_[0].intensity = 4.0f;
	spotLights_[0].distance = 7.0f;
	spotLights_[0].decay = 2.0f;
	spotLights_[0].cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
	spotLights_[0].cosFalloffStart = std::cos(std::numbers::pi_v<float> / 4.0f);

	spotLights_[1].color = {1.0f, 1.0f, 1.0f, 1.0f};
	spotLights_[1].position = {2.0f, 1.25f, 0.0f};
	spotLights_[1].direction = {-1.0f, -1.0f, 0.0f};
	spotLights_[1].intensity = 4.0f;
	spotLights_[1].distance = 7.0f;
	spotLights_[1].decay = 2.0f;
	spotLights_[1].cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
	spotLights_[1].cosFalloffStart = std::cos(std::numbers::pi_v<float> / 4.0f);

	activeAreaLightCount_ = 2;
	areaLights_[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
	areaLights_[0].position = {0.0f, 3.0f, 0.0f};
	areaLights_[0].normal = {1.0f, -1.0f, 0.0f};
	areaLights_[0].intensity = 4.0f;
	areaLights_[0].width = 2.0f;
	areaLights_[0].height = 2.0f;
	areaLights_[0].radius = 0.1f;
	areaLights_[0].decay = 2.0f;

	areaLights_[1].color = {1.0f, 1.0f, 1.0f, 1.0f};
	areaLights_[1].position = {-5.0f, 3.0f, 0.0f};
	areaLights_[1].normal = {1.0f, -1.0f, 0.0f};
	areaLights_[1].intensity = 4.0f;
	areaLights_[1].width = 2.0f;
	areaLights_[1].height = 2.0f;
	areaLights_[1].radius = 0.1f;
	areaLights_[1].decay = 2.0f;
}

void SampleScene::Update() {
#ifdef USE_IMGUI
	if (ImGui::Begin("SampleCamera")) {
		ImGui::Checkbox("Use Debug Camera (F1)", &useDebugCamera_);
		ImGui::Text("Debug: LMB drag rotate, Shift+LMB drag pan, Wheel zoom");
		if (ImGui::TreeNode("Transform")) {

			if (!useDebugCamera_) {
				ImGui::DragFloat3("Scale", &cameraTransform_.scale.x, 0.01f);
				ImGui::DragFloat3("Rotate", &cameraTransform_.rotate.x, 0.01f);
				ImGui::DragFloat3("Translate", &cameraTransform_.translate.x, 0.01f);
			}
			ImGui::TreePop();
		}
		ImGui::End();
	}

	if (ImGui::Begin("Pad Input")) {
		ImGui::Text("押されているパッドボタン");
		const std::array<std::pair<Input::PadButton, const char*>, 14> padButtons = {
		    {{Input::PadButton::kButtonA, "A"},
		     {Input::PadButton::kButtonB, "B"},
		     {Input::PadButton::kButtonX, "X"},
		     {Input::PadButton::kButtonY, "Y"},
		     {Input::PadButton::kButtonLeftShoulder, "LB"},
		     {Input::PadButton::kButtonRightShoulder, "RB"},
		     {Input::PadButton::kButtonBack, "Back"},
		     {Input::PadButton::kButtonStart, "Start"},
		     {Input::PadButton::kButtonLeftThumb, "LStick"},
		     {Input::PadButton::kButtonRightThumb, "RStick"},
		     {Input::PadButton::kButtonUp, "DPad Up"},
		     {Input::PadButton::kButtonDown, "DPad Down"},
		     {Input::PadButton::kButtonLeft, "DPad Left"},
		     {Input::PadButton::kButtonRight, "DPad Right"}}
        };

		Input* input = Input::GetInstance();
		bool hasPush = false;
		for (const auto& [button, buttonName] : padButtons) {
			if (input->PushButton(button)) {
				ImGui::BulletText("%s", buttonName);
				hasPush = true;
			}
		}
		if (input->PushLeftTrigger()) {
			ImGui::BulletText("LT");
			hasPush = true;
		}
		if (input->PushRightTrigger()) {
			ImGui::BulletText("RT");
			hasPush = true;
		}
		if (!hasPush) {
			ImGui::TextDisabled("(なし)");
		}

		ImGui::Separator();
		ImGui::Text("このフレームで押されたボタン");
		bool hasTrigger = false;
		for (const auto& [button, buttonName] : padButtons) {
			if (input->TriggerButton(button)) {
				ImGui::BulletText("%s", buttonName);
				hasTrigger = true;
			}
		}
		if (input->TriggerLeftTrigger()) {
			ImGui::BulletText("LT");
			hasTrigger = true;
		}
		if (input->TriggerRightTrigger()) {
			ImGui::BulletText("RT");
			hasTrigger = true;
		}
		if (!hasTrigger) {
			ImGui::TextDisabled("(なし)");
		}
		ImGui::Separator();
		ImGui::Text("アナログ入力");
		const Vector2 leftStick = input->GetJoyStickLXY();
		const Vector2 rightStick = input->GetJoyStickRXY();
		ImGui::Text("Left Stick  : X %.3f / Y %.3f", leftStick.x, leftStick.y);
		ImGui::Text("Right Stick : X %.3f / Y %.3f", rightStick.x, rightStick.y);
		ImGui::Text("LT: %.3f", input->GetLeftTrigger());
		ImGui::Text("RT: %.3f", input->GetRightTrigger());
	}
	ImGui::End();
	if (ImGui::Begin("Human")) {
		if (ImGui::TreeNode("Transform")) {
			ImGui::DragFloat3("Scale", &humanTransform_.scale.x, 0.1f);
			ImGui::DragFloat3("Rotate", &humanTransform_.rotate.x, 0.1f);
			ImGui::DragFloat3("Translate", &humanTransform_.translate.x, 0.1f);
			ImGui::TreePop();
		}
		if (!humanAnimationClips_.empty()) {
			std::vector<const char*> animationNames;
			animationNames.reserve(humanAnimationClips_.size());
			for (const auto& clip : humanAnimationClips_) {
				animationNames.push_back(clip.name.c_str());
			}
			int selectedIndex = static_cast<int>(currentHumanAnimationIndex_);
			if (ImGui::Combo("Animation", &selectedIndex, animationNames.data(), static_cast<int>(animationNames.size()))) {
				currentHumanAnimationIndex_ = static_cast<size_t>(selectedIndex);
				humanObj_->SetAnimation(&humanAnimationClips_[currentHumanAnimationIndex_], true);
				humanAnimationTime_ = 0.0f;
			}
		}
	}
	ImGui::End();
	if (ImGui::Begin("Particle Editor")) {
		ImGui::Text("Sample Particle Emitter");
		if (ImGui::TreeNode("Emitter Transform")) {
			ImGui::DragFloat3("Scale##Particle", &particleTransform_.scale.x, 0.01f);
			ImGui::DragFloat3("Rotate##Particle", &particleTransform_.rotate.x, 0.01f);
			ImGui::DragFloat3("Translate##Particle", &particleTransform_.translate.x, 0.01f);
			ImGui::TreePop();
		}

		if (sampleParticleEmitter_) {
			float frequency = sampleParticleEmitter_->GetFrequency();
			if (ImGui::DragFloat("Frequency", &frequency, 0.01f, 0.0f, 10.0f)) {
				sampleParticleEmitter_->SetFrequency(frequency);
			}

			int emitCount = static_cast<int>(sampleParticleEmitter_->GetCount());
			if (ImGui::DragInt("EmitCount", &emitCount, 1, 1, 1024)) {
				sampleParticleEmitter_->SetCount(static_cast<uint32_t>(emitCount));
			}

			Vector3 acceleration = sampleParticleEmitter_->GetAcceleration();
			if (ImGui::DragFloat3("Acceleration", &acceleration.x, 0.01f)) {
				sampleParticleEmitter_->SetAcceleration(acceleration);
			}

			Vector3 areaMin = sampleParticleEmitter_->GetAreaMin();
			if (ImGui::DragFloat3("AreaMin", &areaMin.x, 0.01f)) {
				sampleParticleEmitter_->SetAreaMin(areaMin);
			}

			Vector3 areaMax = sampleParticleEmitter_->GetAreaMax();
			if (ImGui::DragFloat3("AreaMax", &areaMax.x, 0.01f)) {
				sampleParticleEmitter_->SetAreaMax(areaMax);
			}

			float life = sampleParticleEmitter_->GetLife();
			if (ImGui::DragFloat("Life", &life, 1.0f, 0.0f, 10000.0f)) {
				sampleParticleEmitter_->SetLife(life);
			}
			Vector4 beforeColor = sampleParticleEmitter_->GetBeforeColor();
			if (ImGui::ColorEdit4("BeforeColor", &beforeColor.x)) {
				sampleParticleEmitter_->SetBeforeColor(beforeColor);
			}

			Vector4 afterColor = sampleParticleEmitter_->GetAfterColor();
			if (ImGui::ColorEdit4("AfterColor", &afterColor.x)) {
				sampleParticleEmitter_->SetAfterColor(afterColor);
			}
			if (ImGui::Button("Emit Now")) {
				sampleParticleEmitter_->Emit();
			}
		}
	}
	ImGui::End();
	if (ImGui::Begin("ScreenEffectd")) {
		static const char* noiseBlendModes[] = {"Overwrite", "Add", "Subtract", "Multiply", "Screen"};
		ImGui::Checkbox("Fullscreen Grayscale (BT709)", &fullScreenGrayscaleEnabled_);
		ImGui::Checkbox("Fullscreen Sepia", &fullScreenSepiaEnabled_);
		ImGui::SliderFloat("Vignette Strength", &vignetteStrength_, 0.0f, 1.0f);
		ImGui::Checkbox("Random Noise (Monochrome)", &randomNoiseEnabled_);
		ImGui::SliderFloat("Random Noise Scale", &randomNoiseScale_, 1.0f, 4096.0f);
		ImGui::Combo("Random Noise Blend", &randomNoiseBlendMode_, noiseBlendModes, IM_ARRAYSIZE(noiseBlendModes));
	}
	ImGui::End();
	if (ImGui::Begin("Portal")) {
		ImGui::Text("Portal A object transform");
		ImGui::DragFloat3("Scale##PortalAScale", &portalATransform_.scale.x, 0.01f, 0.01f, 100.0f);
		ImGui::DragFloat3("Rotate##PortalARotate", &portalATransform_.rotate.x, 0.01f);
		ImGui::DragFloat3("Translate##PortalATranslate", &portalATransform_.translate.x, 0.01f);
		ImGui::Separator();
		ImGui::Text("Portal B object transform");
		ImGui::DragFloat3("Scale##PortalBScale", &portalBTransform_.scale.x, 0.01f, 0.01f, 100.0f);
		ImGui::DragFloat3("Rotate##PortalBRotate", &portalBTransform_.rotate.x, 0.01f);
		ImGui::DragFloat3("Translate##PortalBTranslate", &portalBTransform_.translate.x, 0.01f);
		ImGui::Separator();
		ImGui::Text("Portal A render texture camera transform");
		ImGui::TextDisabled("(Portal B transform based + local adjustment)");
		ImGui::DragFloat3("Adjust Rotate##PortalTextureCameraAAdjustRotate", &portalTextureCameraAAdjust_.rotate.x, 0.01f);
		ImGui::DragFloat3("Adjust Translate##PortalTextureCameraAAdjustTranslate", &portalTextureCameraAAdjust_.translate.x, 0.01f);
		ImGui::InputFloat3("Result Rotate##PortalTextureCameraARotate", &portalTextureCameraATransform_.rotate.x, "%.3f", ImGuiInputTextFlags_ReadOnly);
		ImGui::InputFloat3("Result Translate##PortalTextureCameraATranslate", &portalTextureCameraATransform_.translate.x, "%.3f", ImGuiInputTextFlags_ReadOnly);
		ImGui::Separator();
		ImGui::Text("Portal B render texture camera transform");
		ImGui::TextDisabled("(Portal A transform based + local adjustment)");
		ImGui::DragFloat3("Adjust Rotate##PortalTextureCameraBAdjustRotate", &portalTextureCameraBAdjust_.rotate.x, 0.01f);
		ImGui::DragFloat3("Adjust Translate##PortalTextureCameraBAdjustTranslate", &portalTextureCameraBAdjust_.translate.x, 0.01f);
		ImGui::InputFloat3("Result Rotate##PortalTextureCameraBRotate", &portalTextureCameraBTransform_.rotate.x, "%.3f", ImGuiInputTextFlags_ReadOnly);
		ImGui::InputFloat3("Result Translate##PortalTextureCameraBTranslate", &portalTextureCameraBTransform_.translate.x, "%.3f", ImGuiInputTextFlags_ReadOnly);
	}
	ImGui::End();


#endif // USE_IMGUI
	if (useDebugCamera_) {
		debugCamera_->Update();
		camera_->SetViewProjectionMatrix(debugCamera_->GetViewMatrix(), debugCamera_->GetProjectionMatrix());
	} else {
		camera_->SetTransform(cameraTransform_);
		camera_->Update();
	}
	/*portalObjectCamera_->SetTransform(portalObjectCameraTransform_);
	portalObjectCamera_->Update();*/
	portalTextureCameraATransform_ = MakePortalAnchoredTextureCameraTransform(portalBTransform_, portalTextureCameraAAdjust_);
	portalTextureCameraA_->SetTransform(portalTextureCameraATransform_);
	portalTextureCameraA_->Update();
	portalTextureCameraBTransform_ = MakePortalAnchoredTextureCameraTransform(portalATransform_, portalTextureCameraBAdjust_);
	portalTextureCameraB_->SetTransform(portalTextureCameraBTransform_);
	portalTextureCameraB_->Update();
	portalMeshA_->SetTransform(portalATransform_);
	portalMeshA_->SetTextureCamera(portalTextureCameraA_.get());

	portalMeshB_->SetTransform(portalBTransform_);
	portalMeshB_->SetTextureCamera(portalTextureCameraB_.get());

	ParticleManager::GetInstance()->Update(camera_.get());
	if (sampleParticleEmitter_) {
		sampleParticleEmitter_->Update(particleTransform_);
	}
	Object3dCommon::GetInstance()->SetDirectionalLight(directionalLight_);
	Object3dCommon::GetInstance()->SetPointLights(pointLights_.data(), activePointLightCount_);
	Object3dCommon::GetInstance()->SetSpotLights(spotLights_.data(), activeSpotLightCount_);
	Object3dCommon::GetInstance()->SetAreaLights(areaLights_.data(), activeAreaLightCount_);
	Object3dCommon::GetInstance()->SetFullScreenGrayscaleEnabled(fullScreenGrayscaleEnabled_);
	Object3dCommon::GetInstance()->SetFullScreenSepiaEnabled(fullScreenSepiaEnabled_);
	Object3dCommon::GetInstance()->GetDxCommon()->SetVignetteStrength(vignetteStrength_);
	Object3dCommon::GetInstance()->SetVignetteStrength(vignetteStrength_);
	Object3dCommon::GetInstance()->SetRandomNoiseEnabled(randomNoiseEnabled_);
	Object3dCommon::GetInstance()->SetRandomNoiseScale(randomNoiseScale_);
	Object3dCommon::GetInstance()->SetRandomNoiseBlendMode(randomNoiseBlendMode_);

	/*uvBallObj_->SetTransform(uvBallTransform_);*/
	/*planeGltf_->SetTransform(planeGTransform_);*/
	/*animatedCubeObj_->SetTransform(animatedCubeTransform_);*/
	/*humanObj_->SetTransform(humanTransform_);*/
	/*ringPrimitive_->SetTransform(ringTransform_);*/
	/*ringPrimitive_->SetColor({1.0f, 0.85f, 0.2f, 1.0f});*/
	
	spherePrimitive_->Update();
	uvBallObj_->Update();
	fieldObj_->Update();
	planeGltf_->Update();
	animatedCubeObj_->Update();
	humanObj_->Update();
	ringUvRotation_ -= 0.05f;


	uvSprite->Update();
	if (overlayCameraSprite_) {
		overlayCameraSprite_->Update();
	}

	Object3dCommon::GetInstance()->SetDefaultCamera(camera_.get());

	float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
	if (humanSkeleton_ && !humanAnimationClips_.empty()) {
		const Animation::AnimationData& currentAnimation = humanAnimationClips_[currentHumanAnimationIndex_];
		humanAnimationTime_ = Animation::AdvanceTime(currentAnimation, humanAnimationTime_, deltaTime, true);
		humanSkeleton_->ApplyAnimation(currentAnimation, humanAnimationTime_);
		humanSkeleton_->Update();
		if (!humanSkinCluster_.mappedPalette.empty()) {
			UpdateSkinCluster(humanSkinCluster_, *humanSkeleton_);
		}
		Matrix4x4 humanWorld = humanObj_->GetWorldMatrix();
		humanSkeleton_->SetObjectMatrix(humanWorld);
	}
}
void SampleScene::Draw() {

	Object3dCommon::GetInstance()->BeginShadowMapPass();
	Object3dCommon::GetInstance()->DrawCommonShadow();
	uvBallObj_->Draw();
	planeGltf_->Draw();
	fieldObj_->Draw();
	animatedCubeObj_->Draw();
	Object3dCommon::GetInstance()->EndShadowMapPass();

	// ポータルテクスチャ用に別カメラ視点をオフスクリーン描画
	portalTextureCameraA_->SetTransform(portalTextureCameraATransform_);
	portalTextureCameraA_->Update();
	portalTextureCameraB_->SetTransform(portalTextureCameraBTransform_);
	portalTextureCameraB_->Update();

	portalRenderTextureA_.BeginRender();
	assert(Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList() != nullptr);
	Object3dCommon::GetInstance()->SetDefaultCamera(portalTextureCameraB_.get());
	DrawSceneGeometryForPortalTexture(portalTextureCameraB_.get());
	portalRenderTextureA_.TransitionToShaderResource();
	Object3dCommon::GetInstance()->GetDxCommon()->ExecuteCommandListAndWait();
	portalRenderTextureB_.BeginRender();
	Object3dCommon::GetInstance()->SetDefaultCamera(portalTextureCameraA_.get());
	DrawSceneGeometryForPortalTexture(portalTextureCameraA_.get());
	portalRenderTextureB_.TransitionToShaderResource();

	// ポータル描画で更新した定数バッファをこの時点で確定させる
	Object3dCommon::GetInstance()->GetDxCommon()->ExecuteCommandListAndWait();

	Object3dCommon::GetInstance()->GetDxCommon()->SetMainRenderTarget();
	Object3dCommon::GetInstance()->SetDefaultCamera(camera_.get());
	portalMeshA_->Update();
	portalMeshB_->Update();
	DrawSceneGeometry(camera_.get(), true);
	SpriteCommon::GetInstance()->DrawCommon();
	uvSprite->Draw();
	if (overlayCameraSprite_) {
		/*overlayCameraSprite_->Draw();*/
	}
}

void SampleScene::SetSceneCameraForDraw(Camera* camera) {
	uvBallObj_->SetCamera(camera);
	fieldObj_->SetCamera(camera);
	planeGltf_->SetCamera(camera);
	animatedCubeObj_->SetCamera(camera);
	humanObj_->SetCamera(camera);
	spherePrimitive_->SetCamera(camera);
}

void SampleScene::UpdateSceneCameraMatricesForDraw() {
	uvBallObj_->UpdateCameraMatrices();
	fieldObj_->UpdateCameraMatrices();
	planeGltf_->UpdateCameraMatrices();
	animatedCubeObj_->UpdateCameraMatrices();
	humanObj_->UpdateCameraMatrices();
	spherePrimitive_->UpdateCameraMatrices();
}

void SampleScene::DrawSceneGeometryForPortalTexture(Camera* camera) {
	Object3dCommon::GetInstance()->SetDefaultCamera(camera);
	uvBallObj_->SetCamera(camera);
	fieldObj_->SetCamera(camera);
	planeGltf_->SetCamera(camera);
	animatedCubeObj_->SetCamera(camera);
	humanObj_->SetCamera(camera);
	spherePrimitive_->SetCamera(camera);
	ParticleManager::GetInstance()->SetCamera(camera);
	UpdateSceneCameraMatricesForDraw();

	Object3dCommon::GetInstance()->DrawCommon();
	uvBallObj_->Draw();
	planeGltf_->Draw();
	fieldObj_->Draw();
	animatedCubeObj_->Draw();
	spherePrimitive_->Draw();
	if (sampleParticleEmitter_) {
		sampleParticleEmitter_->Draw();
	}

	Object3dCommon::GetInstance()->DrawCommonSkinningToon();
	humanObj_->Draw();
	Object3dCommon::GetInstance()->DrawCommonWireframeNoDepth();
}

void SampleScene::DrawSceneGeometry(Camera* camera, bool drawPortals) {
	SetSceneCameraForDraw(camera);
	ParticleManager::GetInstance()->SetCamera(camera);
	UpdateSceneCameraMatricesForDraw();
	Object3dCommon::GetInstance()->DrawCommon();
	uvBallObj_->Draw();
	planeGltf_->Draw();
	fieldObj_->Draw();
	animatedCubeObj_->Draw();
	spherePrimitive_->Draw();
	if (sampleParticleEmitter_) {
		sampleParticleEmitter_->Draw();
	}
	if (drawPortals) {
		Object3dCommon::GetInstance()->DrawCommonPortal();
		portalMeshA_->Draw();
		portalMeshB_->Draw();
	}

	Object3dCommon::GetInstance()->DrawCommonSkinningToon();
	humanObj_->Draw();
	Object3dCommon::GetInstance()->DrawCommonWireframeNoDepth();
}

void SampleScene::Finalize() {}
