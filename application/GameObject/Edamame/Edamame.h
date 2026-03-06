#pragma once
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include <string>
#include <Object3d/Object3d.h>
#include "Primitive/Primitive.h"

class Edamame
{
public:
    Edamame();
    void Initialize();
    void Update();
    void Draw();
    void SetPlayerCamera(PlayerCamera* camera);
    void SetCamera(Camera* camera);

    void SetModel(const std::string& filePath);
    Transform& GetTransform() { return collisionTransform_; }
    void CheckCollision();
    bool OnCollisionRay();
private:
    std::unique_ptr<Object3d>obj_ = nullptr;
#ifdef _DEBUG
    std::unique_ptr<Primitive>primitive_ = nullptr;
#endif
    AABB localAABB_ = {};
    Transform collisionTransform_ = {};
    PlayerCamera* playerCamera_ = nullptr;

    Transform worldTransform_ = {};
};

