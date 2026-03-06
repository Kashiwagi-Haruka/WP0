#include "GameScene.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include "ParticleManager.h"
#include "SceneManager.h"
#include "Sprite/SpriteCommon.h"
#include "TextureManager.h"
#include <algorithm>
#include <cmath>
#include <numbers>

GameScene::GameScene() {
	sceneTransition = std::make_unique<SceneTransition>();

	pause = std::make_unique<Pause>();
	BGMData = Audio::GetInstance()->SoundLoadFile("Resources/audio/BGM/Tailshaft.mp3");
	Audio::GetInstance()->SetSoundVolume(&BGMData, 0.3f);
}

GameScene::~GameScene() {}

void GameScene::Finalize() {

	Audio::GetInstance()->SoundUnload(&BGMData);

}

void GameScene::Initialize() {

	sceneEndClear = false;
	sceneEndOver = false;
	isBGMPlaying = false;
	
	sceneTransition->Initialize(false);
	isTransitionIn = true;
	isTransitionOut = false;
	nextSceneName.clear();
	activePointLightCount_ = 3;
	pointLights_[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
	pointLights_[0].position = {-75.0f, 10.0f, -75.0f};
	pointLights_[0].intensity = 1.0f;
	pointLights_[0].radius = 10.0f;
	pointLights_[0].decay = 0.7f;
	pointLights_[1].color = {1.0f, 0.9f, 0.9f, 1.0f};
	pointLights_[1].position = {75.0f, 5.0f, 75.0f};
	pointLights_[1].intensity = 0.0f;
	pointLights_[1].radius = 10.0f;
	pointLights_[1].decay = 0.7f;
	pointLights_[2].color = {0.4f, 0.4f, 1.0f, 1.0f};
	pointLights_[2].position = {-75.0f, 5.0f, 75.0f};
	pointLights_[2].intensity = 1.0f;
	pointLights_[2].radius = 5.0f;
	pointLights_[2].decay = 0.7f;

	directionalLight_.color = {76.0f/255.0f, 96.0f/255.0f, 178/255.0f, 1.0f};
	directionalLight_.direction = {0.0f, -1.0f, 0.5f};
	directionalLight_.intensity = 1.0f;

	activeSpotLightCount_ = 1;
	spotLights_[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
	spotLights_[0].position = {-50.0f, 5.0f, -50.0f};
	spotLights_[0].direction = {0.0f, 1.0f, 0.0f};
	spotLights_[0].intensity = 0.0f;
	spotLights_[0].distance = 7.0f;
	spotLights_[0].decay = 2.0f;
	spotLights_[0].cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
	spotLights_[0].cosFalloffStart = std::cos(std::numbers::pi_v<float> / 4.0f);
	pause->Initialize();
}

void GameScene::DebugImGui() {

#ifdef USE_IMGUI
	if (ImGui::Begin("SampleLight")) {
		if (ImGui::TreeNode("DirectionalLight")) {
			ImGui::ColorEdit4("LightColor", &directionalLight_.color.x);
			ImGui::DragFloat3("LightDirection", &directionalLight_.direction.x, 0.1f, -1.0f, 1.0f);
			ImGui::DragFloat("LightIntensity", &directionalLight_.intensity, 0.1f, 0.0f, 10.0f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("PointLight")) {

			for (uint32_t index = 0; index < activePointLightCount_; ++index) {
				ImGui::PushID(static_cast<int>(index));
				if (ImGui::TreeNode("PointLight")) {
					ImGui::ColorEdit4("PointLightColor", &pointLights_[index].color.x);
					ImGui::DragFloat("PointLightIntensity", &pointLights_[index].intensity, 0.1f);
					ImGui::DragFloat3("PointLightPosition", &pointLights_[index].position.x, 0.1f);
					ImGui::DragFloat("PointLightRadius", &pointLights_[index].radius, 0.1f);
					ImGui::DragFloat("PointLightDecay", &pointLights_[index].decay, 0.1f);
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("SpotLight")) {
			ImGui::ColorEdit4("SpotLightColor", &spotLights_[0].color.x);
			ImGui::DragFloat("SpotLightIntensity", &spotLights_[0].intensity, 0.1f);
			ImGui::DragFloat3("SpotLightPosition", &spotLights_[0].position.x, 0.1f);
			ImGui::DragFloat3("SpotLightDirection", &spotLights_[0].direction.x, 0.1f);
			ImGui::DragFloat("SpotLightDistance", &spotLights_[0].distance, 0.1f);
			ImGui::DragFloat("SpotLightDecay", &spotLights_[0].decay, 0.1f);
			ImGui::DragFloat("SpotLightCosAngle", &spotLights_[0].cosAngle, 0.1f, 0.0f, 1.0f);
			ImGui::DragFloat("SpotLightCosFalloffStart", &spotLights_[0].cosFalloffStart, 0.1f, 0.0f, 1.0f);
			ImGui::TreePop();
		}
	}
	ImGui::End();
	

#endif // USE_IMGUI
}


void GameScene::Update() {
	if (!isBGMPlaying) {
		Audio::GetInstance()->SoundPlayWave(BGMData, true);
		isBGMPlaying = true;
	}

	DebugImGui();
	pause->Update(isPause);
	Pause::Action pauseAction = pause->ConsumeAction();


	Object3dCommon::GetInstance()->SetDirectionalLight(directionalLight_);
	Object3dCommon::GetInstance()->SetPointLights(pointLights_.data(), activePointLightCount_);
	Object3dCommon::GetInstance()->SetSpotLights(spotLights_.data(), activeSpotLightCount_);


	if (isTransitionIn || isTransitionOut) {
		sceneTransition->Update();
		if (sceneTransition->IsEnd() && isTransitionIn) {
			isTransitionIn = false;
		}
		if (sceneTransition->IsEnd() && isTransitionOut) {
			SceneManager::GetInstance()->ChangeScene(nextSceneName);
		}
	}
}

void GameScene::Draw() {
	
	pause->Draw();
	if (isTransitionIn || isTransitionOut) {

		sceneTransition->Draw();
	}
}