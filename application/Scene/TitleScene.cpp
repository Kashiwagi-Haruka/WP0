#include "TitleScene.h"
#include "Input.h"
#include "SceneManager.h"
#include "Sprite/SpriteCommon.h"
#include "TextureManager.h"
#include <imgui.h>
TitleScene::TitleScene() {
	
	BGMData = Audio::GetInstance()->SoundLoadFile("Resources/audio/BGM/Rendez-vous_2.mp3");
	Audio::GetInstance()->SetSoundVolume(&BGMData, 0.3f);
	transition = std::make_unique<SceneTransition>();
}

void TitleScene::Finalize() { Audio::GetInstance()->SoundUnload(&BGMData); }

void TitleScene::Initialize() {

	isBGMPlaying = false;
	isTransitionIn = true;
	isTransitionOut = false;
	transition->Initialize(false);
}

void TitleScene::Update() {
	if (!isBGMPlaying) {
		Audio::GetInstance()->SoundPlayWave(BGMData, true);
		isBGMPlaying = true;
	}

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
			SceneManager::GetInstance()->ChangeScene("Game");
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

	if (isTransitionIn || isTransitionOut) {
		transition->Draw();
	}

}