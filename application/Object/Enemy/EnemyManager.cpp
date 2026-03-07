#include "EnemyManager.h"
#include "Function.h"
#include <algorithm>
#include <cstdlib>
#include "Object3d/Object3dCommon.h"

namespace {
constexpr int kFinalWave = 5;
}
void EnemyManager::Clear() {
	enemies.clear(); // unique_ptr が自動削除
	hitEffects.clear();
}

void EnemyManager::Initialize(Camera* camera) {
	Clear();
	camera_ = camera;

	// ウェーブシステムの初期化
	currentWave_ = 0;
	waveState_ = WaveState::kWaiting;
	waveTimer_ = 0.0f;
	waveDelay_ = 3.0f; // ウェーブ間の待機時間（秒）
	totalEnemiesKilled_ = 0;
	allWavesComplete_ = false;

	// 最初のウェーブを開始
	StartNextWave();
}

void EnemyManager::StartNextWave() {
	if (allWavesComplete_) {
		return;
	}
	currentWave_++;
	waveState_ = WaveState::kSpawning;
	waveTimer_ = 0.0f;

	// 現在のウェーブに基づいて敵を生成
	SpawnWaveEnemies();
}

void EnemyManager::SpawnWaveEnemies() {

	// ウェーブごとの設定
	struct WaveConfig {
		int enemyCount;    // 敵の数
		float startX;      // 開始X位置
		float endX;        // 終了X位置
		float minY;        // 最小Y位置
		float maxY;        // 最大Y位置
		bool randomHeight; // ランダムな高さにするか
		float minZ;        // Z最小
		float maxZ;        // Z最大
	};

	WaveConfig config;

		switch (currentWave_) {
	case 1: // ウェーブ1: 少数、低い位置、広い間隔
		config = {5, -40.0f, 0.0f, 1.5f, 2.0f, false, -60.0f, -20.0f};
		break;

	case 2: // ウェーブ2: 中数、やや高い位置
		config = {8, -40.0f, 0.0f, 1.5f, 3.0f, true, -60.0f, -20.0f};
		break;

	case 3: // ウェーブ3: 多数、バラバラの高さ
		config = {12, -40.0f, 0.0f, 1.5f, 4.0f, true, -60.0f, -20.0f};
		break;

	case 4: // ウェーブ4: 密集、高低差大
		config = {15, -40.0f, 0.0f, 1.0f, 5.0f, true, -60.0f, -20.0f};
		break;

	case 5: // ウェーブ5: 大量、ランダム配置
		config = {20, -40.0f, 0.0f, 1.0f, 6.0f, true, -60.0f, -20.0f};
		break;

	default: // ウェーブ6以降: どんどん増える
		config = {
		    15 + (currentWave_ - 5) * 3, // 徐々に増加
		    10.0f,
		    0.0f + (currentWave_ - 5) * 5.0f,
		    1.0f,
		    6.0f,
		    true,
		    -60.0f,
		    -20.0f};
		break;
	}

	// 敵を生成
	float spacing = (config.endX - config.startX) / config.enemyCount;
	const float minDistance = 2.0f;
	const float minDistanceSq = minDistance * minDistance;
	const int maxAttempts = 40;
	std::vector<Vector3> spawnPositions;
	spawnPositions.reserve(config.enemyCount);

	for (int i = 0; i < config.enemyCount; i++) {
		Vector3 pos = {};
		bool placed = false;
		const float baseX = config.startX + i * spacing;

		for (int attempt = 0; attempt < maxAttempts; ++attempt) {
			float x = baseX;
			if (attempt > 0) {
				x = config.startX + ((float)rand() / RAND_MAX) * (config.endX - config.startX);
			}

			float y;
			if (config.randomHeight) {
				// ランダムな高さ
				y = config.minY + ((float)rand() / RAND_MAX) * (config.maxY - config.minY);
			} else {
				// 固定の高さ
				y = config.minY;
			}

			// ランダムなZ位置のバリエーション
			float z = config.minZ + ((float)rand() / RAND_MAX) * (config.maxZ - config.minZ);

			Vector3 candidate = {x, y, z};
			bool overlaps = false;
			for (const auto& existing : spawnPositions) {
				Vector3 delta = candidate - existing;
				float distanceSq = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
				if (distanceSq < minDistanceSq) {
					overlaps = true;
					break;
				}
			}

			if (!overlaps) {
				pos = candidate;
				placed = true;
				break;
			}
		}

		if (!placed) {
			pos = {baseX, config.minY, (config.minZ + config.maxZ) * 0.5f};
		}

		spawnPositions.push_back(pos);
		AddEnemy(camera_, pos);
	}

	waveState_ = WaveState::kActive;
}

void EnemyManager::AddEnemy(Camera* camera, const Vector3& pos) {
	auto e = std::make_unique<Enemy>();
	e->Initialize(camera, pos);
	e->SetCamera(camera);
	Enemy* enemyPtr = e.get();
	enemies.push_back(std::move(e));

	auto hitEffect = std::make_unique<EnemyHitEffect>();
	hitEffect->SetCamera(camera);
	hitEffect->Initialize();
	hitEffects.push_back({enemyPtr, std::move(hitEffect)});
}
void EnemyManager::Update(Camera* camera, const Vector3& housePos, const Vector3& houseScale, const Vector3& playerPos, bool isPlayerAlive) {

	// ウェーブの状態管理
	switch (waveState_) {
	case WaveState::kWaiting:
		// 待機中（次のウェーブまでの猶予時間）
		waveTimer_ += 1.0f / 60.0f;
		if (waveTimer_ >= waveDelay_) {
			StartNextWave();
		}
		break;

	case WaveState::kActive:
		// ウェーブ進行中：全滅チェック
		CheckWaveComplete();
		break;

	case WaveState::kComplete:
		// ウェーブクリア：次のウェーブへ
		if (!allWavesComplete_) {
			waveState_ = WaveState::kWaiting;
			waveTimer_ = 0.0f;
		}
		break;

	default:
		break;
	}

	// 敵の更新
	for (auto& e : enemies) {
		if (e->GetIsAlive() || e->IsDying()) {
			e->SetCamera(camera);
			e->Update(housePos, houseScale, playerPos, isPlayerAlive);
		}
	}
	for (auto& entry : hitEffects) {
		if (entry.enemy && entry.enemy->GetIsAlive()) {
			entry.effect->SetPosition(entry.enemy->GetPosition());
		}
		entry.effect->Update();
	}
}

void EnemyManager::CheckWaveComplete() {
	// 生存している敵の数をカウント
	int aliveCount = 0;
	for (auto& e : enemies) {
		if (e->GetIsAlive()) {
			aliveCount++;
		}
	}

	// 全滅したらウェーブクリア
	if (aliveCount == 0 && !enemies.empty()) {
		waveState_ = WaveState::kComplete;
		totalEnemiesKilled_ += static_cast<int>(enemies.size());
		if (currentWave_ >= kFinalWave) {
			allWavesComplete_ = true;
		}

		// 倒した敵をクリア
		enemies.clear();
	}
}

void EnemyManager::Draw() {
	for (auto& e : enemies) {
		if (e->GetIsAlive() || e->IsDying()) {
			e->Draw();
		}
	}
	Object3dCommon::GetInstance()->DrawCommonNoCullDepth();
	for (auto& entry : hitEffects) {
		entry.effect->Draw();
	}
	Object3dCommon::GetInstance()->DrawCommon();
}

void EnemyManager::OnEnemyDamaged(Enemy* enemy) {
	for (auto& entry : hitEffects) {
		if (entry.enemy == enemy) {
			entry.effect->Activate(enemy->GetPosition());
			break;
		}
	}
}
void EnemyManager::ForceStartWave(int waveNumber) {
	Clear();
	allWavesComplete_ = false;
	waveTimer_ = 0.0f;
	currentWave_ = std::clamp(waveNumber, 1, maxWave_);
	waveState_ = WaveState::kSpawning;
	SpawnWaveEnemies();
}

void EnemyManager::ForceBossPhase() {
	Clear();
	allWavesComplete_ = true;
	waveTimer_ = 0.0f;
	currentWave_ = maxWave_;
	waveState_ = WaveState::kComplete;
}

int EnemyManager::GetAliveEnemyCount() const {
	int count = 0;
	for (const auto& e : enemies) {
		if (e->GetIsAlive()) {
			count++;
		}
	}
	return count;
}

bool EnemyManager::IsWaveActive() const { return waveState_ == WaveState::kActive; }

bool EnemyManager::IsWaveComplete() const { return waveState_ == WaveState::kComplete; }

float EnemyManager::GetWaveProgress() const {
	if (enemies.empty())
		return 1.0f;

	int aliveCount = GetAliveEnemyCount();
	int totalCount = static_cast<int>(enemies.size());

	return 1.0f - ((float)aliveCount / totalCount);
}