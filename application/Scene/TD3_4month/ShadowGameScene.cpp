#include "ShadowGameScene.h"
#include "Input.h"
#include "SceneManager.h"
#include "Sprite/SpriteCommon.h"
#include"Object3d/Object3dCommon.h"
#include"DirectXCommon.h"
#include<numbers>
#include"RigidBody.h"
#include "WinApp.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"
#include "Particle/ParticleManager.h"

ShadowGameScene::ShadowGameScene()
{
    //シーン遷移の設定
    transition_ = std::make_unique<SceneTransition>();
    //デバックカメラ
    debugCamera_ = std::make_unique<DebugCamera>();
    //プレイヤーの生成
    player_ = std::make_unique<Player>();
    //プレイヤー視点のカメラ
    playerCamera_ = std::make_unique<PlayerCamera>();
    playerCamera_->SetPlayerTransformPtr(&player_->GetTransform());
    //テスト地面
    testField_ = std::make_unique<TestField>();
    //SEを読み込む
    Portal::LoadSE();
    //ホワイトボード管理
    portalManager_ = std::make_unique<PortalManager>(&player_->GetTransform().translate);
    portalManager_->SetPlayerCamera(playerCamera_.get());

    //携帯打刻機
    timeCardWatch_ = std::make_unique<TimeCardWatch>();
    //懐中電灯
    flashlight_ = std::make_unique<Flashlight>();
    // 鍵管理
    key_ = std::make_unique<Key>();
    // 枝豆管理
    edamame_ = std::make_unique<Edamame>();
    //椅子
    chair_ = std::make_unique<Chair>();
    //衝突管理
    collisionManager_ = std::make_unique<CollisionManager>();
}

ShadowGameScene::~ShadowGameScene()
{
    Portal::UnLoadSE();
}

void ShadowGameScene::Initialize()
{
    isPause_ = false;
    noiseTimer_ = kNoiseTimer_;
    isNoise_ = false;

    //シーン遷移の設定
    transition_->Initialize(false);
    isTransitionIn_ = true;
    isTransitionOut_ = false;

    playerCamera_->Initialize();

    //デバックカメラの設定
    debugCamera_->Initialize();
    debugCamera_->SetTranslation(playerCamera_->GetTransform().translate);
    //プレイヤーの初期化
    player_->Initialize();
    //テスト地面
    testField_->Initialize();

    InitializeLights();

    //ホワイトボード管理
    portalManager_->Initialize();
    portalManager_->SetPlayerCamera(playerCamera_.get());

    //携帯打刻機
    timeCardWatch_->Initialize();
    //Playerの座標のポインタを入れる
    timeCardWatch_->SetTransformPtr(&player_->GetTransform());

    // 鍵
    key_->Initialize();
    key_->SetPlayerCamera(playerCamera_.get());

    // 枝豆
    edamame_->Initialize();
    edamame_->SetPlayerCamera(playerCamera_.get());

    //椅子
    chair_->Initialize();
    chair_->SetPlayerCamera(playerCamera_.get());

    SetSceneCameraForDraw(playerCamera_->GetCamera());
}

void ShadowGameScene::Update()
{
    //カーソルを画面中央に設定する
    auto* input = Input::GetInstance();

    if (input->TriggerKey(DIK_TAB)) {
        //Tabキーでポーズ
        isPause_ = (isPause_) ? false : true;

        if (isPause_) {
            input->SetIsCursorVisible(true);
            input->SetIsCursorStability(false);
        } else {
            input->SetIsCursorVisible(false);
            input->SetIsCursorStability(true);
        }
    }

    if (isPause_) {
        return;
    }

    //シーン遷移の更新処理
    UpdateSceneTransition();
    //カメラの更新処理
    UpdateCamera();
    //ライトの更新処理
    UpdateLight();
    //ゲームオブジェクトの更新処理
    UpdateGameObject();

    //オブジェクトの当たり判定
    CheckCollision();
}

void ShadowGameScene::Draw()
{
    //ゲームオブジェクトの描画処理
    DrawModel();

    //スプライト共通
    SpriteCommon::GetInstance()->DrawCommon();
    playerCamera_->DrawRaySprite();
    //シーン遷移の描画処理
    DrawSceneTransition();
}

void ShadowGameScene::Finalize()
{
}

void ShadowGameScene::DebugImGui()
{
#ifdef USE_IMGUI
    ImGui::Begin("shadowGameScene");
    ImGui::End();
#endif // USE_IMGUI
}

void ShadowGameScene::CheckCollision()
{
    //ホワイトボードとrayの当たり判定作成する
    portalManager_->CheckCollision(timeCardWatch_.get());
    key_->CheckCollision();

    edamame_->CheckCollision();
    chair_->CheckCollision();

    collisionManager_->ClearColliders();

    collisionManager_->AddCollider(player_.get());
    collisionManager_->AddCollider(chair_.get());


    for (auto& portal : portalManager_->GetPortals()) {
        collisionManager_->AddCollider(portal.get());
    }

    for (auto& whiteBoard : portalManager_->GetWhiteBoards()) {
        collisionManager_->AddCollider(whiteBoard.get());
    }

    collisionManager_->AddCollider(flashlight_.get());
    collisionManager_->AddCollider(testField_.get());

    collisionManager_->SetCamera(playerCamera_->GetCamera());

    collisionManager_->CheckAllCollisions();
}

void ShadowGameScene::InitializeLights()
{
    //懐中電灯
    flashlight_->Initialize();
    flashlight_->SetCamera(playerCamera_->GetCamera());

    activePointLightCount_ = 2;
    pointLights_[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    pointLights_[0].position = { 0.0f, 5.0f, 0.0f };
    pointLights_[0].intensity = 1.0f;
    pointLights_[0].radius = 10.0f;
    pointLights_[0].decay = 1.0f;
    pointLights_[1].color = { 1.0f, 0.0f, 0.0f, 1.0f };
    pointLights_[1].position = { 5.0f, 5.0f, 5.0f };
    pointLights_[1].intensity = 1.0f;
    pointLights_[1].radius = 10.0f;
    pointLights_[1].decay = 1.0f;

    directionalLight_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    directionalLight_.direction = { 0.0f, 1.0f, 0.0f };
    directionalLight_.intensity = 0.1f;

    activeSpotLightCount_ = 2;
    spotLights_[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    spotLights_[0].position = { 2.0f, 1.25f, 0.0f };
    spotLights_[0].direction = { -1.0f, -1.0f, 0.0f };
    spotLights_[0].intensity = 4.0f;
    spotLights_[0].distance = 7.0f;
    spotLights_[0].decay = 2.0f;
    spotLights_[0].cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
    spotLights_[0].cosFalloffStart = std::cos(std::numbers::pi_v<float> / 4.0f);

    activeAreaLightCount_ = 2;
    areaLights_[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    areaLights_[0].position = { 0.0f, 3.0f, 0.0f };
    areaLights_[0].normal = { 1.0f, -1.0f, 0.0f };
    areaLights_[0].intensity = 4.0f;
    areaLights_[0].width = 2.0f;
    areaLights_[0].height = 2.0f;
    areaLights_[0].radius = 0.1f;
    areaLights_[0].decay = 2.0f;

    areaLights_[1].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    areaLights_[1].position = { -5.0f, 3.0f, 0.0f };
    areaLights_[1].normal = { 1.0f, -1.0f, 0.0f };
    areaLights_[1].intensity = 4.0f;
    areaLights_[1].width = 2.0f;
    areaLights_[1].height = 2.0f;
    areaLights_[1].radius = 0.1f;
    areaLights_[1].decay = 2.0f;
}
#pragma region //private更新処理
void ShadowGameScene::UpdateCamera()
{
    if (useDebugCamera_) {
        debugCamera_->Update();
        playerCamera_->GetCamera()->SetViewProjectionMatrix(debugCamera_->GetViewMatrix(), debugCamera_->GetProjectionMatrix());
    }
    Object3dCommon::GetInstance()->SetDefaultCamera(playerCamera_->GetCamera());
#ifdef USE_IMGUI
    if (ImGui::Begin("Camera")) {
        ImGui::Checkbox("Use Debug Camera (F1)", &useDebugCamera_);
        ImGui::Text("Debug: LMB drag rotate, Shift+LMB drag pan, Wheel zoom");
        if (ImGui::TreeNode("Transform")) {

            if (!useDebugCamera_) {
                auto& playerCameraT = player_->GetTransform();
                ImGui::DragFloat3("Scale", &playerCameraT.scale.x, 0.01f);
                ImGui::DragFloat3("Rotate", &playerCameraT.rotate.x, 0.01f);
                ImGui::DragFloat3("Translate", &playerCameraT.translate.x, 0.01f);
            }
            ImGui::TreePop();
        }
        ImGui::End();
    }

#endif
}

void ShadowGameScene::UpdateSceneTransition()
{
    if (Input::GetInstance()->TriggerKey(DIK_SPACE) && !isTransitionOut_) {
        transition_->Initialize(true);
        isTransitionOut_ = true;
    }
    if (isTransitionIn_ || isTransitionOut_) {
        transition_->Update();
        if (transition_->IsEnd() && isTransitionIn_) {
            isTransitionIn_ = false;
        }
        if (transition_->IsEnd() && isTransitionOut_) {
            //シーンの切り替え
     /*       SceneManager::GetInstance()->ChangeScene("Title");*/
        }
    }
}

void ShadowGameScene::UpdateGameObject()
{
#pragma region//Lightを組み込む
    Object3dCommon::GetInstance()->SetDirectionalLight(directionalLight_);
    Object3dCommon::GetInstance()->SetPointLights(pointLights_.data(), activePointLightCount_);
    Object3dCommon::GetInstance()->SetSpotLights(spotLights_.data(), activeSpotLightCount_);
    Object3dCommon::GetInstance()->SetAreaLights(areaLights_.data(), activeAreaLightCount_);
#pragma endregion

    bool vignetteStrength = true;

    Object3dCommon::GetInstance()->SetFullScreenGrayscaleEnabled(false);
    Object3dCommon::GetInstance()->SetFullScreenSepiaEnabled(false);
    Object3dCommon::GetInstance()->GetDxCommon()->SetVignetteStrength(vignetteStrength);
    Object3dCommon::GetInstance()->SetVignetteStrength(vignetteStrength);

    if (PlayerCommand::GetInstance()->Shot()) {
        if (!isNoise_) {
            isNoise_ = true;
        }
    }
    if (isNoise_) {
        float randomNoiseScale = 1.0f;
        noiseTimer_ -= YoshidaMath::kDeltaTime;
        if (noiseTimer_ <= 0.0f) {
            isNoise_ = false;
            noiseTimer_ = kNoiseTimer_;
        }

    }
    BlendMode randomNoiseBlendMode = kBlendModeSub;

    Object3dCommon::GetInstance()->SetRandomNoiseEnabled(isNoise_);
    Object3dCommon::GetInstance()->SetRandomNoiseScale(noiseTimer_);
    Object3dCommon::GetInstance()->SetRandomNoiseBlendMode(randomNoiseBlendMode);

#pragma region//ゲームオブジェクト

    for (auto& portal : portalManager_->GetPortals()) {
        if (portal->GetIsPlayerHit()) {
            Transform* portalTransform = portal->GetWarpPosParent();
            player_->SetTranslate(portalTransform->translate + playerCamera_->GetRay().diff);
            player_->SetRotate(portalTransform->rotate);
            break;
        }

    }


    if (!useDebugCamera_) {
        playerCamera_->Update();
    }

    timeCardWatch_->Update();

    player_->Update();

    testField_->Update();

    portalManager_->UpdateWhiteBoard();
    portalManager_->UpdatePortal();
    ParticleManager::GetInstance()->Update(playerCamera_->GetCamera());
    Object3dCommon::GetInstance()->SetDefaultCamera(playerCamera_->GetCamera());

    key_->Update();
    edamame_->Update();
    chair_->Update();

#pragma endregion
}
void ShadowGameScene::UpdateLight()
{

    //懐中電灯
    flashlight_->Update();
    spotLights_[1] = flashlight_->GetSpotLight();


#ifdef USE_IMGUI
    if (ImGui::TreeNode("PointLight")) {
        ImGui::ColorEdit4("PointLightColor", &pointLights_[0].color.x);
        ImGui::DragFloat("PointLightIntensity", &pointLights_[0].intensity, 0.1f);
        ImGui::DragFloat3("PointLightPosition", &pointLights_[0].position.x, 0.1f);
        ImGui::DragFloat("PointLightRadius", &pointLights_[0].radius, 0.1f);
        ImGui::DragFloat("PointLightDecay", &pointLights_[0].decay, 0.1f);
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("PointLight1")) {
        ImGui::ColorEdit4("PointLightColor1", &pointLights_[1].color.x);
        ImGui::DragFloat("PointLightIntensity1", &pointLights_[1].intensity, 0.1f);
        ImGui::DragFloat3("PointLightPosition1", &pointLights_[1].position.x, 0.1f);
        ImGui::DragFloat("PointLightRadius1", &pointLights_[1].radius, 0.1f);
        ImGui::DragFloat("PointLightDecay1", &pointLights_[1].decay, 0.1f);
        ImGui::TreePop();
    }
#endif
}
#pragma endregion

#pragma region //private描画処理
void ShadowGameScene::DrawSceneTransition()
{
    if (isTransitionIn_ || isTransitionOut_) {
        transition_->Draw();
    }
}

void ShadowGameScene::DrawModel()
{
    Object3dCommon::GetInstance()->BeginShadowMapPass();
    Object3dCommon::GetInstance()->DrawCommonShadow();

    DrawGameObject(true, false);

    Object3dCommon::GetInstance()->EndShadowMapPass();

    for (auto& portal : portalManager_->GetPortals()) {
        portal->RenderPortalTextures([this](Camera* camera) {
            Object3dCommon::GetInstance()->SetDefaultCamera(camera);
            SetSceneCameraForDraw(camera);
            DrawSceneGeometry(false);
            });
    }

    Object3dCommon::GetInstance()->GetDxCommon()->SetMainRenderTarget();

    Object3dCommon::GetInstance()->SetDefaultCamera(playerCamera_->GetCamera());
    SetSceneCameraForDraw(playerCamera_->GetCamera());
    DrawSceneGeometry(true);
}
void ShadowGameScene::DrawGameObject(bool isShadow, bool isDrawParticle)
{
    // テスト地面
    testField_->Draw();
    //携帯打刻機の描画処理
    timeCardWatch_->Draw();
    //懐中電灯
    flashlight_->Draw();
    collisionManager_->DrawColliders();
    // 鍵の描画処理
    key_->Draw();
    // 枝豆の描画処理
    edamame_->Draw();
    //椅子の描画
    chair_->Draw();
    //ポータル管理の描画
    portalManager_->Draw(isShadow, isDrawParticle);

    if (!isShadow) {
        Object3dCommon::GetInstance()->DrawCommonSkinning();
    }

    //プレイヤーの描画処理
    player_->Draw();

}
void ShadowGameScene::DrawSceneGeometry(bool drawPortalParticle) {

    Object3dCommon::GetInstance()->DrawCommon();
    //影じゃない
    DrawGameObject(false, drawPortalParticle);
}

void ShadowGameScene::SetSceneCameraForDraw(Camera* camera)
{
    player_->SetCamera(camera);
    testField_->SetCamera(camera);
    portalManager_->SetCamera(camera);
    timeCardWatch_->SetCamera(camera);
    flashlight_->SetCamera(camera);
    key_->SetCamera(camera);
    edamame_->SetCamera(camera);
    chair_->SetCamera(camera);
}
#pragma endregion