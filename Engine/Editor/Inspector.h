#pragma once

#include "Transform.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

#include <string>

class Object3d;
class Primitive;

struct InspectorMaterial final {
	Vector4 color = {1.0f, 1.0f, 1.0f, 1.0f};
	bool enableLighting = true;
	float shininess = 40.0f;
	float environmentCoefficient = 0.0f;
	bool grayscaleEnabled = false;
	bool sepiaEnabled = false;
	float distortionStrength = 0.0f;
	float distortionFalloff = 1.0f;
	Vector3 uvScale = {1.0f, 1.0f, 1.0f};
	Vector3 uvRotate = {0.0f, 0.0f, 0.0f};
	Vector3 uvTranslate = {0.0f, 0.0f, 0.0f};
	Vector2 uvAnchor = {0.0f, 0.0f};
};

class Inspector final {
public:
	static bool
	    DrawObjectInspector(size_t index, std::string& objectName, Transform& transform, InspectorMaterial& material, bool isPlaying, bool& transformChanged, bool& materialChanged, bool& nameChanged);
	static bool DrawPrimitiveInspector(
	    size_t index, std::string& primitiveName, Transform& transform, InspectorMaterial& material, bool isPlaying, bool& transformChanged, bool& materialChanged, bool& nameChanged);

private:
	static bool DrawTransformEditor(const std::string& idSuffix, Transform& transform);
	static bool DrawMaterialEditor(const std::string& idSuffix, InspectorMaterial& material);
};