#include "MapchipField.h"
#include "GameBase.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3d.h"

const float MapchipField::kTileSize = 1.0f;

MapchipField::MapchipField() {

	// モデル読み込み（map.obj）
	ModelManager::GetInstance()->LoadModel("Resources/3d", "map");
	fieldObj = std::make_unique<Object3d>();
}

MapchipField::~MapchipField() {}

void MapchipField::Initialize(Camera* camera) {

	camera_ = camera;

	transform_ = {
	    .scale{100.0f,  10.0f, 100.0f },
        .rotate{0.0f,    0.0f,  0.0f   },
        .translate{0.0f, -9.5f, 0.0f}
    };

	fieldObj->Initialize();
	fieldObj->SetModel("map");
	fieldObj->SetCamera(camera_);
	fieldObj->SetTransform(transform_);
}
void MapchipField::LoadFromCSV(const std::string& filename) {
	CSVManager::GetInstance()->LoadCSV(filename);
	const auto& csv = CSVManager::GetInstance()->GetData();

	for (int y = 0; y < kHeight; y++) {
		for (int x = 0; x < kWidth; x++) {
			if (y < csv.size() && x < csv[y].size()) {
				field[y][x] = csv[y][x];
			} else {
				field[y][x] = 0;
			}
		}
	}
}

void MapchipField::Update() { fieldObj->Update(); }

void MapchipField::Draw() { fieldObj->Draw(); }

bool MapchipField::IsWall(int x, int y) const {
	if (x < 0 || y < 0 || x >= kWidth || y >= kHeight)
		return true;
	return field[y][x] == 1;
}

void MapchipField::WorldToTile(const Vector3& pos, int& tx, int& ty) {

	// x=横 (右+)
	tx = (int)floor(pos.x / kTileSize);

	// y=縦 (上+)
	ty = (int)floor(pos.y / kTileSize);
}
