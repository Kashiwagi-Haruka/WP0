#pragma once
#include <memory>
#include "Transform.h"
#include "Object3d/Object3d.h"
class PlayerFallAttack {

	Transform fallingEffectTransform_;
	std::unique_ptr<Object3d> fallingEffectObject_;

	public:
	void Initialize();
	void Update(Camera* camera,Transform playerT);
	void Draw();


};
