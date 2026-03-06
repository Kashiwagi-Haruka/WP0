#pragma once
#include <map>
#include <memory>
#include <string>

class Model;
class DirectXCommon;

class ModelManager {

	static std::unique_ptr<ModelManager> instance;

	ModelManager(ModelManager&) = delete;
	ModelManager& operator=(ModelManager&) = delete;

	std::map<std::string, std::unique_ptr<Model>> models;
	struct ModelSource {
		std::string directoryPath;
		std::string filePath;
		bool isGltf = false;
	};
	std::map<std::string, ModelSource> modelSources;

public:
	ModelManager() = default;
	~ModelManager() = default;
	static ModelManager* GetInstance();

	void Initialize(DirectXCommon* dxCommon);
	void LoadModel(const std::string& directionalPath, const std::string& filePath);
	void LoadGltfModel(const std::string& directionalPath, const std::string& filePath);
	Model* FindModel(const std::string& filePath);
	std::unique_ptr<Model> CreateModelInstance(const std::string& filePath);
	void Finalize();
};