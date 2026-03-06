#pragma once

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

// 1頂点分の入力データ
struct VertexData {
	Vector4 position; // 頂点座標（x, y, z, w）
	Vector2 texcoord; // テクスチャ座標（u, v）
	Vector3 normal;   // 法線ベクトル
};