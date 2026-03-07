#define NOMINMAX
#include "House.h"
#include "Object3d/Object3dCommon.h"
#include "Model/ModelManager.h"
#include "Function.h"
#include <numbers>
#include "Object/House/HouseHP.h"
#include <algorithm>

House::House() {}

void House::Initialize(Camera* camera) {

	ModelManager::GetInstance()->LoadModel("Resources/3d","house");
	ModelManager::GetInstance()->LoadModel("Resources/3d","HPBar");
	ModelManager::GetInstance()->LoadModel("Resources/3d","HPFlame");
	object_ = std::make_unique<Object3d>();

	object_->Initialize();
	object_->SetModel("house");

	object_->SetCamera(camera);
	object_->SetScale({1, 1, 1});
	position_ = {0.0f, 1.5f, 0.0f};
	object_->SetTranslate(position_);

	hpbar_ = std::make_unique<Object3d>();
	hpbar_->SetModel("HPBar");
	hpbar_->Initialize();
	hpbar_->SetCamera(camera);

	hpflame_ = std::make_unique<Object3d>();
	hpflame_->SetModel("HPFlame");
	hpflame_->Initialize();
	hpflame_->SetCamera(camera);

	hpBarT_ = {
	    .scale{1, 1, 1},
        .rotate{0, 0, 0},
        .translate{0, 0, 0}
    };
	hpFlameT_ = {
	    .scale{1, 1, 1},
        .rotate{0, 0, 0},
        .translate{0, 0, 0}
    };
	HouseHP::GetInstance()->SetHP(hp_);
}
void House::Damage(int amount) {
	hp_ = std::max(0, hp_ - amount);
	HouseHP::GetInstance()->SetHP(hp_);
}
void House::Update(Camera* camera) {
	object_->SetCamera(camera);
	object_->SetTranslate(position_);
	object_->Update();

	constexpr float kHpOffsetY = 1.0f;
	constexpr float kHpBarMaxScale = 1.0f;
	constexpr float kHpBarHalfWidth = 0.5f;
	Vector3 hpBasePos = {position_.x, position_.y+kHpOffsetY, position_.z};

	float hpRatio = std::clamp(static_cast<float>(hp_) / HouseHP::GetInstance()->GetMaxHP(), 0.0f, 1.0f);
	hpBarT_.scale = {kHpBarMaxScale * hpRatio, 1.0f, 1.0f};
	hpBarT_.translate = hpBasePos;
	hpFlameT_.translate = hpBasePos;
	Matrix4x4 billboard = camera->GetWorldMatrix();
	billboard.m[3][0] = billboard.m[3][1] = billboard.m[3][2] = 0;

	Matrix4x4 BarW = Function::Multiply(billboard, Function::MakeAffineMatrix(hpBarT_.scale, hpBarT_.rotate, hpBarT_.translate));
	Matrix4x4 FlameW = Function::Multiply(billboard, Function::MakeAffineMatrix(hpFlameT_.scale, hpFlameT_.rotate, hpFlameT_.translate));

	hpflame_->SetCamera(camera);
	hpflame_->SetWorldMatrix(FlameW);
	hpflame_->SetEnableLighting(false);
	hpflame_->Update();

	hpbar_->SetCamera(camera);
	hpbar_->SetWorldMatrix(BarW);
	hpbar_->SetEnableLighting(false);
	hpbar_->Update();
}

void House::Draw() {

	object_->Draw();
	Object3dCommon::GetInstance()->DrawCommonNoCullDepth();
	hpflame_->Draw();
	hpbar_->Draw();
}
