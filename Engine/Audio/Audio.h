#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <wrl.h>
#include <xaudio2.h>
struct ChunkHeader {
	// チャンク識別子("RIFF" や "fmt " など)
	char id[4];
	// チャンク本体サイズ
	int32_t size;
};

struct RiffHeader {
	// RIFF チャンク情報
	ChunkHeader chunk;
	// RIFF の種類("WAVE")
	char type[4];
};
struct FormatChunk {
	// fmt チャンク情報
	ChunkHeader chunk;
	// 波形フォーマット情報
	WAVEFORMATEX fmt;
};
struct SoundData {
	// 再生時に使う波形フォーマット
	WAVEFORMATEX wfex;
	// PCM データ本体
	std::vector<BYTE> buffer;
	// このサウンドの既定音量(0.0f～1.0f)
	float volume = 1.0f;
	// エディター表示用の名前
	std::string debugName;
};

class Audio {

public:
	struct EditorSoundEntry {
		SoundData* soundData = nullptr;
		std::string name;
		bool isPlaying = false;
		bool isLoop = false;
	};

private:
	static std::unique_ptr<Audio> instance;

	// XAudio2 本体
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;

	// 出力先となるマスターボイス
	IXAudio2MasteringVoice* masterVoice_;

	// API 呼び出し結果保持用
	HRESULT result_;
	struct ActiveVoice {
		// 実際に再生中のソースボイス
		IXAudio2SourceVoice* voice;
		// どの SoundData を再生しているか識別するための先頭アドレス
		const BYTE* audioData;
		// ループ再生中かどうか
		bool isLoop;
	};

	struct EditorTrackedSound {
		SoundData* soundData = nullptr;
	};

	// 現在再生中のボイス一覧
	std::vector<ActiveVoice> activeVoices_;
	// エディター表示対象のサウンド一覧
	std::vector<EditorTrackedSound> editorTrackedSounds_;

	// 指定サウンドに紐づく再生中ボイスのみ停止
	void StopVoicesForSound(const SoundData& soundData);
	// 全ボイスを停止
	void StopAllVoices();
	// エディター表示対象へ登録
	void TrackSoundForEditor(SoundData* soundData);

public:
	// 非ループ音声の終了監視と後始末
	void Update();
	// シングルトン取得
	static Audio* GetInstance();
	// Audio システム破棄
	void Finalize();
	// XAudio2 / MediaFoundation 初期化
	void InitializeIXAudio();
	// 音声ファイル読み込み
	SoundData SoundLoadFile(const char* filename);
	// 音声データ解放
	void SoundUnload(SoundData* soundData);
	// 音声再生
	void SoundPlayWave(const SoundData& sounddata, bool isLoop = false);
	// 音量設定
	void SetSoundVolume(SoundData* soundData, float volume);
	// エディター表示用のサウンド情報を取得
	std::vector<EditorSoundEntry> GetEditorSoundEntries() const;
	// XAudio2 へのアクセス用 getter
	Microsoft::WRL::ComPtr<IXAudio2> GetIXAudio2() { return xAudio2_; };
};