#include "TutorialUI.h"
#include "Sprite.h"
#include "TextureManager.h"
#include <string>

void TutorialUI::Initialize() {
	tutorialIconHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/tuatorial/Icon.png");
	tutorialProgressBackHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/ExpBackGauge.png");
	tutorialProgressFillHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/ExpGauge.png");
	tutorialProgressLabelHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/tutoriaGauge.png");
	tutorialSkipBackHandle_ = tutorialProgressBackHandle_;
	tutorialSkipFillHandle_ = tutorialProgressFillHandle_;
	tutorialSkipLabelHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/Skip.png");
	for (int i = 0; i < kStepSpriteCount; ++i) {
		const std::string filePath = "Resources/2d/tuatorial/" + std::to_string(i + 1) + ".png";
		tutorialStepHandles_[i] = TextureManager::GetInstance()->GetTextureIndexByfilePath(filePath.c_str());
	}

	tutorialIconSprite_ = std::make_unique<Sprite>();
	tutorialProgressBackSprite_ = std::make_unique<Sprite>();
	tutorialProgressFillSprite_ = std::make_unique<Sprite>();
	tutorialProgressLabelSprite_ = std::make_unique<Sprite>();
	tutorialSkipBackSprite_ = std::make_unique<Sprite>();
	tutorialSkipFillSprite_ = std::make_unique<Sprite>();
	tutorialSkipLabelSprite_ = std::make_unique<Sprite>();
	for (auto& sprite : tutorialStepSprites_) {
		sprite = std::make_unique<Sprite>();
	}

	tutorialIconSprite_->Initialize(tutorialIconHandle_);
	tutorialIconSprite_->SetScale({64.0f, 64.0f});
	tutorialIconSprite_->SetPosition({1280.0f-20.0f, 20.0f});
	tutorialIconSprite_->SetAnchorPoint({1.0f, 0.0f});
	tutorialIconSprite_->Update();

	for (int i = 0; i < kStepSpriteCount; ++i) {
		tutorialStepSprites_[i]->Initialize(tutorialStepHandles_[i]);
		tutorialStepSprites_[i]->SetScale({kStepSpriteWidth, kStepSpriteHeight});
		tutorialStepSprites_[i]->SetPosition({1280.0f-80.0f, 20.0f});
		tutorialStepSprites_[i]->SetAnchorPoint({1.0f, 0.0f});
		tutorialStepSprites_[i]->Update();
	}

	tutorialProgressBackSprite_->Initialize(tutorialProgressBackHandle_);
	tutorialProgressBackSprite_->SetScale({kProgressBarWidth, kProgressBarHeight});
	tutorialProgressBackSprite_->SetPosition({100.0f, 80.0f});
	tutorialProgressBackSprite_->Update();

	tutorialProgressFillSprite_->Initialize(tutorialProgressFillHandle_);
	tutorialProgressFillSprite_->SetScale({kProgressBarWidth, kProgressBarHeight});
	tutorialProgressFillSprite_->SetPosition({100.0f, 80.0f});
	tutorialProgressFillSprite_->Update();

	tutorialProgressLabelSprite_->Initialize(tutorialProgressLabelHandle_);
	tutorialProgressLabelSprite_->SetAnchorPoint({1.0f, 0.5f});
	tutorialProgressLabelSprite_->SetScale({100.0f, 100.0f});
	tutorialProgressLabelSprite_->SetPosition({100.0f, 80.0f + kProgressBarHeight * 0.5f});
	tutorialProgressLabelSprite_->Update();

	tutorialSkipBackSprite_->Initialize(tutorialSkipBackHandle_);
	tutorialSkipBackSprite_->SetScale({kSkipBarWidth, kSkipBarHeight});
	tutorialSkipBackSprite_->SetPosition({100.0f, 110.0f});
	tutorialSkipBackSprite_->Update();

	tutorialSkipFillSprite_->Initialize(tutorialSkipFillHandle_);
	tutorialSkipFillSprite_->SetScale({kSkipBarWidth, kSkipBarHeight});
	tutorialSkipFillSprite_->SetPosition({100.0f, 110.0f});
	tutorialSkipFillSprite_->Update();

	tutorialSkipLabelSprite_->Initialize(tutorialSkipLabelHandle_);
	tutorialSkipLabelSprite_->SetAnchorPoint({1.0f, 0.5f});
	tutorialSkipLabelSprite_->SetScale({80.0f, 80.0f});
	tutorialSkipLabelSprite_->SetPosition({100.0f, 110.0f + kSkipBarHeight * 0.5f});
	tutorialSkipLabelSprite_->Update();
}

void TutorialUI::Update(float tutorialProgress, float skipProgress, int stepIndex, bool showSkip) {
	currentStepIndex_ = stepIndex;
	showSkip_ = showSkip;

	if (tutorialProgressFillSprite_) {
		tutorialProgressFillSprite_->SetScale({kProgressBarWidth * tutorialProgress, kProgressBarHeight});
		tutorialProgressFillSprite_->Update();
	}
	if (tutorialProgressLabelSprite_) {
		tutorialProgressLabelSprite_->Update();
	}
	if (tutorialSkipFillSprite_) {
		tutorialSkipFillSprite_->SetScale({kSkipBarWidth * skipProgress, kSkipBarHeight});
		tutorialSkipFillSprite_->Update();
	}
	if (tutorialSkipLabelSprite_) {
		tutorialSkipLabelSprite_->Update();
	}
}

void TutorialUI::Draw() {
	if (tutorialIconSprite_) {
		tutorialIconSprite_->Draw();
	}
	if (currentStepIndex_ >= 0 && currentStepIndex_ < kStepSpriteCount) {
		if (tutorialStepSprites_[currentStepIndex_]) {
			tutorialStepSprites_[currentStepIndex_]->Draw();
		}
	}
	if (tutorialProgressBackSprite_) {
		tutorialProgressBackSprite_->Draw();
	}
	if (tutorialProgressFillSprite_) {
		tutorialProgressFillSprite_->Draw();
	}
	if (tutorialProgressLabelSprite_) {
		tutorialProgressLabelSprite_->Draw();
	}
	if (showSkip_) {
		if (tutorialSkipBackSprite_) {
			tutorialSkipBackSprite_->Draw();
		}
		if (tutorialSkipFillSprite_) {
			tutorialSkipFillSprite_->Draw();
		}
		if (tutorialSkipLabelSprite_) {
			tutorialSkipLabelSprite_->Draw();
		}
	}
}