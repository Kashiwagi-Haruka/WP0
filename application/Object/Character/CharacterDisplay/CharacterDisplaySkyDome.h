#pragma once
#include <memory>
#include "Object3d/Object3d.h"
#include "Transform.h"
class CharacterDisplaySkyDome {

	std::unique_ptr<Object3d> object_;
	Transform transform_;

	public:

	void Initialize(Camera* camera);
	void Update();
	void Draw();
};
