#include "EditorGrid.h"

#include <cstdlib>

std::vector<EditorGridLine> EditorGrid::CreateLines(int halfLineCount, float spacing, float lineWidth) {
	if (halfLineCount < 0) {
		halfLineCount = 0;
	}
	if (spacing <= 0.0f) {
		spacing = 1.0f;
	}

	const int lineCount = halfLineCount * 2 + 1;
	std::vector<EditorGridLine> lines;
	lines.reserve(static_cast<size_t>(lineCount) * 2u);

	const float min = -static_cast<float>(halfLineCount) * spacing;
	const float max = static_cast<float>(halfLineCount) * spacing;

	for (int i = -halfLineCount; i <= halfLineCount; ++i) {
		const float offset = static_cast<float>(i) * spacing;
		const Vector4 color = GetLineColor(i);

		lines.push_back({
		    {min, 0.0f, offset},
		    {max, 0.0f, offset},
		    color,
		    lineWidth,
		});
		lines.push_back({
		    {offset, 0.0f, min},
		    {offset, 0.0f, max},
		    color,
		    lineWidth,
		});
	}

	return lines;
}

Vector4 EditorGrid::GetLineColor(int index) {
	if (index == 0) {
		return {1.0f, 1.0f, 0.0f, 1.0f}; // origin: yellow
	}

	const int oneDigit = std::abs(index) % 10;

	if (oneDigit == 0) {
		return {1.0f, 0.0f, 0.0f, 1.0f}; // ones digit 0: red
	}
	if (oneDigit == 5) {
		return {0.0f, 1.0f, 0.0f, 1.0f}; // ones digit 5: green
	}
	return {1.0f, 1.0f, 1.0f, 1.0f}; // others: white
}