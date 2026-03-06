#pragma once

#include "Engine/math/Vector3.h"
#include "Engine/math/Vector4.h"

#include <vector>

struct EditorGridLine final {
	Vector3 start;
	Vector3 end;
	Vector4 color;
	float width;
};

class EditorGrid final {
public:
	static std::vector<EditorGridLine> CreateLines(int halfLineCount = 50, float spacing = 1.0f, float lineWidth = 1.0f);

private:
	static Vector4 GetLineColor(int index);
};