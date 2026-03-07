#pragma once
#include "CSVManager.h"
#include "Transform.h"
#include "Vector3.h"
#include <memory>
#include <vector>
class Object3d;
class GameBase;
class Camera;

class MapchipField {

	static const int kWidth = 20;
	static const int kHeight = 20;
	static const float kTileSize;

	int field[kHeight][kWidth];

	// ★ タイル用の Object3d をまとめて持つ
	std::unique_ptr<Object3d> fieldObj = nullptr;

	Camera* camera_ = nullptr;
	Transform transform_;
	Vector3 initPos_ = {50, 50, 0};

public:
	MapchipField();
	~MapchipField();

	void Initialize(Camera* camera);
	void Update();
	void Draw();
	void SetCamera(Camera* camera) { camera_ = camera; }
	bool IsWall(int x, int y) const;
	void LoadFromCSV(const std::string& filename);
	static void WorldToTile(const Vector3& pos, int& tx, int& ty);
};
