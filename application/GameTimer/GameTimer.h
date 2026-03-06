#pragma once
#include <memory>
class GameTimer {

	// ゲーム内の経過時間(秒)を保持するタイマー。
	float timer_ = 0.0f;
	// シングルトンの実体。
	static std::unique_ptr<GameTimer> instance;

public:
	// シングルトンのインスタンス取得。
	static GameTimer* GetInstance();
	// タイマーを0に戻す。
	void Reset();
	// 1フレーム分の時間を加算する。
	void Update();
	// 現在のタイマー値(秒)を取得する。
	float GetTimer() { return timer_; };
};