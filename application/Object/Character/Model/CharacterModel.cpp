#include "CharacterModel.h"
#include "Model/ModelManager.h"
void CharacterModel::LoadModel() { 
	ModelManager::GetInstance()->LoadGltfModel("Resources/3d", "sizuku"); 
	ModelManager::GetInstance()->LoadModel("Resources/3d", "Enemy");
	ModelManager::GetInstance()->LoadGltfModel("Resources/3d", "WaterBoss");
}