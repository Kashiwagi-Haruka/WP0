#include "SceneFactory.h"
#include "Scene/GameOverScene.h"
#include "Scene/GameScene.h"
#include "Scene/ResultScene.h"
#include "Scene/SampleScene/SampleScene.h"
#include "Scene/TitleScene.h"
#include <memory>
std::unique_ptr<BaseScene> SceneFactory::CreateScene(const std::string& sceneName) {
	std::unique_ptr<BaseScene> scene = nullptr;

	if (sceneName == "Title") {
		scene = std::make_unique<TitleScene>();
	} else if (sceneName == "Game") {
		scene = std::make_unique<GameScene>();
	} else if (sceneName == "Result") {
		scene = std::make_unique<ResultScene>();
	} else if (sceneName == "GameOver") {
		scene = std::make_unique<GameOverScene>();
	} else if (sceneName == "Sample") {
		scene = std::make_unique<SampleScene>();
	}
	return scene;
}
std::vector<std::string> SceneFactory::GetSceneNames() const {
	return {
	    "Title", "Game", "Result", "GameOver", "Sample",
	};
}