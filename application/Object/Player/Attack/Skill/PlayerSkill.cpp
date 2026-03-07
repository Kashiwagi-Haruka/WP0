#include "PlayerSkill.h"
#include "GameBase.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include <cmath>
#include <numbers>
PlayerSkill::PlayerSkill() {
	state = up;
	isSkillEnd = true;
	isSpecialEnd_ = true;
	ModelManager::GetInstance()->LoadModel("Resources/3d", "iceFlower");
}
void PlayerSkill::Initialize() {
	debugBox_ = std::make_unique<Object3d>();
	debugBox_->Initialize();
	debugBox_->SetCamera(camera_);
	debugBox_->SetModel("debugBox");
	debugDamageBox1_ = std::make_unique<Object3d>();
	debugDamageBox1_->Initialize();
	debugDamageBox1_->SetCamera(camera_);
	debugDamageBox1_->SetModel("debugBox");
	debugDamageBox2_ = std::make_unique<Object3d>();
	debugDamageBox2_->Initialize();
	debugDamageBox2_->SetCamera(camera_);
	debugDamageBox2_->SetModel("debugBox");
	specialDebugBox_ = std::make_unique<Object3d>();
	specialDebugBox_->Initialize();
	specialDebugBox_->SetCamera(camera_);
	specialDebugBox_->SetModel("debugBox");
	skillUpObject_ = std::make_unique<Object3d>();
	skillUpObject_->Initialize();
	skillUpObject_->SetCamera(camera_);
	skillUpObject_->SetModel("playerSkillUp");

	skillUnderObject_ = std::make_unique<Object3d>();
	skillUnderObject_->Initialize();
	skillUnderObject_->SetCamera(camera_);
	skillUnderObject_->SetModel("playerSkillUnder");
	transform_ = {
	    .scale{2.0f, 2.0f, 2.0f},
        .rotate{0.0f, 0.0f, 0.0f},
        .translate{0.0f, 0.0f, 0.0f}
    };
	damageTransform1_ = {
	    .scale{1.5f, 1.5f, 1.5f},
        .rotate{0.0f, 0.0f, 0.0f},
        .translate{0.0f, 0.0f, 0.0f}
    };
	damageTransform2_ = {
	    .scale{0.0f, 0.0f, 0.0f},
        .rotate{0.0f, 0.0f, 0.0f},
        .translate{0.0f, 0.0f, 0.0f}
    };
	particle_ = {
	    .scale{0.1f, 0.1f, 0.1f},
        .rotate{0,    0,    0   },
        .translate{0,    0,    0   }
    };
	skillEmitter_ = std::make_unique<ParticleEmitter>(
	    "skill", particle_, 1.0f, 10, Vector3{0, 0.01f, 0}, Vector3{-transform_.scale.x, 0, -transform_.scale.z}, Vector3{transform_.scale.x, 1.0f, transform_.scale.z});
	skillEmitter_->SetLife(20.0f);
	iceFlowers_ = std::make_unique<std::vector<Object3d>>();
	iceFlowers_->clear();
	iceFlowerTransforms_.clear();
	isSpecialEnd_ = true;
	specialTime_ = 0;
}
void PlayerSkill::Update() {
	

	switch (state) {
	case PlayerSkill::up:
		damageTransform1_.translate.y = Function::Lerp(transform_.translate.y, downstartposY, upTime);
		if (upTime < 1.0f) {
			upTime += 0.1f;
		} else {
			state = middle;
		}
		break;
	case PlayerSkill::middle:
		damageTransform1_.rotate.y += 0.5f;
		if (middleTime < 1.0f) {
			middleTime += 0.1f;
		} else {
			state = down;
		}

		break;
	case PlayerSkill::down:
		damageTransform1_.translate.y = Function::Lerp(downstartposY, 2.5f, downTime);
		if (downTime < 1.0f) {
			downTime += 0.1f;
		} else {
			state = damage;	
		}
		break;
	case PlayerSkill::damage:
		damageTransform2_.translate.y = 0.0f;
		damageTransform2_.scale.x = Function::Lerp(0, 3, damageTime);
		damageTransform2_.scale.y = Function::Lerp(0, 3, damageTime);
		damageTransform2_.scale.z = Function::Lerp(0, 3, damageTime);
		if (damageTime < 1.0f) {
			damageTime += 0.1f;
		} else {
			if (endTime < 30.0f) {
				endTime++;
			} else {
				isSkillEnd = true;
			}
		}
		break;
	default:
		break;
	}

	
	debugBox_->SetCamera(camera_);
	debugBox_->SetTransform(transform_);
	debugBox_->Update();
	debugDamageBox1_->SetCamera(camera_);
	debugDamageBox1_->SetTransform(damageTransform1_);
	debugDamageBox1_->Update();
	
	debugDamageBox2_->SetCamera(camera_);
	debugDamageBox2_->SetTransform(damageTransform2_);
	debugDamageBox2_->Update();

	skillUpObject_->SetCamera(camera_);
	skillUpObject_->SetTransform(damageTransform1_);
	skillUpObject_->Update();

	skillUnderObject_->SetCamera(camera_);
	skillUnderObject_->SetTransform(damageTransform2_);
	skillUnderObject_->Update();
	skillUnderObject_->SetTransform(damageTransform2_);
	skillEmitter_->Update(particle_);
}
void PlayerSkill::EnsureIceFlowerCount(int count) {
	if (count < 0) {
		count = 0;
	}
	if (!iceFlowers_) {
		iceFlowers_ = std::make_unique<std::vector<Object3d>>();
	}
	if (static_cast<int>(iceFlowers_->size()) == count) {
		return;
	}
	iceFlowers_->clear();
	iceFlowers_->resize(static_cast<size_t>(count));
	iceFlowerTransforms_.clear();
	iceFlowerTransforms_.resize(static_cast<size_t>(count));
	for (size_t i = 0; i < iceFlowers_->size(); i++) {
		(*iceFlowers_)[i] = Object3d();
		(*iceFlowers_)[i].Initialize();
		(*iceFlowers_)[i].SetCamera(camera_);
		(*iceFlowers_)[i].SetModel("iceFlower");
		iceFlowerTransforms_[i] = {
		    .scale{1.0f,		                     1.0f, 1.0f},
            .rotate{std::numbers::pi_v<float> / 2.0f, 0.0f, 0.0f},
            .translate{0.0f,                             0.0f, 0.0f}
        };
		(*iceFlowers_)[i].SetTransform(iceFlowerTransforms_[i]);
	}
}

void PlayerSkill::UpdateSpecialAttack(const Transform& playerTransform) {
	if (isSpecialEnd_) {
		return;
	}
	specialTransform_ = playerTransform;
	if (!iceFlowers_ || iceFlowers_->empty()) {
		isSpecialEnd_ = true;
		specialTime_ = 0;
		return;
	}
	for (size_t i = 0; i < iceFlowers_->size(); i++) {
		const float angle = (static_cast<float>(i) / iceFlowers_->size()) * std::numbers::pi_v<float> * 2.0f;

		iceFlowerTransforms_[i].translate.x = specialTransform_.translate.x + specialRadius_ * cosf(angle);
		iceFlowerTransforms_[i].translate.z = specialTransform_.translate.z + specialRadius_ * sinf(angle);
		iceFlowerTransforms_[i].translate.y -= specialFallSpeed_;
		iceFlowerTransforms_[i].rotate.y = specialTransform_.rotate.y;
		(*iceFlowers_)[i].SetCamera(camera_);
		(*iceFlowers_)[i].SetTransform(iceFlowerTransforms_[i]);
		(*iceFlowers_)[i].Update();
	}
	if (specialTime_ > specialTimeMax_) {
		isSpecialEnd_ = true;
		specialTime_ = 0;
	} else {
		specialTime_++;
	}
	specialDebugBox_->SetColor({1.0f, 0.0f, 1.0f, 0.5f});
	specialDebugBox_->SetCamera(camera_);
	specialDebugBox_->SetTransform(specialTransform_);
	specialDebugBox_->Update();
}

void PlayerSkill::StartAttack(const Transform& playerTransform) {
	transform_ = playerTransform;
	damageTransform1_ = playerTransform;
	damageTransform2_ = playerTransform;
	particle_.translate = {transform_.translate.x, transform_.translate.y - (transform_.scale.y * 0.5f), transform_.translate.z};

	damageTransform2_.translate.y = -5.0f;
	isSkillEnd = false;
	skillTime = 0;
	state = up;
	upTime = 0;
	middleTime = 0;
	downTime = 0;
	damageTime = 0;
	endTime = 0;
	skillDamageId_++;
}

void PlayerSkill::StartSpecialAttack(const Transform& playerTransform, int iceCount) {
	specialTransform_ = playerTransform;
	EnsureIceFlowerCount(iceCount);
	if (!iceFlowers_ || iceFlowers_->empty()) {
		isSpecialEnd_ = true;
		specialTime_ = 0;
		return;
	}
	isSpecialEnd_ = false;
	specialTime_ = 0;
	for (size_t i = 0; i < iceFlowers_->size(); i++) {
		const float angle = (static_cast<float>(i) / iceFlowers_->size()) * std::numbers::pi_v<float> * 2.0f;
		iceFlowerTransforms_[i].translate.x = specialTransform_.translate.x + specialRadius_ * cosf(angle);
		iceFlowerTransforms_[i].translate.z = specialTransform_.translate.z + specialRadius_ * sinf(angle);
		iceFlowerTransforms_[i].translate.y = specialTransform_.translate.y + specialStartHeight_;
		iceFlowerTransforms_[i].rotate.y = specialTransform_.rotate.y;
		(*iceFlowers_)[i].SetCamera(camera_);
		(*iceFlowers_)[i].SetTransform(iceFlowerTransforms_[i]);
		(*iceFlowers_)[i].Update();
	}
}

void PlayerSkill::Draw() {

#ifdef _DEBUG
	debugBox_->Draw();
	debugDamageBox1_->Draw();
	debugDamageBox2_->Draw();
#endif // _DEBUG

	if (state == State::damage) {
		if (skillEmitter_) {
			skillEmitter_->Draw();
		}
	}
	Object3dCommon::GetInstance()->DrawCommon();
	Object3dCommon::GetInstance()->SetBlendMode(BlendMode::kBlendScreen);
	skillUpObject_->Draw();
	skillUnderObject_->Draw();

	Object3dCommon::GetInstance()->SetBlendMode(BlendMode::kBlendModeAlpha);
}

void PlayerSkill::DrawSpecialAttack() {
	if (!iceFlowers_) {
		return;
	}
	Object3dCommon::GetInstance()->SetBlendMode(BlendMode::kBlendModeAdd);
	for (size_t i = 0; i < iceFlowers_->size(); i++) {
		(*iceFlowers_)[i].Draw();
	}
	Object3dCommon::GetInstance()->SetBlendMode(BlendMode::kBlendModeAlpha);
}



