#include "EnemyHitEffect.h"
#include "GameBase.h"
#include "Model/ModelManager.h"
#include <numbers>

void EnemyHitEffect::Initialize() {

	ModelManager::GetInstance()->LoadModel("Resources/3d","HitEffect");
	
	hitEffect_ = std::make_unique<Object3d>();
	hitEffect_->Initialize();
	hitEffect_->SetModel("HitEffect");
	hitEffect_->SetEnableLighting(false);
	hitEffect_->SetCamera(camera_);
	hitTransform_ = {
	    .scale{1, 1, 1},
        .rotate{0, 0, 0},
        .translate{0, 0, 0}
    };
	hitPrimitive_ = std::make_unique<Primitive>();
	hitPrimitive_->Initialize(Primitive::Plane, "Resources/3d/Circle.png");
	hitPrimitiveTransform_ = {
	    .scale{3.0f, 0.001f, 1.0f},
        .rotate{0, 0, 0},
        .translate{0, 0, 0}
    };
	hitPrimitive_->SetTransform(hitPrimitiveTransform_);
	hitPrimitive_->SetEnableLighting(false);
	hitPrimitive_->SetCamera(camera_);


	hitPrimitiveInner_ = std::make_unique<Primitive>();
	hitPrimitiveInnerTransform_ = {
	    .scale{0.001f, 3.0f, 1.0f},
        .rotate{0,    0,    0   },
        .translate{0,    0,    0   }
    };
	hitPrimitiveInner_->Initialize(Primitive::Plane, "Resources/3d/Circle.png");
	hitPrimitiveInner_->SetEnableLighting(false);
	hitPrimitiveInner_->SetTransform(hitPrimitiveInnerTransform_);

	enemyPosition_ = {0.0f, 0.0f, 0.0f};
}

void EnemyHitEffect::Activate(const Vector3& position) {
	isActive_ = true;
	activeTimer_ = activeDuration_;
	enemyPosition_ = position;
}

void EnemyHitEffect::Update() {
	if (!isActive_) {
		return;
	}

	activeTimer_ -= 1.0f / 60.0f;
	if (activeTimer_ <= 0.0f) {
		isActive_ = false;
		return;
	}

	hitTransform_.translate = enemyPosition_;
	hitTransform_.rotate.y = std::numbers::pi_v<float>;
	hitPrimitiveTransform_.translate = enemyPosition_;
	hitPrimitiveTransform_.rotate.y = std::numbers::pi_v<float>;
	hitPrimitiveInnerTransform_.translate = enemyPosition_;
	hitPrimitiveInnerTransform_.rotate.y = std::numbers::pi_v<float>;

	Matrix4x4 c = camera_->GetWorldMatrix();
	c.m[3][0] = c.m[3][1] = c.m[3][2] = 0;
	Matrix4x4 world = Function::Multiply(c, Function::MakeAffineMatrix(hitTransform_.scale, hitTransform_.rotate, hitTransform_.translate));
	Matrix4x4 primitiveWorld = Function::Multiply(c, Function::MakeAffineMatrix(hitPrimitiveTransform_.scale, hitPrimitiveTransform_.rotate, hitPrimitiveTransform_.translate));
	Matrix4x4 primitiveInnerWorld = Function::Multiply(c, Function::MakeAffineMatrix(hitPrimitiveInnerTransform_.scale, hitPrimitiveInnerTransform_.rotate, hitPrimitiveInnerTransform_.translate));
	
	hitEffect_->SetCamera(camera_);
	hitEffect_->SetWorldMatrix(world);
	hitEffect_->Update();
	hitPrimitive_->SetCamera(camera_);
	hitPrimitive_->SetWorldMatrix(primitiveWorld);
	hitPrimitive_->Update();
	hitPrimitiveInner_->SetCamera(camera_);
	hitPrimitiveInner_->SetWorldMatrix(primitiveInnerWorld);
	hitPrimitiveInner_->Update();
	
}

void EnemyHitEffect::Draw() {
	if (!isActive_) {
		return;
	}

	hitEffect_->Draw();
	//hitPrimitive_->Draw();
	//hitPrimitiveInner_->Draw();
}