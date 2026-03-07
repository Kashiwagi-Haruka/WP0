#include "CharacterDisplaySkyDome.h"
#include "Model/ModelManager.h"
void CharacterDisplaySkyDome::Initialize(Camera* camera) { 
	ModelManager::GetInstance()->LoadModel("Resources/3d/CharacterDisplay", "CharacterDisplaySkydome");
	object_ = std::make_unique<Object3d>(); 
	object_->Initialize();
	object_->SetModel("CharacterDisplaySkydome");
	object_->SetCamera(camera);
	transform_ = {
	    .scale{100, 100, 100},
        .rotate{0,   0,   0  },
        .translate{0,   0,   0  }
    };
	object_->SetTransform(transform_);
	object_->SetEnableLighting(false);
}

void CharacterDisplaySkyDome::Update() { object_->Update(); }

void CharacterDisplaySkyDome::Draw() { object_->Draw(); }