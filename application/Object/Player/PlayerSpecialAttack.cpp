#include "PlayerSpecialAttack.h"
#include "GameBase.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include <numbers>
PlayerSpecialAttack::PlayerSpecialAttack() { ModelManager::GetInstance()->LoadModel("Resources/3d","iceFlower"); }



void PlayerSpecialAttack::Initialize() {
	
}
void PlayerSpecialAttack::Update(const Transform& playerTransform) { transform_ = playerTransform; }

void PlayerSpecialAttack::Draw() {
	
}