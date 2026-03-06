#pragma once
#include "GameObject/Portal/Portal.h"
#include "GameObject/Portal/PortalParticle.h"
#include "GameObject/WhiteBoard/WhiteBoard.h"
#include "Vector3.h"
#include"Transform.h"
#include <array>

class TimeCardWatch;
class PlayerCamera;
class PortalManager {
public:
	PortalManager(Vector3* pos);
	void Initialize();
	void UpdateWhiteBoard();
	void UpdatePortal();
	void SetCamera(Camera* camera);
	void Draw(bool isShadow, bool drawParticle = true);
	void SetPlayerCamera(PlayerCamera* playerCamera);
	/// @brief 作成できるポータル地点との当たり判定を作成する
	/// @param timeCardWatch 携帯打刻機
	/// @param camera かめら
	/// @param warpPos ワープ地点の設定をする
	void CheckCollision(TimeCardWatch* timeCardWatch);
	std::vector<std::unique_ptr<Portal>>& GetPortals() { return portals_; };
	std::vector<std::unique_ptr<WhiteBoard>>& GetWhiteBoards() { return whiteBoards_; }

private:

	void SpawnPortal(WhiteBoard* board);
	void DrawWhiteBoard();
	void DrawPortal(bool isShadow);
	Transform firstWarpPosTransform_ = { 0.0f };

	std::vector<std::unique_ptr<WhiteBoard>> whiteBoards_;
	std::vector<std::unique_ptr<Portal>> portals_;
	std::unique_ptr<PortalParticle> portalParticle_ = nullptr;
	PlayerCamera* playerCamera_ = nullptr;
	Vector3* playerPos_ = nullptr;
	std::vector<WhiteBoard*> preWhiteBoards_;
	WhiteBoard* pendingWhiteBoard_ = nullptr;
	bool isPendingPortalSpawn_ = false;
};