#pragma once
#include <cstdint>

namespace {
    //プレイヤー陣営
    constexpr uint32_t kCollisionPlayer = 0b1;
    //壁
    constexpr uint64_t kCollisionFloor = 0b1 << 1;
    //アイテム
    constexpr uint64_t kCollisionItem = 0b1 << 2;
    //ポータル
    constexpr uint32_t kCollisionPortal = 0b1 << 3;
    //敵陣営
    constexpr uint32_t kCollisionEnemy = 0b1 << 4;
    //椅子
    constexpr uint64_t kCollisionChair = 0b1 << 5;
    //何の陣営にも属さない
    constexpr uint32_t kCollisionNone = 0b0;
}
