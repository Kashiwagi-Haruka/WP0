#pragma once
#include "Light/AreaLight.h"
#include "Light/DirectionalLight.h"
#include "Light/PointLight.h"
#include "Light/SpotLight.h"
#include "Matrix4x4.h"
#include "PSO/CreatePSO.h"
#include <Windows.h>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <wrl.h>
class Camera;
class DirectXCommon;

// 3Dオブジェクト描画に必要なPSO・ライト・描画共通状態を一元管理するシングルトン
class Object3dCommon {

private:
	// シングルトン本体
	static std::unique_ptr<Object3dCommon> instance;

	// DrawCommon系で利用する既定カメラ
	Camera* defaultCamera = nullptr;

	// DirectX共通機能への参照
	DirectXCommon* dxCommon_ = nullptr;

	// DirectX API呼び出し結果（主に初期化時の一時保持）
	HRESULT hr_;

	// 現在のブレンドモード
	BlendMode blendMode_ = BlendMode::kBlendModeNone;
	// ブレンド状態の管理クラス
	BlendModeManager blendModeManager_;

	// 標準描画用PSO
	std::unique_ptr<CreatePSO> pso_;
	// トゥーン描画用PSO
	std::unique_ptr<CreatePSO> psoToon_;
	// エミッシブ描画用PSO
	std::unique_ptr<CreatePSO> psoEmissive_;
	// カリングなし描画用PSO
	std::unique_ptr<CreatePSO> psoNoCull_;
	// 深度なし描画用PSO
	std::unique_ptr<CreatePSO> psoNoDepth_;
	// カリングなし＋深度なし描画用PSO
	std::unique_ptr<CreatePSO> psoNoCullDepth_;
	// ワイヤーフレーム描画用PSO
	std::unique_ptr<CreatePSO> psoWireframe_;
	// ワイヤーフレーム＋深度なし描画用PSO
	std::unique_ptr<CreatePSO> psoWireframeNoDepth_;
	// ライン描画用PSO
	std::unique_ptr<CreatePSO> psoLine_;
	// ライン＋深度なし描画用PSO
	std::unique_ptr<CreatePSO> psoLineNoDepth_;
	// エディタグリッド描画用PSO
	std::unique_ptr<CreatePSO> psoEditorGrid_;
	// スキニング描画用PSO
	std::unique_ptr<CreatePSO> psoSkinning_;
	// スキニング＋トゥーン描画用PSO
	std::unique_ptr<CreatePSO> psoSkinningToon_;
	// ミラー描画用PSO
	std::unique_ptr<CreatePSO> psoMirror_;
	// ポータル描画用PSO
	std::unique_ptr<CreatePSO> psoPortal_;
	// シャドウマップ生成用PSO
	std::unique_ptr<CreatePSO> psoShadow_;

	// Directional Light（共通）
	DirectionalLight* directionalLightData_ = nullptr;
	// ディレクショナルライト定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_ = nullptr;
	// ポイントライト配列へのCPU書き込みポインタ
	PointLight* pointlightData_ = nullptr;
	// ポイントライト構造化バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource_ = nullptr;
	// ポイントライト数へのCPU書き込みポインタ
	PointLightCount* pointLightCountData_ = nullptr;
	// ポイントライト数の定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightCountResource_ = nullptr;
	// ポイントライト配列のSRVインデックス
	uint32_t pointLightSrvIndex_ = 0;
	// スポットライト配列へのCPU書き込みポインタ
	SpotLight* spotLightData_ = nullptr;
	// スポットライト構造化バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> spotLightResource_ = nullptr;
	// スポットライト数へのCPU書き込みポインタ
	SpotLightCount* spotLightCountData_ = nullptr;
	// スポットライト数の定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> spotLightCountResource_ = nullptr;
	// スポットライト配列のSRVインデックス
	uint32_t spotLightSrvIndex_ = 0;
	// エリアライト配列へのCPU書き込みポインタ
	AreaLight* areaLightData_ = nullptr;
	// エリアライト構造化バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> areaLightResource_ = nullptr;
	// エリアライト数へのCPU書き込みポインタ
	AreaLightCount* areaLightCountData_ = nullptr;
	// エリアライト数の定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> areaLightCountResource_ = nullptr;
	// エリアライト配列のSRVインデックス
	uint32_t areaLightSrvIndex_ = 0;
	// 環境マップのSRVインデックス
	uint32_t environmentMapSrvIndex_ = 0;
	// シャドウマップのSRVインデックス
	uint32_t shadowMapSrvIndex_ = 0;
	// 現在利用中の環境マップパス
	std::string environmentMapPath_;
	// シャドウマップ深度テクスチャ
	Microsoft::WRL::ComPtr<ID3D12Resource> shadowMapResource_ = nullptr;
	// シャドウマップ描画用DSVヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> shadowDsvHeap_ = nullptr;
	// シャドウマップ描画用ビューポート
	D3D12_VIEWPORT shadowViewport_{};
	// シャドウマップ描画用シザー矩形
	D3D12_RECT shadowScissorRect_{};
	// シャドウマップ解像度
	static constexpr uint32_t kShadowMapSize_ = 2048;
	// シャドウマップパス中かどうかのフラグ
	bool isShadowMapPassActive_ = false;
	// シャドウ計算用ライト位置
	Vector3 shadowLightPosition_ = {0.0f, 80.0f, 0.0f};
	// シャドウ投影用近クリップ距離
	float shadowCameraNear_ = 0.1f;
	// シャドウ投影用遠クリップ距離
	float shadowCameraFar_ = 300.0f;
	// シャドウ投影正射影の半幅
	float shadowOrthoHalfWidth_ = 80.0f;
	// シャドウ投影正射影の半高さ
	float shadowOrthoHalfHeight_ = 80.0f;
	// 全画面グレースケール有効化フラグ
	bool fullScreenGrayscaleEnabled_ = false;
	// 全画面セピア有効化フラグ
	bool fullScreenSepiaEnabled_ = false;
	// エディタ指定ライトを強制使用するか
	bool useEditorLights_ = false;
	// エディタ指定のディレクショナルライト
	DirectionalLight editorDirectionalLight_ = {
	    {1.0f, 1.0f, 1.0f, 1.0f},
        {0.0f, -1.0f, 0.0f},
        1.0f
    };
	// エディタ指定のポイントライト配列
	std::array<PointLight, kMaxPointLights> editorPointLights_{};
	// エディタ指定ポイントライト有効数
	uint32_t editorPointLightCount_ = 0;
	// エディタ指定のスポットライト配列
	std::array<SpotLight, kMaxSpotLights> editorSpotLights_{};
	// エディタ指定スポットライト有効数
	uint32_t editorSpotLightCount_ = 0;
	// エディタ指定のエリアライト配列
	std::array<AreaLight, kMaxAreaLights> editorAreaLights_{};
	// エディタ指定エリアライト有効数
	uint32_t editorAreaLightCount_ = 0;
	// ルートシグネチャ・DescriptorHeap共通設定
	void DrawSet();

public:
	// シングルトン取得
	static Object3dCommon* GetInstance();
	// 生成
	Object3dCommon();
	// 破棄
	~Object3dCommon();
	// 各種PSO・ライトバッファ・シャドウマップ等の初期化
	void Initialize(DirectXCommon* dxCommon);
	// シングルトン解放
	void Finalize();
	// 既定カメラ設定
	void SetDefaultCamera(Camera* camera) { this->defaultCamera = camera; }
	// 既定カメラ取得
	Camera* GetDefaultCamera() const { return defaultCamera; };
	// 標準描画設定
	void DrawCommon();
	// 指定カメラで標準描画設定
	void DrawCommon(Camera* camera);
	// トゥーン描画設定
	void DrawCommonToon();
	// エミッシブ描画設定
	void DrawCommonEmissive();
	// カリングなし描画設定
	void DrawCommonNoCull();
	// 深度なし描画設定
	void DrawCommonNoDepth();
	// カリングなし＋深度なし描画設定
	void DrawCommonNoCullDepth();
	// ワイヤーフレーム＋深度なし描画設定
	void DrawCommonWireframeNoDepth();
	// ライン＋深度なし描画設定
	void DrawCommonLineNoDepth();
	// エディタグリッド描画設定
	void DrawCommonEditorGrid();
	// スキニング描画設定
	void DrawCommonSkinning();
	// スキニング＋トゥーン描画設定
	void DrawCommonSkinningToon();
	// ミラー描画設定
	void DrawCommonMirror();
	// ポータル描画設定
	void DrawCommonPortal();
	// 指定カメラでポータル描画設定
	void DrawCommonPortal(Camera* camera);
	// シャドウマップ描画設定
	void DrawCommonShadow();
	// シャドウマップパス開始
	void BeginShadowMapPass();
	// シャドウマップパス終了
	void EndShadowMapPass();
	// 汎用アップロードバッファ生成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);
	// DirectX共通クラス取得
	DirectXCommon* GetDxCommon() const { return dxCommon_; };
	// ブレンドモード設定
	void SetBlendMode(BlendMode blendmode);
	// ブレンドモード取得
	BlendMode GetBlendMode() const { return blendMode_; }
	// ディレクショナルライトCB取得
	ID3D12Resource* GetDirectionalLightResource() const { return directionalLightResource_.Get(); }
	// ポイントライト数CB取得
	ID3D12Resource* GetPointLightCountResource() const { return pointLightCountResource_.Get(); }
	// ポイントライトSRVインデックス取得
	uint32_t GetPointLightSrvIndex() const { return pointLightSrvIndex_; }
	// スポットライト数CB取得
	ID3D12Resource* GetSpotLightCountResource() const { return spotLightCountResource_.Get(); }
	// スポットライトSRVインデックス取得
	uint32_t GetSpotLightSrvIndex() const { return spotLightSrvIndex_; }
	// エリアライト数CB取得
	ID3D12Resource* GetAreaLightCountResource() const { return areaLightCountResource_.Get(); }
	// エリアライトSRVインデックス取得
	uint32_t GetAreaLightSrvIndex() const { return areaLightSrvIndex_; }
	// 環境マップSRVインデックス取得
	uint32_t GetEnvironmentMapSrvIndex() const { return environmentMapSrvIndex_; }
	// シャドウマップSRVインデックス取得
	uint32_t GetShadowMapSrvIndex() const { return shadowMapSrvIndex_; }
	// シャドウマップパス状態取得
	bool IsShadowMapPassActive() const { return isShadowMapPassActive_; }

	// ファイルパスから環境マップを設定
	void SetEnvironmentMapTexture(const std::string& filePath);
	// 既存リソースを環境マップとして設定
	void SetEnvironmentMapTextureResource(ID3D12Resource* resource, DXGI_FORMAT format);

	// ディレクショナルライト設定
	void SetDirectionalLight(DirectionalLight& light);
	// ポイントライト配列設定
	void SetPointLights(const PointLight* pointLights, uint32_t count);
	// スポットライト配列設定
	void SetSpotLights(const SpotLight* spotLights, uint32_t count);
	// エリアライト配列設定
	void SetAreaLights(const AreaLight* areaLights, uint32_t count);
	// エディタライト上書き有効化
	void SetEditorLightOverride(bool enabled) { useEditorLights_ = enabled; }
	// エディタライト上書き有効状態取得
	bool IsEditorLightOverrideEnabled() const { return useEditorLights_; }
	// エディタライト一式を設定
	void SetEditorLights(
	    const DirectionalLight& directionalLight, const PointLight* pointLights, uint32_t pointCount, const SpotLight* spotLights, uint32_t spotCount, const AreaLight* areaLights, uint32_t areaCount);

	// ディレクショナルライト視点のViewProjection行列取得
	Matrix4x4 GetDirectionalLightViewProjectionMatrix() const;

	// 全画面グレースケールの有効化切り替え
	void SetFullScreenGrayscaleEnabled(bool enable) { fullScreenGrayscaleEnabled_ = enable; }
	// 全画面グレースケール状態取得
	bool IsFullScreenGrayscaleEnabled() const { return fullScreenGrayscaleEnabled_; }
	// 全画面セピアの有効化切り替え
	void SetFullScreenSepiaEnabled(bool enable) { fullScreenSepiaEnabled_ = enable; }
	// 全画面セピア状態取得
	bool IsFullScreenSepiaEnabled() const { return fullScreenSepiaEnabled_; }

	// ビネット強度設定
	void SetVignetteStrength(float strength);

	// ランダムノイズ有効化切り替え
	void SetRandomNoiseEnabled(bool enabled);
	// ランダムノイズスケール設定
	void SetRandomNoiseScale(float scale);
	// ランダムノイズブレンドモード設定
	void SetRandomNoiseBlendMode(int blendMode);
};