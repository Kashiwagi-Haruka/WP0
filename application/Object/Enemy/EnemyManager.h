#pragma once
#include "Enemy.h"
#include <memory>
#include <vector>
#include "EnemyHitEffect.h"
class EnemyManager {

public:
	// ウェーブの状態
	enum class WaveState {
		kWaiting,  // 次のウェーブ待機中
		kSpawning, // 敵生成中
		kActive,   // ウェーブ進行中
		kComplete  // ウェーブクリア
	};

private:
	struct HitEffectEntry {
		Enemy* enemy;
		std::unique_ptr<EnemyHitEffect> effect;
	};
	std::vector<std::unique_ptr<Enemy>> enemies;
	std::vector<HitEffectEntry> hitEffects;
	Camera* camera_ = nullptr;

	// ウェーブシステム
	int currentWave_ = 0;        // 現在のウェーブ番号
	WaveState waveState_;        // ウェーブの状態
	float waveTimer_ = 0.0f;     // ウェーブタイマー
	float waveDelay_ = 3.0f;     // ウェーブ間の待機時間（秒）
	int totalEnemiesKilled_ = 0; // 倒した敵の総数
	bool allWavesComplete_ = false;
	int maxWave_ = 5;

public:
	EnemyManager() {}
	~EnemyManager() = default;

		void Initialize(Camera* camera);
	void AddEnemy(Camera* camera, const Vector3& pos);
	void Update(Camera* camera, const Vector3& housePos, const Vector3& houseScale, const Vector3& playerPos, bool isPlayerAlive);
	void Draw();
	void Clear();
	void OnEnemyDamaged(Enemy* enemy);
	// ウェーブシステム関連
	void StartNextWave();     // 次のウェーブを開始
	void SpawnWaveEnemies();  // ウェーブに応じた敵を生成
	void CheckWaveComplete(); // ウェーブクリア判定

	// ゲッター
	int GetCurrentWave() const { return currentWave_; }
	WaveState GetWaveState() const { return waveState_; }
	int GetAliveEnemyCount() const; // 生存している敵の数
	int GetTotalEnemiesKilled() const { return totalEnemiesKilled_; }
	bool IsWaveActive() const;     // ウェーブ進行中か
	bool IsWaveComplete() const;   // ウェーブクリアしたか
	float GetWaveProgress() const; // ウェーブの進行度（0.0～1.0）
	bool AreAllWavesComplete() const { return allWavesComplete_; }
	int GetMaxWave() const { return maxWave_; }

	// セッター
	void SetWaveDelay(float delay) { waveDelay_ = delay; }
	void ForceStartWave(int waveNumber);
	void ForceBossPhase();

	// 敵リストへのアクセス（当たり判定用など）
	std::vector<std::unique_ptr<Enemy>>& GetEnemies() { return enemies; }
};