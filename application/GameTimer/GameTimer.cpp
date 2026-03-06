#include "GameTimer.h"
// シングルトンの実体。
std::unique_ptr<GameTimer> GameTimer::instance = nullptr;

// シングルトンのインスタンスを取得する。
GameTimer* GameTimer::GetInstance() {

	if (instance == nullptr) {
		instance = std::make_unique<GameTimer>();
	}
	return instance.get();
}

// タイマーを0に戻す。
void GameTimer::Reset() { timer_ = 0.0f; }

// 1フレーム分の時間を加算する。
void GameTimer::Update() { timer_ += 1.0f / 60.0f; }