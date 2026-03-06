#pragma once
#include "BaseScene.h"
#include <memory>
#include <imgui.h>
#include "SceneTransition/SceneTransition.h"

#include "DebugCamera.h"
#pragma region //GameObject
#include"GameObject/Player/Player.h"
#include"GameObject/TestField/TestField.h"
#include"GameObject/GameCamera/PlayerCamera/PlayerCamera.h"
#include"GameObject/Portal/PortalManager.h"
#include"GameObject/TimeCard/TimeCardWatch.h"
#include"GameObject/Flashlight/Flashlight.h"
#include"GameObject/Key/Key.h"
#include <GameObject/Chair/Chair.h>
#pragma endregion

#include"GameObject/YoshidaMath/CollisionManager/CollisionManager.h"

#include "Light/DirectionalLight.h" 
#include "Light/PointLight.h"
#include "Light/SpotLight.h" 
#include "Light/AreaLight.h"
#include"Audio.h"
#include <GameObject/Edamame/Edamame.h>


class ShadowGameScene : public BaseScene
{
private:

    bool isPause_ = false;
    const float kNoiseTimer_ = 0.5f;
    float noiseTimer_ = kNoiseTimer_;
    bool isNoise_ = false;

#pragma region//カメラの設定

    std::unique_ptr<PlayerCamera> playerCamera_ = nullptr;
    std::unique_ptr<DebugCamera> debugCamera_ = nullptr;
    bool useDebugCamera_ = false;
#pragma endregion

#pragma region//シーン遷移の設定
    //シーン遷移の設定
    std::unique_ptr<SceneTransition> transition_ = nullptr;
    //遷移入り
    bool isTransitionIn_ = false;
    //遷移抜け
    bool isTransitionOut_ = false;
#pragma endregion

#pragma region//ゲームオブジェクトの設定
    //Player
    std::unique_ptr<Player> player_ = nullptr;
    //TestField
    std::unique_ptr<TestField> testField_ = nullptr;
    //ポータル管理
    std::unique_ptr<PortalManager> portalManager_ = nullptr;
    //携帯打刻機
    std::unique_ptr<TimeCardWatch> timeCardWatch_ = nullptr;
    //懐中電灯
    std::unique_ptr<Flashlight> flashlight_ = nullptr;
    //鍵
    std::unique_ptr<Key> key_ = nullptr;
    //枝豆
    std::unique_ptr<Edamame> edamame_ = nullptr;
    //椅子
    std::unique_ptr<Chair> chair_ = nullptr;
#pragma endregion
    //衝突管理
    std::unique_ptr<CollisionManager> collisionManager_ = nullptr;

#pragma region// light
    //DirectionalLight
    DirectionalLight directionalLight_{};
    //PointLight
    std::array<PointLight, kMaxPointLights> pointLights_{};
    uint32_t activePointLightCount_ = 0;
    //SpotLight
    std::array<SpotLight, kMaxSpotLights> spotLights_{};
    uint32_t activeSpotLightCount_ = 0;
    //AreaLight
    std::array<AreaLight, kMaxAreaLights> areaLights_{};
    uint32_t activeAreaLightCount_ = 0;
#pragma endregion
public:
    //シーンのコンストラクタ
    ShadowGameScene();
    //デストラクタ
    ~ShadowGameScene() override;
    //初期化処理
    void Initialize() override;
    //更新処理
    void Update() override;
    //描画処理
    void Draw() override;
    //終了処理
    void Finalize() override;
    //デバック
    void DebugImGui();
    //衝突判定チェック
    void CheckCollision();
private:
    // =======================================
    // プライベート初期化
    // =======================================
    void InitializeLights();
    // =======================================
    // プライベート更新処理
    // =======================================
    //カメラの更新処理
    void UpdateCamera();
    //シーン遷移の更新処理
    void UpdateSceneTransition();
    //ゲームオブジェクトの更新処理
    void UpdateGameObject();
    //ポイントライトの更新処理
    void UpdateLight();
    // =======================================
    // プライベート描画処理
    // =======================================
    //シーン遷移の描画処理
    void DrawSceneTransition();
    //ゲームオブジェクトの描画処理
    void DrawModel();
    void DrawGameObject(bool isShadow, bool isDrawParticle);
    void DrawSceneGeometry(bool drawPortalParticle = true);
    void SetSceneCameraForDraw(Camera* camera);
};

