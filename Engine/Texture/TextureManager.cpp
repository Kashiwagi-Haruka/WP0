#include "TextureManager.h"
#include "DirectXCommon.h"
#include "SrvManager/SrvManager.h"
#include "StringUtility.h"
std::unique_ptr<TextureManager> TextureManager::instance = nullptr;
uint32_t TextureManager::kSRVIndexTop = 1;

// TextureManager のシングルトンインスタンスを返す
TextureManager* TextureManager::GetInstance() {

	if (instance == nullptr) {
		instance = std::make_unique<TextureManager>();
	}
	return instance.get();
}

// 利用する DirectX 共通機能と SRV マネージャーを設定する
void TextureManager::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;
	srvManager_ = SrvManager::GetInstance();
	textureDatas.reserve(srvManager_->kMaxSRVCount_);
}

// インスタンスを破棄して終了処理を行う
void TextureManager::Finalize() { instance.reset(); }

// ファイルパス指定でテクスチャを読み込み、SRV を作成する
void TextureManager::LoadTextureName(const std::string& filePath) {

	if (textureDatas.contains(filePath)) {
		return;
	}
	assert(srvManager_->CanAllocate());

	// テクスチャファイルを読んでプログラムで使えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString_(filePath);
	HRESULT hr_ = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr_));

	// ミップマップの作成
	DirectX::ScratchImage mipImages{};
	hr_ = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr_));

	TextureData& textureData = textureDatas[filePath];
	textureData.filePath = filePath;
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = CreateTextureResource(textureData.metadata);

	// ★ ここを UploadTextureData に統一
	UploadTextureData(textureData.resource.Get(), mipImages);

	// SRVハンドル設定
	textureData.srvIndex = srvManager_->Allocate();
	textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureData.metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(textureData.metadata.mipLevels);

	dxCommon_->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);

	// デバッグログ出力
	std::string log = "Texture Loaded: " + filePath + " | SRV Index: " + std::to_string(textureData.srvIndex) + " | GPU Handle: " + std::to_string(textureData.srvHandleGPU.ptr) + "\n";
	OutputDebugStringA(log.c_str());
}
// メモリ上の画像データを読み込み、SRV を作成する
void TextureManager::LoadTextureFromMemory(const std::string& key, const uint8_t* data, size_t size) {
	if (textureDatas.contains(key)) {
		return;
	}
	assert(srvManager_->CanAllocate());

	DirectX::ScratchImage image{};
	HRESULT hr_ = DirectX::LoadFromWICMemory(data, size, DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr_));

	DirectX::ScratchImage mipImages{};
	hr_ = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr_));

	TextureData& textureData = textureDatas[key];
	textureData.filePath = key;
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = CreateTextureResource(textureData.metadata);

	UploadTextureData(textureData.resource.Get(), mipImages);

	textureData.srvIndex = srvManager_->Allocate();
	textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureData.metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(textureData.metadata.mipLevels);

	dxCommon_->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);

	std::string log = "Embedded Texture Loaded: " + key + " | SRV Index: " + std::to_string(textureData.srvIndex) + " | GPU Handle: " + std::to_string(textureData.srvHandleGPU.ptr) + "\n";
	OutputDebugStringA(log.c_str());
}

// RGBA8 の生配列からテクスチャを生成して SRV を作成する
void TextureManager::LoadTextureFromRGBA8(const std::string& key, uint32_t width, uint32_t height, const uint8_t* data) {
	if (textureDatas.contains(key)) {
		return;
	}
	assert(srvManager_->CanAllocate());

	DirectX::ScratchImage image{};
	HRESULT hr_ = image.Initialize2D(DXGI_FORMAT_B8G8R8A8_UNORM, width, height, 1, 1);
	assert(SUCCEEDED(hr_));

	const DirectX::Image* img = image.GetImage(0, 0, 0);
	assert(img != nullptr);
	std::memcpy(img->pixels, data, static_cast<size_t>(width) * height * 4);

	DirectX::ScratchImage mipImages{};
	hr_ = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr_));

	TextureData& textureData = textureDatas[key];
	textureData.filePath = key;
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = CreateTextureResource(textureData.metadata);

	UploadTextureData(textureData.resource.Get(), mipImages);

	textureData.srvIndex = srvManager_->Allocate();
	textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureData.metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(textureData.metadata.mipLevels);

	dxCommon_->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);

	std::string log = "Embedded Texture Loaded (RGBA8): " + key + " | SRV Index: " + std::to_string(textureData.srvIndex) + " | GPU Handle: " + std::to_string(textureData.srvHandleGPU.ptr) + "\n";
	OutputDebugStringA(log.c_str());
}
// メタデータに基づいてテクスチャリソースを作成する
Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::CreateTextureResource(DirectX::TexMetadata& metadata) {

	// metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width);                             // Textureの幅
	resourceDesc.Height = UINT(metadata.height);                           // Textureの高さ
	resourceDesc.MipLevels = UINT16(metadata.mipLevels);                   // mipmapの数
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);            // 奥行き or 配列Textureの配列数
	resourceDesc.Format = metadata.format;                                 // TextureのFormat
	resourceDesc.SampleDesc.Count = 1;                                     // サンプリングカウント。1固定。
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension); // Textureの次元数。普段使っているのは2次元

	// 利用するHeapの設定。非常に特殊な運用。02_04exで一般的なケース版がある
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;                        // 細かい設定を行う
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK; // WriteBackポリシーでCPUアクセス可能
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;          // プロセッサの近くに配置

	// Resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource_ = nullptr;
	HRESULT hr_ = dxCommon_->GetDevice()->CreateCommittedResource(
	    &heapProperties,                   // Heapの設定
	    D3D12_HEAP_FLAG_NONE,              // Heapの特殊な設定。特になし。
	    &resourceDesc,                     // Resourceの設定
	    D3D12_RESOURCE_STATE_GENERIC_READ, // 初回のResourceState。Textureは基本読むだけ
	    nullptr,                           // Clear最適値。使わないのでnullptr
	    IID_PPV_ARGS(&textureResource_));  // 作成するResourceポインタへのポインタ
	assert(SUCCEEDED(hr_));
	return textureResource_;
}

// uint32_t TextureManager::GetTextureIndexByfilePath(const std::string& filePath){
//
//	LoadTextureName(filePath);
//
//
//	auto it = std::find_if(textureDatas.begin(), textureDatas.end(), [&](const auto& pair) { return pair.second.filePath == filePath; });
//
//
//	if (it != textureDatas.end()) {
//
//		uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas.begin(), it));
//		return textureIndex;
//	}
//	assert(0);
//	return 0;
// }
// ファイルパスから SRV インデックスを取得する
uint32_t TextureManager::GetTextureIndexByfilePath(const std::string& filePath) {
	LoadTextureName(filePath);

	auto it = textureDatas.find(filePath);
	assert(it != textureDatas.end());

	return it->second.srvIndex;
}

// ファイルパスから SRV インデックスを取得する
uint32_t TextureManager::GetsrvIndex(const std::string& filePath) {
	// 未読み込みなら読み込む
	LoadTextureName(filePath);

	auto it = textureDatas.find(filePath);
	assert(it != textureDatas.end());

	return it->second.srvIndex;
}

// ファイルパスから GPU SRV ハンドルを取得する
D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filePath) {
	LoadTextureName(filePath);

	auto it = textureDatas.find(filePath);
	assert(it != textureDatas.end());

	return it->second.srvHandleGPU;
}

// ファイルパスからテクスチャのメタデータを取得する
const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath) {
	LoadTextureName(filePath);

	auto it = textureDatas.find(filePath);
	assert(it != textureDatas.end());

	return it->second.metadata;
}

// 全 mip レベルの画像データを GPU リソースへ書き込む
void TextureManager::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages) {
	// Meta情報を取得
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	// 全MipMapについて
	for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel) {
		// MipMapLevelを指定して各Imageを取得
		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);

		// Textureに転送
		HRESULT hr_ = texture->WriteToSubresource(
		    UINT(mipLevel),      // 全領域へコピー
		    nullptr,             // 元データアドレス
		    img->pixels,         // 1ラインサイズ
		    UINT(img->rowPitch), // 1枚サイズ
		    UINT(img->slicePitch));
		assert(SUCCEEDED(hr_));
	}
}
// SRV インデックスからテクスチャメタデータを取得する
DirectX::TexMetadata& TextureManager::GetMetaData(uint32_t srvIndex) {
	for (auto& [key, data] : textureDatas) {
		if (data.srvIndex == srvIndex) {
			return data.metadata;
		}
	}
	assert(false && "Invalid srvIndex");
	static DirectX::TexMetadata dummy{};
	return dummy;
}
// SRV インデックスから GPU SRV ハンドルを取得する
D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t srvIndex) {
	for (auto& [key, data] : textureDatas) {
		if (data.srvIndex == srvIndex) {
			return data.srvHandleGPU;
		}
	}
	// RenderTexture など TextureManager で管理していない SRV は
	// SRV マネージャーから直接ハンドルを取得する。
	assert(srvManager_ != nullptr);
	return srvManager_->GetGPUDescriptorHandle(srvIndex);
}