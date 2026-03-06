#pragma once
#include "Matrix4x4.h"
#include <cstdint>
#include <vector>

// 1頂点に対するウェイト情報
struct VertexWeightData {
	float weight;         // ボーンの影響度
	uint32_t vertexIndex; // 対象頂点インデックス
};

// 1ジョイントに紐づく逆バインド行列とウェイト配列
struct JointWeightData {
	Matrix4x4 inverseBindPoseMatrix;             // 逆バインドポーズ行列
	std::vector<VertexWeightData> vertexWeights; // このジョイントが影響する頂点群
};