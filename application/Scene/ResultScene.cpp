#include "ResultScene.h"
#include "Input.h"
#include "SceneManager.h"
#include "Sprite/SpriteCommon.h"
#include "TextureManager.h"
#include <algorithm>

ResultScene::ResultScene() {

	BGM_ = Audio::GetInstance()->SoundLoadFile("Resources/audio/BGM/アンドロイドの涙.mp3");

	transition = std::make_unique<SceneTransition>();
}

void ResultScene::Finalize() { Audio::GetInstance()->SoundUnload(&BGM_); }

void ResultScene::Initialize() {
	isSceneEnd_ = false;

	Input::GetInstance()->SetIsCursorStability(false);
	Input::GetInstance()->SetIsCursorVisible(true);
	transition->Initialize(false);
	isTransitionIn = true;
	isTransitionOut = false;
	isBGMPlaying = false;
}
void ResultScene::Update() {
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

void ResultScene::Draw() {
	SpriteCommon::GetInstance()->DrawCommon();

	if (isTransitionIn || isTransitionOut) {
		transition->Draw();
	}
}