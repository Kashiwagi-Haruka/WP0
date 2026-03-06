#pragma once
#include <DirectXTex.h>
#include <Windows.h>
#include <cstdint>
#include <d3d12.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <wrl.h>

class DirectXCommon;
class SrvManager;

// テクスチャの読み込み・SRV管理を行うシングルトン
class TextureManager {

private:
	// テクスチャ 1 件分の管理データ
	struct TextureData {
		std::string filePath;                            // キー兼パス
		DirectX::TexMetadata metadata;                   // テクスチャメタ情報
		Microsoft::WRL::ComPtr<ID3D12Resource> resource; // GPU リソース
		uint32_t srvIndex;                               // SRV 管理番号
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;        // CPU 側 SRV ハンドル
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;        // GPU 側 SRV ハンドル
	};

	std::unordered_map<std::string, TextureData> textureDatas; // 読み込み済みテクスチャ一覧

	static std::unique_ptr<TextureManager> instance;
	static uint32_t kSRVIndexTop;

	TextureManager(TextureManager&) = delete;
	TextureManager& operator=(TextureManager&) = delete;

	// metadata から D3D12 テクスチャリソースを生成する
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(DirectX::TexMetadata& metadata);
	// ScratchImage の各 mip データをリソースへ転送する
	void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages);

	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;

public:
	TextureManager() = default;
	~TextureManager() = default;
	// シングルトンインスタンスの取得
	static TextureManager* GetInstance();

	// 初期化
	void Initialize(DirectXCommon* dxCommon);
	// ファイルからテクスチャを読み込む（未読み込み時のみ）
	void LoadTextureName(const std::string& filePath);
	// メモリ上の画像データからテクスチャを読み込む
	void LoadTextureFromMemory(const std::string& key, const uint8_t* data, size_t size);
	// RGBA8 生データからテクスチャを作成する
	void LoadTextureFromRGBA8(const std::string& key, uint32_t width, uint32_t height, const uint8_t* data);
	// ファイルパスから SRV インデックスを取得する
	uint32_t GetTextureIndexByfilePath(const std::string& filePath);
	// ファイルパスから GPU SRV ハンドルを取得する
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(const std::string& filePath);
	// ファイルパスから SRV インデックスを取得する
	uint32_t GetsrvIndex(const std::string& filePath);
	// ファイルパスからメタデータを取得する
	const DirectX::TexMetadata& GetMetaData(const std::string& filepath);
	// SRV インデックスからメタデータを取得する
	DirectX::TexMetadata& GetMetaData(uint32_t srvIndex);
	// SRV インデックスから GPU SRV ハンドルを取得する
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t srvIndex);

	// 終了
	void Finalize();
};