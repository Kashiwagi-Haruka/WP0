#include "PlayerFallAttack.h"
#include "Object3d/Object3dCommon.h"
void PlayerFallAttack::Initialize(){ 
	fallingEffectObject_ = std::make_unique<Object3d>(); 
	fallingEffectObject_->Initialize();
	fallingEffectObject_->SetModel("FallingEffect");
}

void PlayerFallAttack::Update(Camera* camera,Transform playerT){
	fallingEffectTransform_.scale = playerT.scale;
	fallingEffectTransform_.translate = playerT.translate;
	fallingEffectTransform_.translate.y -= 1.0f;
	fallingEffectTransform_.rotate.y += 0.2f;
	fallingEffectObject_->SetCamera(camera);
	fallingEffectObject_->SetTransform(fallingEffectTransform_);
	fallingEffectObject_->Update();
}

void PlayerFallAttack::Draw(){
	Object3dCommon::GetInstance()->SetBlendMode(BlendMode::kBlendModeAdd);
	fallingEffectObject_->Draw();
	Object3dCommon::GetInstance()->SetBlendMode(BlendMode::kBlendModeAlpha);
}