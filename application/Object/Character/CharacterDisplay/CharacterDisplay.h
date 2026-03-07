#pragma once
#include "Camera.h"
#include "Object3d/Object3d.h"
#include "Transform.h"
#include "CharacterDisplaySkyDome.h"
#include <memory>
#include "Object/Character/Sizuku/Sizuku.h"
#include <numbers>
#include "Light/DirectionalLight.h"
class CharacterDisplay {
	std::unique_ptr<Sizuku> sizukuObject_ = nullptr;
	std::unique_ptr<CharacterDisplaySkyDome> skyDome_ = nullptr;
	std::unique_ptr<Camera> camera_ = nullptr;
	Transform characterTransform_ = {
	    .scale{1.0f, 1.0f, 1.0f},
	    .rotate{0.0f,  std::numbers::pi_v<float>,  0.0f },
	    .translate{0.0f,  2.0f, 0.0f },
	};
	Transform cameraTransform_ = {
	    .scale{1.0f, 1.0f, 1.0f  },
	    .rotate{0.1f, 0.0f, 0.0f  },
	    .translate{0.0f, 2.0f, -5.0f},
	};
	float rotateSpeed_ = 0.01f;
	bool isActive_ = true;
	DirectionalLight directionalLight{.color{1,1,1,},.direction{0,-1,1.0f},.intensity{0.5f}};

public:
	void Initialize();
	void Update();
	void Draw();
	void SetActive(bool isActive) { isActive_ = isActive; }
	bool IsActive() const { return isActive_; }
	void SetCharacterTransform(const Transform& transform) { characterTransform_ = transform; }
	const Transform& GetCharacterTransform() const { return characterTransform_; }
	void SetCameraTransform(const Transform& transform) { cameraTransform_ = transform; }
	const Transform& GetCameraTransform() const { return cameraTransform_; }
};