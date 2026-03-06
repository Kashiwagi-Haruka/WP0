#pragma once
#include "Vector4.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace Color {
// 0-255 の RGBA 値を 0.0-1.0 の Vector4 に変換する
Vector4 RGBAToVector4(int r, int g, int b, int a) { return {static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f, static_cast<float>(a) / 255.0f}; }
// 0xRRGGBBAA 形式のカラーコードを Vector4(RGBA) に変換する
Vector4 ColorCodeToVector4(uint32_t color) {
	float r = ((color >> 24) & 0xFF) / 255.0f;
	float g = ((color >> 16) & 0xFF) / 255.0f;
	float b = ((color >> 8) & 0xFF) / 255.0f;
	float a = ((color) & 0xFF) / 255.0f;

	return {r, g, b, a};
}
// HSV(0-360, 0-100, 0-100) + Alpha(0-100) を Vector4(RGBA) に変換する
Vector4 HSVIntToVector4(int h, int s, int v, int a = 100) {
	h = h % 360;
	if (h < 0)
		h += 360;

	float sf = std::clamp(s / 100.0f, 0.0f, 1.0f);
	float vf = std::clamp(v / 100.0f, 0.0f, 1.0f);
	float af = std::clamp(a / 100.0f, 0.0f, 1.0f);

	float c = vf * sf;
	float x = c * (1.0f - std::fabs(std::fmod(h / 60.0f, 2.0f) - 1.0f));

	float m = vf - c;

	float r = 0.0f, g = 0.0f, b = 0.0f;

	if (h < 60) {
		r = c;
		g = x;
	} else if (h < 120) {
		r = x;
		g = c;
	} else if (h < 180) {
		g = c;
		b = x;
	} else if (h < 240) {
		g = x;
		b = c;
	} else if (h < 300) {
		r = x;
		b = c;
	} else {
		r = c;
		b = x;
	}

	return {r + m, g + m, b + m, af};
}
} // namespace Color