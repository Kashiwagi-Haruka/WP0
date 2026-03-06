#include "SceneManager.h"
#include "Engine/Editor/Hierarchy.h"
#include <cassert>

std::unique_ptr<SceneManager> SceneManager::instance_ = nullptr;

SceneManager* SceneManager::GetInstance() {
	if (!instance_) {
		instance_ = std::make_unique<SceneManager>();
	}
	return instance_.get();
}

void SceneManager::RequestReinitializeCurrentScene() { isSceneReinitializeRequested_ = true; }

void SceneManager::Finalize() {

	if (scene_) {
		scene_->Finalize();
		scene_.reset();
	}

	nextscene_.reset();
	currentSceneName_.clear();
	nextSceneName_.clear();

	// ★ Singleton の instance を解放
	instance_.reset();
}

void SceneManager::Update() {

	// シーン再初期化
	if (isSceneReinitializeRequested_ && scene_) {
		scene_->Finalize();
		scene_->Initialize();
		isSceneReinitializeRequested_ = false;
	}

	// シーン切り替え
	if (nextscene_) {

		if (scene_) {
			scene_->Finalize();
		}

		scene_ = std::move(nextscene_);
		currentSceneName_ = nextSceneName_;
		nextSceneName_.clear();
		scene_->SetSceneManager(this);
		scene_->Initialize();
		isSceneReinitializeRequested_ = false;
	}

	if (scene_) {
		scene_->Update();
	}
}

void SceneManager::Draw() {
	if (scene_) {
		scene_->Draw();
	}
}

void SceneManager::ChangeScene(const std::string& sceneName) {

	assert(sceneFactory_);
	assert(nextscene_ == nullptr);

	nextscene_ = sceneFactory_->CreateScene(sceneName);
	nextSceneName_ = sceneName;
}
std::vector<std::string> SceneManager::GetSceneNames() const {
	if (!sceneFactory_) {
		return {};
	}
	return sceneFactory_->GetSceneNames();
}