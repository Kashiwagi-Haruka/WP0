#include "RaySprite.h"
#include "TextureManager.h"
#include "Sprite/SpriteCommon.h"
#include"WinApp.h"
#include"GameObject/KeyBindConfig.h"
#include"GameObject/TimeCard/TimeCardWatch.h"

RaySprite::RaySprite()
{
	handHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/hand.png");
	grabHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/grabHand.png");
	portalHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/mouseUI.png");

	sprite_ = std::make_unique<Sprite>();

}

void RaySprite::Initialize()
{
	sprite_->Initialize(handHandle_);
	sprite_->SetAnchorPoint({ 0.5f,0.5f });
	sprite_->SetRotation(0.0f);
	sprite_->SetScale({ 128.0f,128.0f });
	sprite_->SetPosition({WinApp::kClientWidth*0.5f,WinApp::kClientHeight*0.5f});
	sprite_->SetColor({1.0f,1.0f,1.0f,1.0f});
	sprite_->Update();
}

void RaySprite::Update()
{
	auto* playerCommand = PlayerCommand::GetInstance();

	if (playerCommand->Interact()) {
		// インタラクトの処理
		SetTexture(RaySprite::GRAB);
	} else if(TimeCardWatch::GetCanMakePortal()){
		SetTexture(RaySprite::PORTAL);
	} else {
		SetTexture(RaySprite::HAND);
	}
	sprite_->Update();
}

void RaySprite::Draw()
{
	SpriteCommon::GetInstance()->DrawCommon();
	sprite_->Draw();
}

void RaySprite::SetTexture(const TextureUI num)
{
	switch (num)
	{
	case TextureUI::PORTAL:
		sprite_->SetTextureHandle(portalHandle_);
		break;
	case TextureUI::HAND:
		sprite_->SetTextureHandle(handHandle_);
		break;
	case TextureUI::GRAB:
		sprite_->SetTextureHandle(grabHandle_);
		break;
	default:
		break;
	}
}
