#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "Transform.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

#include "Inspector.h"
#include "Light/AreaLight.h"
#include "Light/DirectionalLight.h"
#include "Light/PointLight.h"
#include "Light/SpotLight.h"

class Object3d;
class Primitive;

class Hierarchy {
public:
	static Hierarchy* GetInstance();

	void RegisterObject3d(Object3d* object);
	void UnregisterObject3d(Object3d* object);
	void RegisterPrimitive(Primitive* primitive);
	void UnregisterPrimitive(Primitive* primitive);
	bool HasRegisteredObjects() const;
	void DrawObjectEditors();
	void DrawEditorGridLines();
	void SetPlayMode(bool isPlaying);
	bool IsPlayMode() const { return isPlaying_; }
	bool LoadObjectEditorsFromJsonIfExists(const std::string& filePath);

private:
	struct EditorSnapshot {
		std::vector<Transform> objectTransforms;
		std::vector<InspectorMaterial> objectMaterials;
		std::vector<std::string> objectNames;
		std::vector<Transform> primitiveTransforms;
		std::vector<InspectorMaterial> primitiveMaterials;
		std::vector<std::string> primitiveNames;
	};

	void DrawSceneSelector();
	void DrawGridEditor();
	void DrawLightEditor();
	void DrawSelectionBoxEditor();
	void DrawAudioEditor();
	void DrawCameraEditor();
	void SyncSelectionBoxToTarget();
	Transform GetSelectedTransform() const;
	bool IsObjectSelected() const;
	std::string GetSceneScopedEditorFilePath(const std::string& defaultFilePath) const;
	void ResetForSceneChange();
	void ApplyEditorSnapshot(const EditorSnapshot& snapshot);
	void UndoEditorChange();
	void RedoEditorChange();

	struct EditorLightState {
		bool overrideSceneLights = false;
		DirectionalLight directionalLight = {
		    {1.0f, 1.0f, 1.0f, 1.0f},
            {0.0f, -1.0f, 0.0f},
            1.0f
        };
		std::vector<PointLight> pointLights;
		std::vector<SpotLight> spotLights;
		std::vector<AreaLight> areaLights;
	};

	bool SaveObjectEditorsToJson(const std::string& filePath) const;
	bool LoadObjectEditorsFromJson(const std::string& filePath);

	std::vector<EditorSnapshot> undoStack_;
	std::vector<EditorSnapshot> redoStack_;

	std::vector<Object3d*> objects_;
	std::vector<std::string> objectNames_;
	std::vector<Transform> editorTransforms_;
	std::vector<InspectorMaterial> editorMaterials_;

	std::vector<Primitive*> primitives_;
	std::vector<std::string> primitiveNames_;
	std::vector<Transform> primitiveEditorTransforms_;
	std::vector<InspectorMaterial> primitiveEditorMaterials_;
	std::string saveStatusMessage_;

	bool showSelectionBox_ = true;
	std::unique_ptr<Primitive> selectionBoxPrimitive_;
	bool selectionBoxDirty_ = true;
	size_t selectedObjectIndex_ = 0;
	bool selectedIsPrimitive_ = false;

	bool hasUnsavedChanges_ = false;
	bool isPlaying_ = false;
	std::string loadedSceneName_;
	bool hasLoadedForCurrentScene_ = false;
	bool enableGridSnap_ = true;
	float gridSnapSpacing_ = 1.0f;
	bool showEditorGridLines_ = true;
	int gridHalfLineCount_ = 50;
	float editorGridY_ = 0.0f;
	bool editorGridDirty_ = true;
	std::unique_ptr<Primitive> editorGridPlane_;
	EditorLightState editorLightState_{};
	std::unordered_map<std::string, float> savedAudioVolumes_;
};