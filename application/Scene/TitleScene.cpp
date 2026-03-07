#include "TitleScene.h"
#include "Input.h"
#include "SceneManager.h"
#include "Sprite/SpriteCommon.h"
#include "TextureManager.h"
#include <imgui.h>
TitleScene::TitleScene() {
	BGSP_.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/title.png");
	BGSP_.sprite = std::make_unique<Sprite>();
	BGSP_.sprite->Initialize(BGSP_.handle);
	logoSP_.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/logo.png");
	logoSP_.sprite = std::make_unique<Sprite>();
	logoSP_.sprite->Initialize(logoSP_.handle);
	ruleSP_.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/rule.png");
	ruleSP_.sprite = std::make_unique<Sprite>();
	ruleSP_.sprite->Initialize(ruleSP_.handle);
	BGMData = Audio::GetInstance()->SoundLoadFile("Resources/audio/BGM/Rendez-vous_2.mp3");
	Audio::GetInstance()->SetSoundVolume(&BGMData, 0.3f);
	transition = std::make_unique<SceneTransition>();
	characterModel_.LoadModel();
}

void TitleScene::Finalize() { Audio::GetInstance()->SoundUnload(&BGMData); }

void TitleScene::Initialize() {

	BGSP_.sprite->SetAnchorPoint({0.5f, 0.5f});
	BGSP_.size = {2000, 2000};
	BGSP_.translate = {640, 0};
	BGSP_.sprite->SetScale(BGSP_.size);
	BGSP_.sprite->SetPosition(BGSP_.translate);
	BGSP_.sprite->Update();
	logoSP_.sprite->SetAnchorPoint({0.5f, 0.5f});
	logoSP_.sprite->SetPosition({640, 300});
	logoSP_.sprite->SetScale({500, 500});
	logoSP_.sprite->Update();
	ruleSP_.sprite->SetAnchorPoint({0.5f, 0.5f});
	ruleSP_.sprite->SetScale({640, 360});
	ruleSP_.sprite->SetPosition({640, 300});
	ruleSP_.sprite->Update();
	pressSpaceHandle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/SPACE.png");

	pressSpaceSprite = std::make_unique<Sprite>();
	pressSpaceSprite->Initialize(pressSpaceHandle);
	pressSpaceSprite->SetAnchorPoint({0.5f, 0.5f});
	pressSpaceSprite->SetTextureRange({0, 0}, {768, 768});
	pressSpaceSprite->SetScale(pressSpaceSize);
	pressSpaceSprite->SetPosition(pressSpacePos);
	pressSpaceSprite->Update();
	isBGMPlaying = false;
	isTransitionIn = true;
	isTransitionOut = false;
	transition->Initialize(false);
	Input::GetInstance()->SetIsCursorStability(false);
	Input::GetInstance()->SetIsCursorVisible(true);
}

void TitleScene::Update() {
	if (!isBGMPlaying) {
		Audio::GetInstance()->SoundPlayWave(BGMData, true);
		isBGMPlaying = true;
	}

	BGSP_.rotate += 0.01f;

	BGSP_.sprite->SetRotation(BGSP_.rotate);
	BGSP_.sprite->Update();

	if (Input::GetInstance()->TriggerKey(DIK_SPACE) && !isTransitionOut) {
		transition->Initialize(true);
		isTransitionOut = true;
	}

	if (isTransitionIn || isTransitionOut) {
		transition->Update();
		if (transition->IsEnd() && isTransitionIn) {
			isTransitionIn = false;
		}
		if (transition->IsEnd() && isTransitionOut) {
			SceneManager::GetInstance()->ChangeScene("Tutorial");
		}
	}
#ifdef USE_IMGUI
	ImGui::Begin("titleScene");
	ImGui::End();
	if (ImGui::Begin("Scene")) {
		if (ImGui::Button("Sample")) {
			SceneManager::GetInstance()->ChangeScene("Sample");
		}
		if (ImGui::Button("Tutorial")) {
			SceneManager::GetInstance()->ChangeScene("Tutorial");
		}
	}
	ImGui::End();
#endif // USE_IMGUI
}
void TitleScene::Draw() {

	SpriteCommon::GetInstance()->DrawCommon();
	BGSP_.sprite->Draw();
	logoSP_.sprite->Draw();
	pressSpaceSprite->Draw();
	if (isTransitionIn || isTransitionOut) {
		transition->Draw();
	}

	if (isTransitionOut) {
		ruleSP_.sprite->Draw();
	}
}