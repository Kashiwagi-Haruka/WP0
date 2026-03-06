#pragma once
#include "Sprite.h"
#include "Vector2.h"
#include <memory>

class RaySprite
{
public:
    enum TextureUI {
        PORTAL,
        HAND,
        GRAB,
    };
    RaySprite();
    void Initialize();
    void Update();
    void Draw();
    void SetTexture(const TextureUI num);
private:
    std::unique_ptr<Sprite> sprite_ = nullptr;
    uint32_t grabHandle_ = 0;
    uint32_t handHandle_ = 0;
    uint32_t portalHandle_ = 0;
};

