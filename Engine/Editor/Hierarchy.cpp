#define NOMINMAX
#include "Hierarchy.h"
#include "EditorGrid.h"
#include "ToolBar.h"
#include "Camera.h"
#include "Engine/BaseScene/SceneManager.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Loadfile/JSON/JsonManager.h"
#include "Function.h"
#include "Object3d/Object3d.h"
#include "Object3d/Object3dCommon.h"
#include "Primitive/Primitive.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <string>

namespace {
std::filesystem::path ResolveObjectEditorJsonPath(const std::string& filePath) { return std::filesystem::path("Resources") / "JSON" / std::filesystem::path(filePath).filename(); }

bool HasObjectEditorJsonFile(const std::string& filePath) { return std::filesystem::exists(ResolveObjectEditorJsonPath(filePath)); }
} // namespace

Hierarchy* Hierarchy::GetInstance() {
	static Hierarchy instance;
	return &instance;
}
std::string Hierarchy::GetSceneScopedEditorFilePath(const std::string& defaultFilePath) const {
	const SceneManager* sceneManager = SceneManager::GetInstance();
	if (!sceneManager) {
		return defaultFilePath;
	}
	const std::string& sceneName = sceneManager->GetCurrentSceneName();
	if (sceneName.empty()) {
		return defaultFilePath;
	}
	return sceneName + "_" + std::filesystem::path(defaultFilePath).filename().string();
}

void Hierarchy::ResetForSceneChange() {
	hasUnsavedChanges_ = false;
	saveStatusMessage_.clear();
	hasLoadedForCurrentScene_ = false;
	editorLightState_.overrideSceneLights = false;
	undoStack_.clear();
	redoStack_.clear();
	savedAudioVolumes_.clear();
	editorLightState_.directionalLight = {
	    {1.0f, 1.0f, 1.0f, 1.0f},
        {0.0f, -1.0f, 0.0f},
        1.0f
    };
	editorLightState_.pointLights.clear();
	editorLightState_.spotLights.clear();
	editorLightState_.areaLights.clear();
	Object3dCommon::GetInstance()->SetEditorLightOverride(false);
}
void Hierarchy::ApplyEditorSnapshot(const EditorSnapshot& snapshot) {
	editorTransforms_ = snapshot.objectTransforms;
	editorMaterials_ = snapshot.objectMaterials;
	objectNames_ = snapshot.objectNames;
	primitiveEditorTransforms_ = snapshot.primitiveTransforms;
	primitiveEditorMaterials_ = snapshot.primitiveMaterials;
	primitiveNames_ = snapshot.primitiveNames;
	selectionBoxDirty_ = true;
}

void Hierarchy::UndoEditorChange() {
	if (undoStack_.empty()) {
		return;
	}
	EditorSnapshot current{};
	current.objectTransforms = editorTransforms_;
	current.objectMaterials = editorMaterials_;
	current.objectNames = objectNames_;
	current.primitiveTransforms = primitiveEditorTransforms_;
	current.primitiveMaterials = primitiveEditorMaterials_;
	current.primitiveNames = primitiveNames_;
	redoStack_.push_back(std::move(current));
	ApplyEditorSnapshot(undoStack_.back());
	undoStack_.pop_back();
	hasUnsavedChanges_ = true;
}

void Hierarchy::RedoEditorChange() {
	if (redoStack_.empty()) {
		return;
	}
	EditorSnapshot current{};
	current.objectTransforms = editorTransforms_;
	current.objectMaterials = editorMaterials_;
	current.objectNames = objectNames_;
	current.primitiveTransforms = primitiveEditorTransforms_;
	current.primitiveMaterials = primitiveEditorMaterials_;
	current.primitiveNames = primitiveNames_;
	undoStack_.push_back(std::move(current));
	ApplyEditorSnapshot(redoStack_.back());
	redoStack_.pop_back();
	hasUnsavedChanges_ = true;
}
void Hierarchy::RegisterObject3d(Object3d* object) {
	if (!object) {
		return;
	}
	if (std::find(objects_.begin(), objects_.end(), object) != objects_.end()) {
		return;
	}
	auto emptyIt = std::find(objects_.begin(), objects_.end(), nullptr);
	if (emptyIt != objects_.end()) {
		const size_t index = static_cast<size_t>(std::distance(objects_.begin(), emptyIt));
		objects_[index] = object;
		object->SetTransform(editorTransforms_[index]);
		const InspectorMaterial& material = editorMaterials_[index];
		object->SetColor(material.color);
		object->SetEnableLighting(material.enableLighting);
		object->SetShininess(material.shininess);
		object->SetEnvironmentCoefficient(material.environmentCoefficient);
		object->SetGrayscaleEnabled(material.grayscaleEnabled);
		object->SetSepiaEnabled(material.sepiaEnabled);
		object->SetDistortionStrength(material.distortionStrength);
		object->SetDistortionFalloff(material.distortionFalloff);
		object->SetUvTransform(material.uvScale, material.uvRotate, material.uvTranslate, material.uvAnchor);
		return;
	}
	const size_t index = objects_.size();
	objects_.push_back(object);
	objectNames_.push_back("Object " + std::to_string(index));
	editorTransforms_.push_back(object->GetTransform());
	editorMaterials_.push_back({
	    object->GetColor(),
	    object->IsLightingEnabled(),
	    object->GetShininess(),
	    object->GetEnvironmentCoefficient(),
	    object->IsGrayscaleEnabled(),
	    object->IsSepiaEnabled(),
	    object->GetDistortionStrength(),
	    object->GetDistortionFalloff(),
	});
}

void Hierarchy::UnregisterObject3d(Object3d* object) {
	if (!object) {
		return;
	}
	for (size_t i = 0; i < objects_.size(); ++i) {
		if (objects_[i] == object) {
			objects_[i] = nullptr;
			if (!selectedIsPrimitive_ && selectedObjectIndex_ == i) {
				selectedObjectIndex_ = 0;
			}
			break;
		}
	}
}

void Hierarchy::RegisterPrimitive(Primitive* primitive) {
	if (!primitive) {
		return;
	}
	if (std::find(primitives_.begin(), primitives_.end(), primitive) != primitives_.end()) {
		return;
	}
	auto emptyIt = std::find(primitives_.begin(), primitives_.end(), nullptr);
	if (emptyIt != primitives_.end()) {
		const size_t index = static_cast<size_t>(std::distance(primitives_.begin(), emptyIt));
		primitives_[index] = primitive;
		primitive->SetTransform(primitiveEditorTransforms_[index]);
		const InspectorMaterial& material = primitiveEditorMaterials_[index];
		primitive->SetColor(material.color);
		primitive->SetEnableLighting(material.enableLighting);
		primitive->SetShininess(material.shininess);
		primitive->SetEnvironmentCoefficient(material.environmentCoefficient);
		primitive->SetGrayscaleEnabled(material.grayscaleEnabled);
		primitive->SetSepiaEnabled(material.sepiaEnabled);
		primitive->SetDistortionStrength(material.distortionStrength);
		primitive->SetDistortionFalloff(material.distortionFalloff);
		primitive->SetUvTransform(material.uvScale, material.uvRotate, material.uvTranslate, material.uvAnchor);
		return;
	}
	const size_t index = primitives_.size();
	primitives_.push_back(primitive);
	primitiveNames_.push_back("Primitive " + std::to_string(index));
	primitiveEditorTransforms_.push_back(primitive->GetTransform());
	primitiveEditorMaterials_.push_back({
	    primitive->GetColor(),
	    primitive->IsLightingEnabled(),
	    primitive->GetShininess(),
	    primitive->GetEnvironmentCoefficient(),
	    primitive->IsGrayscaleEnabled(),
	    primitive->IsSepiaEnabled(),
	    primitive->GetDistortionStrength(),
	    primitive->GetDistortionFalloff(),
	});
}

void Hierarchy::UnregisterPrimitive(Primitive* primitive) {
	if (!primitive) {
		return;
	}
	for (size_t i = 0; i < primitives_.size(); ++i) {
		if (primitives_[i] == primitive) {
			primitives_[i] = nullptr;
			if (selectedIsPrimitive_ && selectedObjectIndex_ == i) {
				selectedObjectIndex_ = 0;
			}
			break;
		}
	}
}

bool Hierarchy::HasRegisteredObjects() const { return !objects_.empty() || !primitives_.empty(); }

bool Hierarchy::LoadObjectEditorsFromJsonIfExists(const std::string& filePath) {
	const SceneManager* sceneManager = SceneManager::GetInstance();
	const std::string sceneName = sceneManager ? sceneManager->GetCurrentSceneName() : std::string();
	if (sceneName != loadedSceneName_) {
		ResetForSceneChange();
		loadedSceneName_ = sceneName;
	}

	const std::string scopedFilePath = GetSceneScopedEditorFilePath(filePath);
	if (!HasObjectEditorJsonFile(scopedFilePath)) {
		return false;
	}
	if (hasLoadedForCurrentScene_) {
		return true;
	}
	hasLoadedForCurrentScene_ = LoadObjectEditorsFromJson(scopedFilePath);
	return hasLoadedForCurrentScene_;
}

bool Hierarchy::SaveObjectEditorsToJson(const std::string& filePath) const {
	nlohmann::json root;
	root["objects"] = nlohmann::json::array();
	root["primitives"] = nlohmann::json::array();
	root["lights"] = {
	    {"overrideSceneLights", editorLightState_.overrideSceneLights},
	    {"directional",
	     {
	         {"color",
	          {editorLightState_.directionalLight.color.x, editorLightState_.directionalLight.color.y, editorLightState_.directionalLight.color.z, editorLightState_.directionalLight.color.w}},
	         {"direction", {editorLightState_.directionalLight.direction.x, editorLightState_.directionalLight.direction.y, editorLightState_.directionalLight.direction.z}},
	         {"intensity", editorLightState_.directionalLight.intensity},
	     }	                                                       },
	    {"point",               nlohmann::json::array()              },
	    {"spot",                nlohmann::json::array()              },
	    {"area",                nlohmann::json::array()              },
	};
	root["audio"] = {
	    {"sounds", nlohmann::json::array()}
    };
	for (size_t i = 0; i < objects_.size(); ++i) {
		const Object3d* object = objects_[i];
		if (!object) {
			continue;
		}
		const Transform& transform = editorTransforms_[i];
		const InspectorMaterial& material = editorMaterials_[i];
		nlohmann::json objectJson;
		objectJson["index"] = i;
		objectJson["name"] = objectNames_[i];
		objectJson["transform"] = {
		    {"scale",     {transform.scale.x, transform.scale.y, transform.scale.z}            },
		    {"rotate",    {transform.rotate.x, transform.rotate.y, transform.rotate.z}         },
		    {"translate", {transform.translate.x, transform.translate.y, transform.translate.z}},
		};
		objectJson["material"] = {
		    {"color",                  {material.color.x, material.color.y, material.color.z, material.color.w}},
		    {"enableLighting",         material.enableLighting                                                 },
		    {"shininess",              material.shininess		                                              },
		    {"environmentCoefficient", material.environmentCoefficient                                         },
		    {"grayscaleEnabled",       material.grayscaleEnabled                                               },
		    {"sepiaEnabled",           material.sepiaEnabled		                                           },
		    {"distortionStrength",     material.distortionStrength                                             },
		    {"distortionFalloff",      material.distortionFalloff                                              },
		    {"uvScale",                {material.uvScale.x, material.uvScale.y, material.uvScale.z}            },
		    {"uvRotate",               {material.uvRotate.x, material.uvRotate.y, material.uvRotate.z}         },
		    {"uvTranslate",            {material.uvTranslate.x, material.uvTranslate.y, material.uvTranslate.z}},
		    {"uvAnchor",               {material.uvAnchor.x, material.uvAnchor.y}                              },
		};
		root["objects"].push_back(objectJson);
	}


	for (size_t i = 0; i < primitives_.size(); ++i) {
		const Primitive* primitive = primitives_[i];
		if (!primitive || primitive == selectionBoxPrimitive_.get()) {
			continue;
		}
		const Transform& transform = primitiveEditorTransforms_[i];
		const InspectorMaterial& material = editorMaterials_[i];
		nlohmann::json primitiveJson;
		primitiveJson["index"] = i;
		primitiveJson["name"] = primitiveNames_[i];
		primitiveJson["transform"] = {
		    {"scale",     {transform.scale.x, transform.scale.y, transform.scale.z}            },
		    {"rotate",    {transform.rotate.x, transform.rotate.y, transform.rotate.z}         },
		    {"translate", {transform.translate.x, transform.translate.y, transform.translate.z}},
		};
		primitiveJson["material"] = {
		    {"color",                  {material.color.x, material.color.y, material.color.z, material.color.w}},
		    {"enableLighting",         material.enableLighting                                                 },
		    {"shininess",              material.shininess		                                              },
		    {"environmentCoefficient", material.environmentCoefficient                                         },
		    {"grayscaleEnabled",       material.grayscaleEnabled                                               },
		    {"sepiaEnabled",           material.sepiaEnabled		                                           },
		    {"distortionStrength",     material.distortionStrength                                             },
		    {"distortionFalloff",      material.distortionFalloff                                              },
		    {"uvScale",                {material.uvScale.x, material.uvScale.y, material.uvScale.z}            },
		    {"uvRotate",               {material.uvRotate.x, material.uvRotate.y, material.uvRotate.z}         },
		    {"uvTranslate",            {material.uvTranslate.x, material.uvTranslate.y, material.uvTranslate.z}},
		    {"uvAnchor",               {material.uvAnchor.x, material.uvAnchor.y}                              },
		};
		root["primitives"].push_back(primitiveJson);
	}
	for (const PointLight& point : editorLightState_.pointLights) {
		root["lights"]["point"].push_back({
		    {"color",     {point.color.x, point.color.y, point.color.z, point.color.w}},
		    {"position",  {point.position.x, point.position.y, point.position.z}      },
		    {"intensity", point.intensity		                                     },
		    {"radius",    point.radius		                                        },
		    {"decay",     point.decay		                                         },
		});
	}

	for (const SpotLight& spot : editorLightState_.spotLights) {
		root["lights"]["spot"].push_back({
		    {"color",           {spot.color.x, spot.color.y, spot.color.z, spot.color.w}},
		    {"position",        {spot.position.x, spot.position.y, spot.position.z}     },
		    {"direction",       {spot.direction.x, spot.direction.y, spot.direction.z}  },
		    {"intensity",       spot.intensity		                                  },
		    {"distance",        spot.distance		                                   },
		    {"decay",           spot.decay		                                      },
		    {"cosAngle",        spot.cosAngle		                                   },
		    {"cosFalloffStart", spot.cosFalloffStart                                    },
		});
	}

	for (const AreaLight& area : editorLightState_.areaLights) {
		root["lights"]["area"].push_back({
		    {"color",     {area.color.x, area.color.y, area.color.z, area.color.w}},
		    {"position",  {area.position.x, area.position.y, area.position.z}     },
		    {"normal",    {area.normal.x, area.normal.y, area.normal.z}           },
		    {"intensity", area.intensity		                                  },
		    {"width",     area.width		                                      },
		    {"height",    area.height		                                     },
		    {"radius",    area.radius		                                     },
		    {"decay",     area.decay		                                      },
		});
	}
	Audio* audio = Audio::GetInstance();
	if (audio) {
		for (const auto& entry : audio->GetEditorSoundEntries()) {
			if (!entry.soundData || entry.name.empty()) {
				continue;
			}
			root["audio"]["sounds"].push_back({
			    {"name",   entry.name             },
			    {"volume", entry.soundData->volume},
			});
		}
	}
	JsonManager* jsonManager = JsonManager::GetInstance();
	jsonManager->SetData(root);
	return jsonManager->SaveJson(filePath);
}

bool Hierarchy::LoadObjectEditorsFromJson(const std::string& filePath) {
	JsonManager* jsonManager = JsonManager::GetInstance();
	if (!jsonManager->LoadJson(filePath)) {
		return false;
	}

	const nlohmann::json& root = jsonManager->GetData();
	if (!root.is_object()) {
		return false;
	}

	if (root.contains("objects") && root["objects"].is_array()) {
		for (const auto& objectJson : root["objects"]) {
			if (!objectJson.contains("index") || !objectJson["index"].is_number_unsigned()) {
				continue;
			}
			const size_t index = objectJson["index"].get<size_t>();
			if (index >= objects_.size() || !objects_[index]) {
				continue;
			}
			if (objectJson.contains("name") && objectJson["name"].is_string()) {
				objectNames_[index] = objectJson["name"].get<std::string>();
			}
			if (objectJson.contains("transform") && objectJson["transform"].is_object()) {
				const auto& transformJson = objectJson["transform"];
				if (transformJson.contains("scale") && transformJson["scale"].is_array() && transformJson["scale"].size() == 3 && transformJson.contains("rotate") &&
				    transformJson["rotate"].is_array() && transformJson["rotate"].size() == 3 && transformJson.contains("translate") && transformJson["translate"].is_array() &&
				    transformJson["translate"].size() == 3) {
					Transform transform = objects_[index]->GetTransform();
					transform.scale = {transformJson["scale"][0].get<float>(), transformJson["scale"][1].get<float>(), transformJson["scale"][2].get<float>()};
					transform.rotate = {transformJson["rotate"][0].get<float>(), transformJson["rotate"][1].get<float>(), transformJson["rotate"][2].get<float>()};
					transform.translate = {transformJson["translate"][0].get<float>(), transformJson["translate"][1].get<float>(), transformJson["translate"][2].get<float>()};
					editorTransforms_[index] = transform;
					objects_[index]->SetTransform(transform);
				}
			}
			InspectorMaterial material = editorMaterials_[index];
			if (objectJson.contains("material") && objectJson["material"].is_object()) {
				const auto& materialJson = objectJson["material"];
				if (materialJson.contains("color") && materialJson["color"].is_array() && materialJson["color"].size() == 4) {
					material.color = {materialJson["color"][0].get<float>(), materialJson["color"][1].get<float>(), materialJson["color"][2].get<float>(), materialJson["color"][3].get<float>()};
				}
				if (materialJson.contains("enableLighting") && materialJson["enableLighting"].is_boolean()) {
					material.enableLighting = materialJson["enableLighting"].get<bool>();
				}
				if (materialJson.contains("shininess") && materialJson["shininess"].is_number()) {
					material.shininess = materialJson["shininess"].get<float>();
				}
				if (materialJson.contains("environmentCoefficient") && materialJson["environmentCoefficient"].is_number()) {
					material.environmentCoefficient = materialJson["environmentCoefficient"].get<float>();
				}
				if (materialJson.contains("grayscaleEnabled") && materialJson["grayscaleEnabled"].is_boolean()) {
					material.grayscaleEnabled = materialJson["grayscaleEnabled"].get<bool>();
				}
				if (materialJson.contains("sepiaEnabled") && materialJson["sepiaEnabled"].is_boolean()) {
					material.sepiaEnabled = materialJson["sepiaEnabled"].get<bool>();
				}
				if (materialJson.contains("distortionStrength") && materialJson["distortionStrength"].is_number()) {
					material.distortionStrength = materialJson["distortionStrength"].get<float>();
				}
				if (materialJson.contains("distortionFalloff") && materialJson["distortionFalloff"].is_number()) {
					material.distortionFalloff = materialJson["distortionFalloff"].get<float>();
				}
				if (materialJson.contains("uvScale") && materialJson["uvScale"].is_array() && materialJson["uvScale"].size() == 3) {
					material.uvScale = {materialJson["uvScale"][0].get<float>(), materialJson["uvScale"][1].get<float>(), materialJson["uvScale"][2].get<float>()};
				}
				if (materialJson.contains("uvRotate") && materialJson["uvRotate"].is_array() && materialJson["uvRotate"].size() == 3) {
					material.uvRotate = {materialJson["uvRotate"][0].get<float>(), materialJson["uvRotate"][1].get<float>(), materialJson["uvRotate"][2].get<float>()};
				}
				if (materialJson.contains("uvTranslate") && materialJson["uvTranslate"].is_array() && materialJson["uvTranslate"].size() == 3) {
					material.uvTranslate = {materialJson["uvTranslate"][0].get<float>(), materialJson["uvTranslate"][1].get<float>(), materialJson["uvTranslate"][2].get<float>()};
				}
				if (materialJson.contains("uvAnchor") && materialJson["uvAnchor"].is_array() && materialJson["uvAnchor"].size() == 2) {
					material.uvAnchor = {materialJson["uvAnchor"][0].get<float>(), materialJson["uvAnchor"][1].get<float>()};
				}
			}
			editorMaterials_[index] = material;
			objects_[index]->SetColor(material.color);
			objects_[index]->SetEnableLighting(material.enableLighting);
			objects_[index]->SetShininess(material.shininess);
			objects_[index]->SetEnvironmentCoefficient(material.environmentCoefficient);
			objects_[index]->SetGrayscaleEnabled(material.grayscaleEnabled);
			objects_[index]->SetSepiaEnabled(material.sepiaEnabled);
			objects_[index]->SetDistortionStrength(material.distortionStrength);
			objects_[index]->SetDistortionFalloff(material.distortionFalloff);
			objects_[index]->SetUvTransform(material.uvScale, material.uvRotate, material.uvTranslate, material.uvAnchor);
		}
	}

	if (root.contains("primitives") && root["primitives"].is_array()) {
		for (const auto& primitiveJson : root["primitives"]) {
			if (!primitiveJson.contains("index") || !primitiveJson["index"].is_number_unsigned()) {
				continue;
			}
			const size_t index = primitiveJson["index"].get<size_t>();
			if (index >= primitives_.size() || !primitives_[index] || primitives_[index] == selectionBoxPrimitive_.get()) {
				continue;
			}
			if (primitiveJson.contains("name") && primitiveJson["name"].is_string()) {
				primitiveNames_[index] = primitiveJson["name"].get<std::string>();
			}
			if (primitiveJson.contains("transform") && primitiveJson["transform"].is_object()) {
				const auto& transformJson = primitiveJson["transform"];
				if (transformJson.contains("scale") && transformJson["scale"].is_array() && transformJson["scale"].size() == 3 && transformJson.contains("rotate") &&
				    transformJson["rotate"].is_array() && transformJson["rotate"].size() == 3 && transformJson.contains("translate") && transformJson["translate"].is_array() &&
				    transformJson["translate"].size() == 3) {
					Transform transform = primitives_[index]->GetTransform();
					transform.scale = {transformJson["scale"][0].get<float>(), transformJson["scale"][1].get<float>(), transformJson["scale"][2].get<float>()};
					transform.rotate = {transformJson["rotate"][0].get<float>(), transformJson["rotate"][1].get<float>(), transformJson["rotate"][2].get<float>()};
					transform.translate = {transformJson["translate"][0].get<float>(), transformJson["translate"][1].get<float>(), transformJson["translate"][2].get<float>()};
					primitiveEditorTransforms_[index] = transform;
					primitives_[index]->SetTransform(transform);
				}
			}
			InspectorMaterial material = primitiveEditorMaterials_[index];
			if (primitiveJson.contains("material") && primitiveJson["material"].is_object()) {
				const auto& materialJson = primitiveJson["material"];
				if (materialJson.contains("color") && materialJson["color"].is_array() && materialJson["color"].size() == 4) {
					material.color = {materialJson["color"][0].get<float>(), materialJson["color"][1].get<float>(), materialJson["color"][2].get<float>(), materialJson["color"][3].get<float>()};
				}
				if (materialJson.contains("enableLighting") && materialJson["enableLighting"].is_boolean()) {
					material.enableLighting = materialJson["enableLighting"].get<bool>();
				}
				if (materialJson.contains("shininess") && materialJson["shininess"].is_number()) {
					material.shininess = materialJson["shininess"].get<float>();
				}
				if (materialJson.contains("environmentCoefficient") && materialJson["environmentCoefficient"].is_number()) {
					material.environmentCoefficient = materialJson["environmentCoefficient"].get<float>();
				}
				if (materialJson.contains("grayscaleEnabled") && materialJson["grayscaleEnabled"].is_boolean()) {
					material.grayscaleEnabled = materialJson["grayscaleEnabled"].get<bool>();
				}
				if (materialJson.contains("sepiaEnabled") && materialJson["sepiaEnabled"].is_boolean()) {
					material.sepiaEnabled = materialJson["sepiaEnabled"].get<bool>();
				}
				if (materialJson.contains("distortionStrength") && materialJson["distortionStrength"].is_number()) {
					material.distortionStrength = materialJson["distortionStrength"].get<float>();
				}
				if (materialJson.contains("distortionFalloff") && materialJson["distortionFalloff"].is_number()) {
					material.distortionFalloff = materialJson["distortionFalloff"].get<float>();
				}
				if (materialJson.contains("uvScale") && materialJson["uvScale"].is_array() && materialJson["uvScale"].size() == 3) {
					material.uvScale = {materialJson["uvScale"][0].get<float>(), materialJson["uvScale"][1].get<float>(), materialJson["uvScale"][2].get<float>()};
				}
				if (materialJson.contains("uvRotate") && materialJson["uvRotate"].is_array() && materialJson["uvRotate"].size() == 3) {
					material.uvRotate = {materialJson["uvRotate"][0].get<float>(), materialJson["uvRotate"][1].get<float>(), materialJson["uvRotate"][2].get<float>()};
				}
				if (materialJson.contains("uvTranslate") && materialJson["uvTranslate"].is_array() && materialJson["uvTranslate"].size() == 3) {
					material.uvTranslate = {materialJson["uvTranslate"][0].get<float>(), materialJson["uvTranslate"][1].get<float>(), materialJson["uvTranslate"][2].get<float>()};
				}
				if (materialJson.contains("uvAnchor") && materialJson["uvAnchor"].is_array() && materialJson["uvAnchor"].size() == 2) {
					material.uvAnchor = {materialJson["uvAnchor"][0].get<float>(), materialJson["uvAnchor"][1].get<float>()};
				}
			}
			primitiveEditorMaterials_[index] = material;
			primitives_[index]->SetColor(material.color);
			primitives_[index]->SetEnableLighting(material.enableLighting);
			primitives_[index]->SetShininess(material.shininess);
			primitives_[index]->SetEnvironmentCoefficient(material.environmentCoefficient);
			primitives_[index]->SetGrayscaleEnabled(material.grayscaleEnabled);
			primitives_[index]->SetSepiaEnabled(material.sepiaEnabled);
			primitives_[index]->SetDistortionStrength(material.distortionStrength);
			primitives_[index]->SetDistortionFalloff(material.distortionFalloff);
			primitives_[index]->SetUvTransform(material.uvScale, material.uvRotate, material.uvTranslate, material.uvAnchor);
		}
	}

	if (root.contains("lights") && root["lights"].is_object()) {
		const auto& lightsJson = root["lights"];

		if (lightsJson.contains("overrideSceneLights") && lightsJson["overrideSceneLights"].is_boolean()) {
			editorLightState_.overrideSceneLights = lightsJson["overrideSceneLights"].get<bool>();
		}

		if (lightsJson.contains("directional") && lightsJson["directional"].is_object()) {
			const auto& directionalJson = lightsJson["directional"];
			if (directionalJson.contains("color") && directionalJson["color"].is_array() && directionalJson["color"].size() == 4) {
				editorLightState_.directionalLight.color = {
				    directionalJson["color"][0].get<float>(), directionalJson["color"][1].get<float>(), directionalJson["color"][2].get<float>(), directionalJson["color"][3].get<float>()};
			}
			if (directionalJson.contains("direction") && directionalJson["direction"].is_array() && directionalJson["direction"].size() == 3) {
				editorLightState_.directionalLight.direction = {
				    directionalJson["direction"][0].get<float>(), directionalJson["direction"][1].get<float>(), directionalJson["direction"][2].get<float>()};
			}
			if (directionalJson.contains("intensity") && directionalJson["intensity"].is_number()) {
				editorLightState_.directionalLight.intensity = directionalJson["intensity"].get<float>();
			}
		}

		if (lightsJson.contains("point") && lightsJson["point"].is_array()) {
			editorLightState_.pointLights.clear();
			for (const auto& pointJson : lightsJson["point"]) {
				if (!pointJson.is_object()) {
					continue;
				}
				PointLight point{};
				if (pointJson.contains("color") && pointJson["color"].is_array() && pointJson["color"].size() == 4) {
					point.color = {pointJson["color"][0].get<float>(), pointJson["color"][1].get<float>(), pointJson["color"][2].get<float>(), pointJson["color"][3].get<float>()};
				}
				if (pointJson.contains("position") && pointJson["position"].is_array() && pointJson["position"].size() == 3) {
					point.position = {pointJson["position"][0].get<float>(), pointJson["position"][1].get<float>(), pointJson["position"][2].get<float>()};
				}
				if (pointJson.contains("intensity") && pointJson["intensity"].is_number()) {
					point.intensity = pointJson["intensity"].get<float>();
				}
				if (pointJson.contains("radius") && pointJson["radius"].is_number()) {
					point.radius = pointJson["radius"].get<float>();
				}
				if (pointJson.contains("decay") && pointJson["decay"].is_number()) {
					point.decay = pointJson["decay"].get<float>();
				}
				editorLightState_.pointLights.push_back(point);
				if (editorLightState_.pointLights.size() >= kMaxPointLights) {
					break;
				}
			}
		}

		if (lightsJson.contains("spot") && lightsJson["spot"].is_array()) {
			editorLightState_.spotLights.clear();
			for (const auto& spotJson : lightsJson["spot"]) {
				if (!spotJson.is_object()) {
					continue;
				}
				SpotLight spot{};
				if (spotJson.contains("color") && spotJson["color"].is_array() && spotJson["color"].size() == 4) {
					spot.color = {spotJson["color"][0].get<float>(), spotJson["color"][1].get<float>(), spotJson["color"][2].get<float>(), spotJson["color"][3].get<float>()};
				}
				if (spotJson.contains("position") && spotJson["position"].is_array() && spotJson["position"].size() == 3) {
					spot.position = {spotJson["position"][0].get<float>(), spotJson["position"][1].get<float>(), spotJson["position"][2].get<float>()};
				}
				if (spotJson.contains("direction") && spotJson["direction"].is_array() && spotJson["direction"].size() == 3) {
					spot.direction = {spotJson["direction"][0].get<float>(), spotJson["direction"][1].get<float>(), spotJson["direction"][2].get<float>()};
				}
				if (spotJson.contains("intensity") && spotJson["intensity"].is_number()) {
					spot.intensity = spotJson["intensity"].get<float>();
				}
				if (spotJson.contains("distance") && spotJson["distance"].is_number()) {
					spot.distance = spotJson["distance"].get<float>();
				}
				if (spotJson.contains("decay") && spotJson["decay"].is_number()) {
					spot.decay = spotJson["decay"].get<float>();
				}
				if (spotJson.contains("cosAngle") && spotJson["cosAngle"].is_number()) {
					spot.cosAngle = spotJson["cosAngle"].get<float>();
				}
				if (spotJson.contains("cosFalloffStart") && spotJson["cosFalloffStart"].is_number()) {
					spot.cosFalloffStart = spotJson["cosFalloffStart"].get<float>();
				}
				editorLightState_.spotLights.push_back(spot);
				if (editorLightState_.spotLights.size() >= kMaxSpotLights) {
					break;
				}
			}
		}

		if (lightsJson.contains("area") && lightsJson["area"].is_array()) {
			editorLightState_.areaLights.clear();
			for (const auto& areaJson : lightsJson["area"]) {
				if (!areaJson.is_object()) {
					continue;
				}
				AreaLight area{};
				if (areaJson.contains("color") && areaJson["color"].is_array() && areaJson["color"].size() == 4) {
					area.color = {areaJson["color"][0].get<float>(), areaJson["color"][1].get<float>(), areaJson["color"][2].get<float>(), areaJson["color"][3].get<float>()};
				}
				if (areaJson.contains("position") && areaJson["position"].is_array() && areaJson["position"].size() == 3) {
					area.position = {areaJson["position"][0].get<float>(), areaJson["position"][1].get<float>(), areaJson["position"][2].get<float>()};
				}
				if (areaJson.contains("normal") && areaJson["normal"].is_array() && areaJson["normal"].size() == 3) {
					area.normal = {areaJson["normal"][0].get<float>(), areaJson["normal"][1].get<float>(), areaJson["normal"][2].get<float>()};
				}
				if (areaJson.contains("intensity") && areaJson["intensity"].is_number()) {
					area.intensity = areaJson["intensity"].get<float>();
				}
				if (areaJson.contains("width") && areaJson["width"].is_number()) {
					area.width = areaJson["width"].get<float>();
				}
				if (areaJson.contains("height") && areaJson["height"].is_number()) {
					area.height = areaJson["height"].get<float>();
				}
				if (areaJson.contains("radius") && areaJson["radius"].is_number()) {
					area.radius = areaJson["radius"].get<float>();
				}
				if (areaJson.contains("decay") && areaJson["decay"].is_number()) {
					area.decay = areaJson["decay"].get<float>();
				}
				editorLightState_.areaLights.push_back(area);
				if (editorLightState_.areaLights.size() >= kMaxAreaLights) {
					break;
				}
			}
		}

		Object3dCommon::GetInstance()->SetEditorLightOverride(editorLightState_.overrideSceneLights);
		Object3dCommon::GetInstance()->SetEditorLights(
		    editorLightState_.directionalLight, editorLightState_.pointLights.empty() ? nullptr : editorLightState_.pointLights.data(), static_cast<uint32_t>(editorLightState_.pointLights.size()),
		    editorLightState_.spotLights.empty() ? nullptr : editorLightState_.spotLights.data(), static_cast<uint32_t>(editorLightState_.spotLights.size()),
		    editorLightState_.areaLights.empty() ? nullptr : editorLightState_.areaLights.data(), static_cast<uint32_t>(editorLightState_.areaLights.size()));
	}
	if (root.contains("audio") && root["audio"].is_object()) {
		const auto& audioJson = root["audio"];
		if (audioJson.contains("sounds") && audioJson["sounds"].is_array()) {
			for (const auto& soundJson : audioJson["sounds"]) {
				if (!soundJson.is_object()) {
					continue;
				}
				if (!soundJson.contains("name") || !soundJson["name"].is_string()) {
					continue;
				}
				if (!soundJson.contains("volume") || !soundJson["volume"].is_number()) {
					continue;
				}
				const std::string name = soundJson["name"].get<std::string>();
				const float volume = std::clamp(soundJson["volume"].get<float>(), 0.0f, 1.0f);
				savedAudioVolumes_[name] = volume;
			}
		}
	}
	return true;
}
void Hierarchy::DrawSceneSelector() {
#ifdef USE_IMGUI
	SceneManager* sceneManager = SceneManager::GetInstance();
	if (!sceneManager) {
		return;
	}
	const std::vector<std::string> sceneNames = sceneManager->GetSceneNames();
	if (sceneNames.empty()) {
		ImGui::TextUnformatted("No scene list available");
		return;
	}

	const std::string& currentScene = sceneManager->GetCurrentSceneName();
	if (ImGui::BeginCombo("Scene", currentScene.empty() ? "(none)" : currentScene.c_str())) {
		for (const std::string& sceneName : sceneNames) {
			const bool isSelected = (sceneName == currentScene);
			if (ImGui::Selectable(sceneName.c_str(), isSelected) && !isSelected) {
				sceneManager->ChangeScene(sceneName);
				hasUnsavedChanges_ = false;
				saveStatusMessage_ = "Scene changed: " + sceneName;
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
#else
	SceneManager* sceneManager = SceneManager::GetInstance();
	if (!sceneManager) {
		return;
	}
#endif
}

void Hierarchy::DrawGridEditor() {
#ifdef USE_IMGUI
	ImGui::Checkbox("Enable Grid Snap", &enableGridSnap_);
	if (ImGui::DragFloat("Grid Snap Spacing", &gridSnapSpacing_, 0.05f, 0.1f, 100.0f, "%.2f")) {
		editorGridDirty_ = true;
	}
	if (ImGui::Checkbox("Draw Editor Grid Lines", &showEditorGridLines_)) {
		editorGridDirty_ = true;
	}
	if (ImGui::DragInt("Grid Half Line Count", &gridHalfLineCount_, 1.0f, 1, 200)) {
		editorGridDirty_ = true;
	}
	if (ImGui::DragFloat("Grid Y", &editorGridY_, 0.01f, -100.0f, 100.0f, "%.2f")) {
		editorGridDirty_ = true;
	}
#endif

	gridSnapSpacing_ = std::max(gridSnapSpacing_, 0.1f);
	gridHalfLineCount_ = std::max(gridHalfLineCount_, 1);
}
void Hierarchy::SetPlayMode(bool isPlaying) { isPlaying_ = isPlaying; }

void Hierarchy::DrawEditorGridLines() {
#ifdef USE_IMGUI
	if (!showEditorGridLines_) {
		return;
	}

	if (editorGridDirty_ || !editorGridPlane_) {
		if (!editorGridPlane_) {
			editorGridPlane_ = std::make_unique<Primitive>();
			editorGridPlane_->SetEditorRegistrationEnabled(false);
			editorGridPlane_->Initialize(Primitive::Plane);
			editorGridPlane_->SetCamera(Object3dCommon::GetInstance()->GetDefaultCamera());
			editorGridPlane_->SetEnableLighting(false);
		}

		const float extent = static_cast<float>(gridHalfLineCount_) * gridSnapSpacing_;
		Transform gridTransform{};
		gridTransform.scale = {extent * 2.0f, extent * 2.0f, 1.0f};
		gridTransform.rotate = {Function::kPi * 0.5f, 0.0f, 0.0f};
		gridTransform.translate = {0.0f, editorGridY_, 0.0f};
		editorGridPlane_->SetTransform(gridTransform);
		editorGridPlane_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
		editorGridPlane_->SetDistortionFalloff(gridSnapSpacing_);                        // spacing
		editorGridPlane_->SetDistortionStrength(static_cast<float>(gridHalfLineCount_)); // half line count
		editorGridPlane_->SetEnvironmentCoefficient(gridSnapSpacing_ * 0.025f);          // line width in world unit
		editorGridDirty_ = false;
	}

	if (!editorGridPlane_) {
		return;
	}

	// シーン切り替え後にカメラが再生成されるため、毎フレーム最新のカメラを参照する
	editorGridPlane_->SetCamera(Object3dCommon::GetInstance()->GetDefaultCamera());

	Object3dCommon::GetInstance()->DrawCommonEditorGrid();
	editorGridPlane_->Update();
	editorGridPlane_->Draw();
	if (!showSelectionBox_ || !IsObjectSelected()) {
		return;
	}
	if (selectionBoxDirty_ || !selectionBoxPrimitive_) {
		SyncSelectionBoxToTarget();
		selectionBoxDirty_ = false;
	}
	if (!selectionBoxPrimitive_) {
		return;
	}
	selectionBoxPrimitive_->SetCamera(Object3dCommon::GetInstance()->GetDefaultCamera());
	Object3dCommon::GetInstance()->DrawCommonWireframeNoDepth();
	selectionBoxPrimitive_->Update();
	selectionBoxPrimitive_->Draw();
#endif
}
void Hierarchy::DrawCameraEditor() {
#ifdef USE_IMGUI
	Object3dCommon* object3dCommon = Object3dCommon::GetInstance();
	if (!object3dCommon) {
		ImGui::TextUnformatted("Object3dCommon unavailable");
		return;
	}
	Camera* camera = object3dCommon->GetDefaultCamera();
	if (!camera) {
		ImGui::TextUnformatted("No default camera");
		return;
	}
	camera->DrawEditorInHierarchy();
#endif
}
void Hierarchy::DrawLightEditor() {
#ifdef USE_IMGUI
	bool overrideChanged = ImGui::Checkbox("Use Editor Lights", &editorLightState_.overrideSceneLights);
	if (overrideChanged) {
		Object3dCommon::GetInstance()->SetEditorLightOverride(editorLightState_.overrideSceneLights);
		hasUnsavedChanges_ = true;
	}

	bool lightChanged = false;
	if (ImGui::TreeNode("Directional Light")) {
		if (!isPlaying_) {
			lightChanged |= ImGui::ColorEdit4("Dir Color", &editorLightState_.directionalLight.color.x);
			lightChanged |= ImGui::DragFloat3("Dir Direction", &editorLightState_.directionalLight.direction.x, 0.01f, -1.0f, 1.0f);
			lightChanged |= ImGui::DragFloat("Dir Intensity", &editorLightState_.directionalLight.intensity, 0.01f, 0.0f, 10.0f);
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Point Lights")) {
		int pointCount = static_cast<int>(editorLightState_.pointLights.size());
		if (!isPlaying_ && ImGui::SliderInt("Point Count", &pointCount, 0, static_cast<int>(kMaxPointLights))) {
			editorLightState_.pointLights.resize(static_cast<size_t>(pointCount));
			lightChanged = true;
		}
		for (size_t i = 0; i < editorLightState_.pointLights.size(); ++i) {
			PointLight& point = editorLightState_.pointLights[i];
			const std::string label = "Point " + std::to_string(i);
			if (ImGui::TreeNode((label + "##point").c_str())) {
				if (!isPlaying_) {
					lightChanged |= ImGui::ColorEdit4(("Color##point_" + std::to_string(i)).c_str(), &point.color.x);
					lightChanged |= ImGui::DragFloat3(("Position##point_" + std::to_string(i)).c_str(), &point.position.x, 0.05f);
					lightChanged |= ImGui::DragFloat(("Intensity##point_" + std::to_string(i)).c_str(), &point.intensity, 0.01f, 0.0f, 10.0f);
					lightChanged |= ImGui::DragFloat(("Radius##point_" + std::to_string(i)).c_str(), &point.radius, 0.05f, 0.0f, 500.0f);
					lightChanged |= ImGui::DragFloat(("Decay##point_" + std::to_string(i)).c_str(), &point.decay, 0.01f, 0.0f, 10.0f);
				}
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Spot Lights")) {
		int spotCount = static_cast<int>(editorLightState_.spotLights.size());
		if (!isPlaying_ && ImGui::SliderInt("Spot Count", &spotCount, 0, static_cast<int>(kMaxSpotLights))) {
			editorLightState_.spotLights.resize(static_cast<size_t>(spotCount));
			lightChanged = true;
		}
		for (size_t i = 0; i < editorLightState_.spotLights.size(); ++i) {
			SpotLight& spot = editorLightState_.spotLights[i];
			if (ImGui::TreeNode(("Spot " + std::to_string(i) + "##spot").c_str())) {
				if (!isPlaying_) {
					lightChanged |= ImGui::ColorEdit4(("Color##spot_" + std::to_string(i)).c_str(), &spot.color.x);
					lightChanged |= ImGui::DragFloat3(("Position##spot_" + std::to_string(i)).c_str(), &spot.position.x, 0.05f);
					lightChanged |= ImGui::DragFloat3(("Direction##spot_" + std::to_string(i)).c_str(), &spot.direction.x, 0.01f, -1.0f, 1.0f);
					lightChanged |= ImGui::DragFloat(("Intensity##spot_" + std::to_string(i)).c_str(), &spot.intensity, 0.01f, 0.0f, 10.0f);
					lightChanged |= ImGui::DragFloat(("Distance##spot_" + std::to_string(i)).c_str(), &spot.distance, 0.05f, 0.0f, 500.0f);
					lightChanged |= ImGui::DragFloat(("Decay##spot_" + std::to_string(i)).c_str(), &spot.decay, 0.01f, 0.0f, 10.0f);
					lightChanged |= ImGui::DragFloat(("Cos Angle##spot_" + std::to_string(i)).c_str(), &spot.cosAngle, 0.001f, -1.0f, 1.0f);
					lightChanged |= ImGui::DragFloat(("Cos Falloff##spot_" + std::to_string(i)).c_str(), &spot.cosFalloffStart, 0.001f, -1.0f, 1.0f);
				}
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Area Lights")) {
		int areaCount = static_cast<int>(editorLightState_.areaLights.size());
		if (!isPlaying_ && ImGui::SliderInt("Area Count", &areaCount, 0, static_cast<int>(kMaxAreaLights))) {
			editorLightState_.areaLights.resize(static_cast<size_t>(areaCount));
			lightChanged = true;
		}
		for (size_t i = 0; i < editorLightState_.areaLights.size(); ++i) {
			AreaLight& area = editorLightState_.areaLights[i];
			if (ImGui::TreeNode(("Area " + std::to_string(i) + "##area").c_str())) {
				if (!isPlaying_) {
					lightChanged |= ImGui::ColorEdit4(("Color##area_" + std::to_string(i)).c_str(), &area.color.x);
					lightChanged |= ImGui::DragFloat3(("Position##area_" + std::to_string(i)).c_str(), &area.position.x, 0.05f);
					lightChanged |= ImGui::DragFloat3(("Normal##area_" + std::to_string(i)).c_str(), &area.normal.x, 0.01f, -1.0f, 1.0f);
					lightChanged |= ImGui::DragFloat(("Intensity##area_" + std::to_string(i)).c_str(), &area.intensity, 0.01f, 0.0f, 10.0f);
					lightChanged |= ImGui::DragFloat(("Width##area_" + std::to_string(i)).c_str(), &area.width, 0.05f, 0.0f, 500.0f);
					lightChanged |= ImGui::DragFloat(("Height##area_" + std::to_string(i)).c_str(), &area.height, 0.05f, 0.0f, 500.0f);
					lightChanged |= ImGui::DragFloat(("Radius##area_" + std::to_string(i)).c_str(), &area.radius, 0.05f, 0.0f, 500.0f);
					lightChanged |= ImGui::DragFloat(("Decay##area_" + std::to_string(i)).c_str(), &area.decay, 0.01f, 0.0f, 10.0f);
				}
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}

	if (lightChanged || editorLightState_.overrideSceneLights) {
		hasUnsavedChanges_ = hasUnsavedChanges_ || lightChanged;
		Object3dCommon::GetInstance()->SetEditorLights(
		    editorLightState_.directionalLight, editorLightState_.pointLights.empty() ? nullptr : editorLightState_.pointLights.data(), static_cast<uint32_t>(editorLightState_.pointLights.size()),
		    editorLightState_.spotLights.empty() ? nullptr : editorLightState_.spotLights.data(), static_cast<uint32_t>(editorLightState_.spotLights.size()),
		    editorLightState_.areaLights.empty() ? nullptr : editorLightState_.areaLights.data(), static_cast<uint32_t>(editorLightState_.areaLights.size()));
	}
#endif
}
void Hierarchy::DrawAudioEditor() {
#ifdef USE_IMGUI
	Audio* audio = Audio::GetInstance();
	if (!audio) {
		ImGui::TextUnformatted("Audio system unavailable");
		return;
	}
	auto entries = audio->GetEditorSoundEntries();
	if (entries.empty()) {
		ImGui::TextUnformatted("No tracked sounds.");
		return;
	}
	for (size_t i = 0; i < entries.size(); ++i) {
		auto& entry = entries[i];
		if (!entry.soundData) {
			continue;
		}
		if (ImGui::TreeNode((entry.name + "##audio_" + std::to_string(i)).c_str())) {
			const auto savedIt = savedAudioVolumes_.find(entry.name);
			if (savedIt != savedAudioVolumes_.end()) {
				audio->SetSoundVolume(entry.soundData, savedIt->second);
			}
			float volume = entry.soundData->volume;
			if (ImGui::SliderFloat(("Volume##audio_volume_" + std::to_string(i)).c_str(), &volume, 0.0f, 1.0f)) {
				audio->SetSoundVolume(entry.soundData, volume);
				savedAudioVolumes_[entry.name] = volume;
				hasUnsavedChanges_ = true;
			}
			if (entry.isPlaying) {
				ImGui::Text("Playing (%s)", entry.isLoop ? "Loop" : "One-shot");
			} else {
				ImGui::TextUnformatted("Stopped");
			}
			if (ImGui::Button(("Play##audio_play_" + std::to_string(i)).c_str())) {
				audio->SoundPlayWave(*entry.soundData, false);
			}
			ImGui::SameLine();
			if (ImGui::Button(("Loop##audio_loop_" + std::to_string(i)).c_str())) {
				audio->SoundPlayWave(*entry.soundData, true);
			}
			ImGui::TreePop();
		}
	}
#endif
}

bool Hierarchy::IsObjectSelected() const {
	if (selectedIsPrimitive_) {
		return selectedObjectIndex_ < primitives_.size() && primitives_[selectedObjectIndex_] != nullptr;
	}
	return selectedObjectIndex_ < objects_.size() && objects_[selectedObjectIndex_] != nullptr;
}

Transform Hierarchy::GetSelectedTransform() const {
	if (selectedIsPrimitive_) {
		if (selectedObjectIndex_ < primitiveEditorTransforms_.size()) {
			return primitiveEditorTransforms_[selectedObjectIndex_];
		}
	} else {
		if (selectedObjectIndex_ < editorTransforms_.size()) {
			return editorTransforms_[selectedObjectIndex_];
		}
	}
	return Transform{};
}

void Hierarchy::SyncSelectionBoxToTarget() {
	if (!showSelectionBox_ || !IsObjectSelected()) {
		return;
	}
	if (!selectionBoxPrimitive_) {
		selectionBoxPrimitive_ = std::make_unique<Primitive>();
		selectionBoxPrimitive_->SetEditorRegistrationEnabled(false);
		selectionBoxPrimitive_->Initialize(Primitive::Box);
		selectionBoxPrimitive_->SetEnableLighting(false);
		selectionBoxPrimitive_->SetColor({1.0f, 0.9f, 0.1f, 1.0f});
	}
	selectionBoxPrimitive_->SetCamera(Object3dCommon::GetInstance()->GetDefaultCamera());
	const Transform selectedTransform = GetSelectedTransform();
	selectionBoxPrimitive_->SetTransform(selectedTransform);
}

void Hierarchy::DrawSelectionBoxEditor() {
#ifdef USE_IMGUI
	if (ImGui::Checkbox("Draw Selection Box", &showSelectionBox_)) {
		selectionBoxDirty_ = true;
	}
#endif
}
void Hierarchy::DrawObjectEditors() {
	LoadObjectEditorsFromJsonIfExists("objectEditors.json");
#ifdef USE_IMGUI

	if (!isPlaying_) {
		for (size_t i = 0; i < objects_.size(); ++i) {
			Object3d* object = objects_[i];
			if (!object) {
				continue;
			}
			const Transform& transform = editorTransforms_[i];
			const InspectorMaterial& material = editorMaterials_[i];
			object->SetTransform(transform);
			object->SetColor(material.color);
			object->SetEnableLighting(material.enableLighting);
			object->SetShininess(material.shininess);
			object->SetEnvironmentCoefficient(material.environmentCoefficient);
			object->SetGrayscaleEnabled(material.grayscaleEnabled);
			object->SetSepiaEnabled(material.sepiaEnabled);
			object->SetDistortionStrength(material.distortionStrength);
			object->SetDistortionFalloff(material.distortionFalloff);
			object->SetUvTransform(material.uvScale, material.uvRotate, material.uvTranslate, material.uvAnchor);
		}

		for (size_t i = 0; i < primitives_.size(); ++i) {
			Primitive* primitive = primitives_[i];
			if (!primitive) {
				continue;
			}
			const Transform& transform = primitiveEditorTransforms_[i];
			const InspectorMaterial& material = primitiveEditorMaterials_[i];
			primitive->SetTransform(transform);
			primitive->SetColor(material.color);
			primitive->SetEnableLighting(material.enableLighting);
			primitive->SetShininess(material.shininess);
			primitive->SetEnvironmentCoefficient(material.environmentCoefficient);
			primitive->SetGrayscaleEnabled(material.grayscaleEnabled);
			primitive->SetSepiaEnabled(material.sepiaEnabled);
			primitive->SetDistortionStrength(material.distortionStrength);
			primitive->SetDistortionFalloff(material.distortionFalloff);
			primitive->SetUvTransform(Function::MakeAffineMatrix(material.uvScale, material.uvRotate, material.uvTranslate, material.uvAnchor));
		}
	}
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	const float kTopToolbarHeight = 44.0f;
	const float kLeftPanelRatio = 0.22f;
	const float kRightPanelRatio = 0.24f;
	const float kPanelMinWidth = 260.0f;
	const float availableHeight = std::max(1.0f, viewport->WorkSize.y - kTopToolbarHeight);
	const float leftPanelWidth = std::max(kPanelMinWidth, viewport->WorkSize.x * kLeftPanelRatio);
	const float rightPanelWidth = std::max(kPanelMinWidth, viewport->WorkSize.x * kRightPanelRatio);


	ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, kTopToolbarHeight), ImGuiCond_Always);
	if (ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
		ToolBar::Result toolbarResult = ToolBar::Draw(isPlaying_, hasUnsavedChanges_, !undoStack_.empty(), !redoStack_.empty());
		if (toolbarResult.undoRequested) {
			UndoEditorChange();
			saveStatusMessage_ = "Undo";
		}
		if (toolbarResult.redoRequested) {
			RedoEditorChange();
			saveStatusMessage_ = "Redo";
		}
		if (toolbarResult.playRequested) {
			if (hasUnsavedChanges_) {
				saveStatusMessage_ = "Warning: unsaved changes. Save To JSON before Play";
			} else {
				SceneManager::GetInstance()->RequestReinitializeCurrentScene();
				SetPlayMode(true);
				saveStatusMessage_ = "Playing";
			}
		}
		if (toolbarResult.stopRequested) {
			SetPlayMode(false);
			saveStatusMessage_ = "Stopped: applied editor values";
		}
	}
	ImGui::End();

	const float contentStartY = viewport->WorkPos.y + kTopToolbarHeight;
	ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, contentStartY), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(leftPanelWidth, availableHeight), ImGuiCond_Always);
	if (ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
		ImGui::Text("Auto Object Editor");
		ImGui::Separator();
		ImGui::SeparatorText("Scene Switch");
		DrawSceneSelector();
		ImGui::SeparatorText("Grid");
		DrawGridEditor();
		ImGui::SeparatorText("Light");
		DrawLightEditor();
		ImGui::SeparatorText("Camera");
		DrawCameraEditor();
		ImGui::SeparatorText("Selection");
		DrawSelectionBoxEditor();
		ImGui::Separator();

		if (!isPlaying_ && ImGui::Button("Save To JSON")) {
			const std::string saveFilePath = GetSceneScopedEditorFilePath("objectEditors.json");
			const bool saved = SaveObjectEditorsToJson(saveFilePath);
			if (saved) {
				hasUnsavedChanges_ = false;
			}
			saveStatusMessage_ = saved ? ("Saved: " + saveFilePath) : ("Save failed: " + saveFilePath);
		}
		if (!saveStatusMessage_.empty()) {
			ImGui::Text("%s", saveStatusMessage_.c_str());
		}

		ImGui::SeparatorText("Object3d");
		for (size_t i = 0; i < objects_.size(); ++i) {
			Object3d* object = objects_[i];
			if (!object) {
				continue;
			}
			std::string displayName = objectNames_[i].empty() ? ("Object " + std::to_string(i)) : objectNames_[i];
			const bool selected = (!selectedIsPrimitive_ && selectedObjectIndex_ == i);
			if (ImGui::Selectable((displayName + "##object_select_" + std::to_string(i)).c_str(), selected)) {
				selectedObjectIndex_ = i;
				selectedIsPrimitive_ = false;
				selectionBoxDirty_ = true;
			}
		}

		ImGui::SeparatorText("Primitive");
		for (size_t i = 0; i < primitives_.size(); ++i) {
			Primitive* primitive = primitives_[i];
			if (!primitive) {
				continue;
			}
			std::string displayName = primitiveNames_[i].empty() ? ("Primitive " + std::to_string(i)) : primitiveNames_[i];
			const bool selected = (selectedIsPrimitive_ && selectedObjectIndex_ == i);
			if (ImGui::Selectable((displayName + "##primitive_select_" + std::to_string(i)).c_str(), selected)) {
				selectedObjectIndex_ = i;
				selectedIsPrimitive_ = true;
				selectionBoxDirty_ = true;
			}
		}
	}
	ImGui::End();

	const float inspectorPosX = viewport->WorkPos.x + viewport->WorkSize.x - rightPanelWidth;
	ImGui::SetNextWindowPos(ImVec2(inspectorPosX, contentStartY), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(rightPanelWidth, availableHeight), ImGuiCond_Always);
	if (ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
		if (!IsObjectSelected()) {
			ImGui::TextUnformatted("No object selected.");
		} else {
			EditorSnapshot beforeEdit{};
			beforeEdit.objectTransforms = editorTransforms_;
			beforeEdit.objectMaterials = editorMaterials_;
			beforeEdit.objectNames = objectNames_;
			beforeEdit.primitiveTransforms = primitiveEditorTransforms_;
			beforeEdit.primitiveMaterials = primitiveEditorMaterials_;
			beforeEdit.primitiveNames = primitiveNames_;
			bool transformChanged = false;
			bool materialChanged = false;
			bool nameChanged = false;
			if (selectedIsPrimitive_) {
				Primitive* primitive = primitives_[selectedObjectIndex_];
				if (primitive) {
					Transform& transform = primitiveEditorTransforms_[selectedObjectIndex_];
					InspectorMaterial& material = primitiveEditorMaterials_[selectedObjectIndex_];
					std::string& name = primitiveNames_[selectedObjectIndex_];
					Inspector::DrawPrimitiveInspector(selectedObjectIndex_, name, transform, material, isPlaying_, transformChanged, materialChanged, nameChanged);
					if (transformChanged) {
						selectionBoxDirty_ = true;
						primitive->SetTransform(transform);
					}
					if (materialChanged) {
						primitive->SetColor(material.color);
						primitive->SetEnableLighting(material.enableLighting);
						primitive->SetShininess(material.shininess);
						primitive->SetEnvironmentCoefficient(material.environmentCoefficient);
						primitive->SetGrayscaleEnabled(material.grayscaleEnabled);
						primitive->SetSepiaEnabled(material.sepiaEnabled);
						primitive->SetDistortionStrength(material.distortionStrength);
						primitive->SetDistortionFalloff(material.distortionFalloff);
						primitive->SetUvTransform(material.uvScale, material.uvRotate, material.uvTranslate, material.uvAnchor);
					}
				}
			} else {
				Object3d* object = objects_[selectedObjectIndex_];
				if (object) {
					Transform& transform = editorTransforms_[selectedObjectIndex_];
					InspectorMaterial& material = editorMaterials_[selectedObjectIndex_];
					std::string& name = objectNames_[selectedObjectIndex_];
					Inspector::DrawObjectInspector(selectedObjectIndex_, name, transform, material, isPlaying_, transformChanged, materialChanged, nameChanged);
					if (transformChanged) {
						selectionBoxDirty_ = true;
						object->SetTransform(transform);
					}
					if (materialChanged) {
						object->SetColor(material.color);
						object->SetEnableLighting(material.enableLighting);
						object->SetShininess(material.shininess);
						object->SetEnvironmentCoefficient(material.environmentCoefficient);
						object->SetGrayscaleEnabled(material.grayscaleEnabled);
						object->SetSepiaEnabled(material.sepiaEnabled);
						object->SetDistortionStrength(material.distortionStrength);
						object->SetDistortionFalloff(material.distortionFalloff);
						object->SetUvTransform(material.uvScale, material.uvRotate, material.uvTranslate, material.uvAnchor);
					}
				}
			}
			if (transformChanged || materialChanged || nameChanged) {
				undoStack_.push_back(std::move(beforeEdit));
				redoStack_.clear();
				hasUnsavedChanges_ = true;
			}
		}
	}
	ImGui::End();
#endif
}