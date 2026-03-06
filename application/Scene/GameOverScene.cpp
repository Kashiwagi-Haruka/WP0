#include "GameOverScene.h"
#include "Input.h"
#include "SceneManager.h"
#include "Sprite/SpriteCommon.h"
#include "TextureManager.h"
GameOverScene::GameOverScene() {
	transition = std::make_unique<SceneTransition>();
	BGM_ = Audio::GetInstance()->SoundLoadFile("Resources/audio/BGM/おそろい.mp3");
}

void GameOverScene::Finalize() { Audio::GetInstance()->SoundUnload(&BGM_); }

void GameOverScene::Initialize() {
	
	transition->Initialize(false);
	isTransitionIn = true;
	isTransitionOut = false;
	isBGMPlaying = false;
}
void GameOverScene::Update() {

	if (!isBGMPlaying) {
		Audio::GetInstance()->SoundPlayWave(BGM_, true);
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
			SceneManager::GetInstance()->ChangeScene("Title");
		}
	}
#ifdef USE_IMGUI
	ImGui::Begin("resultScene");
	ImGui::End();
#endif // USE_IMGUI
}

void GameOverScene::Draw() {
	SpriteCommon::GetInstance()->DrawCommon();
	if (isTransitionIn || isTransitionOut) {
		transition->Draw();
	}
}