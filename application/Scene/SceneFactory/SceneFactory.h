#pragma once
#include "AbstractSceneFactory.h"
#include <vector>
class SceneFactory : public AbstractSceneFactory {

	std::unique_ptr<BaseScene> CreateScene(const std::string& sceneName) override;
	std::vector<std::string> GetSceneNames() const override;

public:
	SceneFactory() = default;
	~SceneFactory() = default;
};