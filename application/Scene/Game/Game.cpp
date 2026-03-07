#include "Game.h"
#include "SceneManager.h"

void Game::Initialize() {
	FrameWork::Initialize();
	GameBase::GetInstance()->Initialize(L"LE2A_04_カシワギハルカ_氷華", 1280, 720);

	SetUnhandledExceptionFilter(GameBase::GetInstance()->ExportDump);

	sceneFactory_ = std::make_unique<SceneFactory>();
	SceneManager::GetInstance()->SetSceneFactory(sceneFactory_.get());
	/*SceneManager::GetInstance()->ChangeScene("Title");*/
	SceneManager::GetInstance()->ChangeScene("Sample");
	/*SceneManager::GetInstance()->ChangeScene("Game");*/
	/*SceneManager::GetInstance()->ChangeScene("Result");*/
	/*SceneManager::GetInstance()->ChangeScene("Tutorial");*/
}

void Game::Update() {
	FrameWork::Update();
	GameBase::GetInstance()->BeginFlame();

	SceneManager::GetInstance()->Update();
	if (!GameBase::GetInstance()->ProcessMessage()) {
		endRequest_ = true;
	}
}

void Game::Draw() {
	SceneManager::GetInstance()->Draw();

	// ゲームの処理
	GameBase::GetInstance()->EndFlame();
}

void Game::Finalize() {

	SceneManager::GetInstance()->Finalize();
	GameBase::GetInstance()->Finalize();
	d3dResourceLeakChecker.LeakChecker();
	CoUninitialize();
	FrameWork::Finalize();
}