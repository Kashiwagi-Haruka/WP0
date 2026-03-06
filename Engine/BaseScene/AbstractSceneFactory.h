#pragma once

#include "BaseScene.h"
#include <memory>
#include <string>
#include <vector>

class AbstractSceneFactory {

public:
	virtual ~AbstractSceneFactory() = default;

	virtual std::unique_ptr<BaseScene> CreateScene(const std::string& sceneName) = 0;
	virtual std::vector<std::string> GetSceneNames() const = 0;
};