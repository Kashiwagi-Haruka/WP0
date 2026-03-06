#include "PortalManager.h"
#include "Object3d/Object3dCommon.h"
#include "GameObject/GameCamera/PlayerCamera/PlayerCamera.h"
#include "GameObject/KeyBindConfig.h"
#include "GameObject/TimeCard/TimeCardWatch.h"
#include "GameObject/WhiteBoard/WalkWhiteBoard.h"
#include "GameObject/YoshidaMath/YoshidaMath.h"
#include "Model/ModelManager.h"

namespace {
    const constexpr uint32_t kMaxWhiteBoards = 6;
}

PortalManager::PortalManager(Vector3* pos) {

    playerPos_ = pos;

    firstWarpPosTransform_ = { .scale = {1.0f,1.0f,1.0f},.rotate = {0.0f,0.0f,0.0f},.translate = { 10.0f, 1.5f, 5.0f } };

    ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/whiteBoard", "whiteBoard");
    std::unique_ptr<WalkWhiteBoard> walkWhite = std::make_unique<WalkWhiteBoard>();
    WalkWhiteBoard::LoadAnimation("Resources/TD3_3102/3d/whiteBoard", "whiteBoard");
    walkWhite->SetModel("whiteBoard");
    walkWhite->SetTargetPosPtr(pos);
    whiteBoards_.push_back(std::move(walkWhite));

    for (int i = 0; i < kMaxWhiteBoards; ++i) {
        std::unique_ptr<WhiteBoard> white = std::make_unique<WhiteBoard>();
        white->SetModel("whiteBoard");
        whiteBoards_.push_back(std::move(white));
    }

    portalParticle_ = std::make_unique<PortalParticle>();
}

void PortalManager::Initialize() {
    for (auto& board : whiteBoards_) {
        board->Initialize();
    }

    portalParticle_->Initialize();
}

void PortalManager::UpdateWhiteBoard() {
    for (auto& board : whiteBoards_) {
        board->Update();
    }
}

void PortalManager::UpdatePortal() {

    if (isPendingPortalSpawn_ && portalParticle_) {
        portalParticle_->Update();
        if (portalParticle_->IsFinished() && pendingWhiteBoard_) {
            SpawnPortal(pendingWhiteBoard_);
            pendingWhiteBoard_ = nullptr;
            isPendingPortalSpawn_ = false;
        }
    }

    for (auto& portal : portals_) {
        portal->Update();
    }
}
void PortalManager::SetCamera(Camera* camera)
{
    for (auto& board : whiteBoards_) {
        board->SetCamera(camera);
    }

    if (portalParticle_) {
        portalParticle_->SetCamera(camera);
    }
};

void PortalManager::DrawWhiteBoard() {
    for (auto& board : whiteBoards_) {
        board->Draw();
    }
}

void PortalManager::DrawPortal(bool isShadow)
{
    for (auto& portal : portals_) {

        portal->SetCamera(playerCamera_->GetCamera());
        portal->UpdateCameraMatrices();

        if (!isShadow) {
            portal->DrawPortals();
            portal->DrawRings();
        }

        portal->DrawWarpPos();
    }

}

void PortalManager::Draw(bool isShadow, bool drawParticle) {

    DrawWhiteBoard();
    DrawPortal(isShadow);

    if (drawParticle && portalParticle_) {
        portalParticle_->Draw();
    }
}

void PortalManager::SetPlayerCamera(PlayerCamera* camera) {

    playerCamera_ = camera;
}

void PortalManager::CheckCollision(TimeCardWatch* timeCardWatch) {
    
    if (isPendingPortalSpawn_) {
        return;
    }

    // whiteBoardとrayの当たり判定
    for (auto& board : whiteBoards_) {
        if (timeCardWatch->OnCollisionObjOfMakePortal(playerCamera_->GetRay(), board->GetAABB(), board->GetTransform())) {

            if (PlayerCommand::GetInstance()->Shot()) {

                if (preWhiteBoards_.size() >= 2) {
                    //ポータルの生成が2個以上になったら
                    preWhiteBoards_.at(0)->ResetCollisionAttribute();
                    preWhiteBoards_.erase(preWhiteBoards_.begin());
                }

                preWhiteBoards_.push_back(board.get());
 
                preWhiteBoards_.back()->SetCollisionAttribute(kCollisionNone);

                pendingWhiteBoard_ = preWhiteBoards_.back();

                isPendingPortalSpawn_ = true;

                if (portalParticle_) {
                    portalParticle_->Start(*playerPos_, preWhiteBoards_.back()->GetTransform().translate);
                }
            }
            break;
        };
    }
}

void PortalManager::SpawnPortal(WhiteBoard* board) {

    Transform* newWarpTransform = nullptr;


    if (portals_.size() >= 2) {
        //ポータルの生成が2個以上になったら
        portals_.erase(portals_.begin());
    }

    //ポータルがないとき
    if (portals_.empty()) {
        //最初の位置に入れる
        newWarpTransform = &firstWarpPosTransform_;
    } else {
        portals_.at(0)->SetWarpPosParent(&board->GetTransform());
        newWarpTransform = &portals_.at(0)->GetTransform();
    }
    //ポータルを新たに作る
    std::unique_ptr portal = std::make_unique<Portal>();

    portal->Initialize();

    Camera* camera = playerCamera_->GetCamera();
    portal->SetCamera(camera);
    portal->SetParentTransform(&board->GetTransform());
    portal->SetWarpPosParent(newWarpTransform);
    portals_.push_back(std::move(portal));
}