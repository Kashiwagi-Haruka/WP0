#include "Edamame.h"
#include <Model/ModelManager.h>
#include <GameObject/YoshidaMath/YoshidaMath.h>
#include <GameObject/KeyBindConfig.h>
#include <Function.h>
#include <GameObject/YoshidaMath/CollisionManager/Collider.h>
#include"Object3d/Object3dCommon.h"


Edamame::Edamame()
{
	obj_ = std::make_unique<Object3d>();

	// モデルをセット
	ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/edamame_billboard", "edamame_billboard");
	obj_->SetModel("edamame_billboard");

#ifdef _DEBUG
	primitive_ = std::make_unique<Primitive>();
#endif
}

void Edamame::Initialize()
{
	worldTransform_ = {
		.scale{0.5f, 0.5f, 0.5f},
		.rotate{0.0f, 0.0f, 0.0f},
		.translate{4.0f, 0.0f, 0.0f}
	};

	obj_->Initialize();
#ifdef _DEBUG
	primitive_->Initialize(Primitive::Box);
	primitive_->SetColor({ 1.0f,1.0f,1.0f,0.1f });
#endif
	localAABB_ = { .min = { -0.5f,-0.5f,-0.5f},.max = {0.5f,0.5f,0.5f} };
}

void Edamame::Update()
{

	obj_->SetTransform(worldTransform_);
	obj_->UpdateBillboard();

	collisionTransform_ = obj_->GetTransform();

	collisionTransform_.scale = YoshidaMath::GetAABBScale(localAABB_);
	collisionTransform_.rotate = { 0.0f };
	//objectからの相対距離
	collisionTransform_.translate.y += 1.375f;

#ifdef _DEBUG
	primitive_->SetTransform(collisionTransform_);
	primitive_->Update();
#endif
}

void Edamame::Draw()
{
	obj_->Draw();
#ifdef _DEBUG
	primitive_->Draw();
#endif
}

void Edamame::SetPlayerCamera(PlayerCamera* camera)
{
	playerCamera_ = camera;
}

void Edamame::SetCamera(Camera* camera)
{
	obj_->SetCamera(camera);
#ifdef _DEBUG
	primitive_->SetCamera(camera);
#endif
}

void Edamame::SetModel(const std::string& filePath)
{
	obj_->SetModel(filePath);
}

void Edamame::CheckCollision()
{
	//keyとrayの当たり判定
	if (OnCollisionRay()) {
		if (PlayerCommand::GetInstance()->Interact()) {
			// カーソルに追従させて持ち上げる処理
			Vector3 origin = playerCamera_->GetTransform().translate;
			origin.y -= 1.0f;
			worldTransform_.translate = origin + (Function::Normalize(playerCamera_->GetRay().diff));
		}
	}
}

bool Edamame::OnCollisionRay()
{
	return playerCamera_->OnCollisionRay(localAABB_, collisionTransform_.translate);
}
