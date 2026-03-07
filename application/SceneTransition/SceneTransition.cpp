#include "SceneTransition.h"
#include "GameBase.h"
#include "TextureManager.h"
SceneTransition::SceneTransition() {

	fadeSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/transition.png");
	fadeSPData.sprite = std::make_unique<Sprite>();
	fadeSPData.sprite->Initialize(fadeSPData.handle);
}
SceneTransition::~SceneTransition() {}

void SceneTransition::Initialize(bool isIn) {

	isIn_ = isIn;
	if (isIn_) {
		fadeSPData.size = {0, 720};
		fadeSPData.translate = {1280, 0};
	} else {
		fadeSPData.size = {1800, 720};
		fadeSPData.translate = {-520, 0};
	}

	fadeSPData.sprite->SetScale(fadeSPData.size);
	fadeSPData.sprite->SetPosition(fadeSPData.translate);
	fadeSPData.sprite->Update();

	isEnd = false;
}
void SceneTransition::Update() {

	if (isIn_) {
		fadeSPData.size.x += 40.0f;
		fadeSPData.translate.x -= 40.0f;
		if (fadeSPData.size.x >= 1800.0f) {
			fadeSPData.size.x = 1800.0f;
			isEnd = true;
		}
	} else {
		fadeSPData.size.x -= 40.0f;
		fadeSPData.translate.x += 40.0f;
		if (fadeSPData.size.x <= 0.0f) {
			fadeSPData.size.x = 0.0f;
			isEnd = true;
		}
	}
	fadeSPData.sprite->SetScale(fadeSPData.size);
	fadeSPData.sprite->SetPosition(fadeSPData.translate);
	fadeSPData.sprite->Update();
}
void SceneTransition::Draw() { fadeSPData.sprite->Draw(); }