#pragma once
#include "Matrix4x4.h"
#include "Vector4.h"

// シェーダーへ渡すマテリアルパラメータ
struct Material {
	Vector4 color;                // ベースカラー
	int enableLighting;           // ライティング有効フラグ
	float padding[3];             // アライメント調整用
	Matrix4x4 uvTransform;        // UV 変換行列
	float shininess;              // スペキュラ強度
	float environmentCoefficient; // 環境反射係数
	int grayscaleEnabled;         // グレースケール有効フラグ
	int sepiaEnabled;             // セピア有効フラグ
	float distortionStrength;     // 歪み強度
	float distortionFalloff;      // 歪み減衰
};