#pragma once
class SceneManager;
class BaseScene {

	SceneManager* sceneManager_ = nullptr;

	protected:
	bool isSceneEnd_ = false;

	public: 

	virtual ~BaseScene() = default;
	virtual void Initialize()=0;
	virtual void Update()=0;
	virtual void Draw()=0;
	virtual void Finalize()=0;
	virtual void SetSceneManager(SceneManager* sceneManager) { sceneManager_ = sceneManager; }
	virtual bool GetIsSceneEnd() { return isSceneEnd_; };
};
