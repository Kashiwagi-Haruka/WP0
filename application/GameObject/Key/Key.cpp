#include "Key.h"
#include <GameObject/YoshidaMath/YoshidaMath.h>
#include <Model/ModelManager.h>
#include <GameObject/KeyBindConfig.h>
#include <Function.h>

Key::Key()
{
	obj_ = std::make_unique<Object3d>();

	// モデルをセット
	ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/gentleman", "gentleman");
	obj_->SetModel("gentleman");

#ifdef _DEBUG
	primitive_ = std::make_unique<Primitive>();
#endif
}

void Key::Initialize()
{
	worldTransform_ = {
		.scale{1.0f, 1.0f, 1.0f},
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

void Key::Update()
{
	obj_->SetTransform(worldTransform_);
	obj_->Update();

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

void Key::Draw()
{
	obj_->Draw();
#ifdef _DEBUG
	primitive_->Draw();
#endif
}

void Key::SetPlayerCamera(PlayerCamera* camera)
{

	playerCamera_ = camera;
}

void Key::SetCamera(Camera* camera)
{
	obj_->SetCamera(camera);

#ifdef _DEBUG
	primitive_->SetCamera(camera);
#endif
}

void Key::SetModel(const std::string& filePath)
{
	obj_->SetModel(filePath);
}

void Key::CheckCollision()
{
	//keyとrayの当たり判定
	if (OnCollisionRay()) {
		if (PlayerCommand::GetInstance()->Interact()) {
			isGrabbed_ = true;
		}

	  
	}

	if (isGrabbed_ && !PlayerCommand::GetInstance()->Interact()) {
		isGrabbed_ = false;
	}

	if (isGrabbed_) {
		// カーソルに追従させて持ち上げる処理
		Vector3 origin = playerCamera_->GetTransform().translate;
		origin.y -= 1.0f;
		worldTransform_.translate = origin + (Function::Normalize(playerCamera_->GetRay().diff));
		worldTransform_.translate.y = (std::max)(worldTransform_.translate.y, 0.0f);
	}
}

bool Key::OnCollisionRay()
{
	return playerCamera_->OnCollisionRay(localAABB_, collisionTransform_.translate);
}

void Key::OnCollision(Collider* collider)
{
}

Vector3 Key::GetWorldPosition() const
{
	return Vector3();
}
