#include "Audio.h"
#include "fstream"
#include <algorithm>
#include <filesystem>
#include <assert.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <string>
#pragma comment(lib, "xAudio2.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")

// Audio シングルトンインスタンス
std::unique_ptr<Audio> Audio::instance = nullptr;

// シングルトンインスタンスを返す
Audio* Audio::GetInstance() {
	if (instance == nullptr) {
		instance = std::make_unique<Audio>();
	}
	return instance.get();
}
// 再生中ボイスとオーディオAPIを終了する
void Audio::Finalize() {
	StopAllVoices();
	if (masterVoice_) {
		masterVoice_->DestroyVoice();
		masterVoice_ = nullptr;
	}
	xAudio2_.Reset();

	result_ = MFShutdown();
	assert(SUCCEEDED(result_));

	instance = nullptr;
}

// Media Foundation / XAudio2 を初期化する
void Audio::InitializeIXAudio() {
	result_ = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	assert(SUCCEEDED(result_));
	result_ = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(result_)); // ここ追加
	result_ = xAudio2_->CreateMasteringVoice(&masterVoice_);
	assert(SUCCEEDED(result_)); // ここも追加
}

// ワンショット再生が終わったボイスを回収する
void Audio::Update() {
	if (activeVoices_.empty()) {
		return;
	}

	for (auto& active : activeVoices_) {
		if (!active.voice) {
			continue;
		}
		if (active.isLoop) {
			continue;
		}

		XAUDIO2_VOICE_STATE state{};
		active.voice->GetState(&state);
		if (state.BuffersQueued == 0) {
			active.voice->DestroyVoice();
			active.voice = nullptr;
		}
	}

	activeVoices_.erase(std::remove_if(activeVoices_.begin(), activeVoices_.end(), [](const ActiveVoice& active) { return active.voice == nullptr; }), activeVoices_.end());
}
void Audio::TrackSoundForEditor(SoundData* soundData) {
	if (!soundData) {
		return;
	}
	for (const auto& tracked : editorTrackedSounds_) {
		if (tracked.soundData == soundData) {
			return;
		}
	}
	editorTrackedSounds_.push_back({soundData});
}
// 音声ファイルを PCM として読み込んで SoundData を生成する
SoundData Audio::SoundLoadFile(const char* filename) {
	// フルパス → UTF-16 変換
	std::wstring filePathW;
	{
		int size = MultiByteToWideChar(CP_UTF8, 0, filename, -1, nullptr, 0);
		filePathW.resize(size);
		MultiByteToWideChar(CP_UTF8, 0, filename, -1, &filePathW[0], size);
	}

	HRESULT hr;
	Microsoft::WRL::ComPtr<IMFSourceReader> pReader;

	// SourceReader 作成
	hr = MFCreateSourceReaderFromURL(filePathW.c_str(), nullptr, &pReader);
	assert(SUCCEEDED(hr));

	// PCM にデコードする設定
	Microsoft::WRL::ComPtr<IMFMediaType> pPCMType;
	hr = MFCreateMediaType(&pPCMType);
	assert(SUCCEEDED(hr));

	hr = pPCMType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	assert(SUCCEEDED(hr));

	hr = pPCMType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	assert(SUCCEEDED(hr));

	hr = pReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pPCMType.Get());
	assert(SUCCEEDED(hr));

	// 実際に適用されたメディアタイプ
	Microsoft::WRL::ComPtr<IMFMediaType> pOutType;
	hr = pReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pOutType);
	assert(SUCCEEDED(hr));

	// WAVEFORMATEX を取り出す
	WAVEFORMATEX* waveFormat = nullptr;
	hr = MFCreateWaveFormatExFromMFMediaType(pOutType.Get(), &waveFormat, nullptr);
	assert(SUCCEEDED(hr));
	// SoundData へ格納
	SoundData soundData{};
	soundData.wfex = *waveFormat; // WAVEFORMATEX コピー
	CoTaskMemFree(waveFormat);
	// 読み込んだ PCM データを格納するバッファ
	std::vector<BYTE> buffer;

	// 1フレームずつ読み込む
	while (true) {
		DWORD streamIndex = 0, flags = 0;
		LONGLONG llTimeStamp = 0;
		Microsoft::WRL::ComPtr<IMFSample> pSample;

		hr = pReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &streamIndex, &flags, &llTimeStamp, &pSample);

		assert(SUCCEEDED(hr));

		if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
			break;

		if (!pSample) {
			continue;
		} else {
			Microsoft::WRL::ComPtr<IMFMediaBuffer> pBuffer;
			hr = pSample->ConvertToContiguousBuffer(&pBuffer);
			assert(SUCCEEDED(hr));

			BYTE* pData = nullptr;
			DWORD maxLength = 0, curLength = 0;

			hr = pBuffer->Lock(&pData, &maxLength, &curLength);
			assert(SUCCEEDED(hr));

			soundData.buffer.insert(soundData.buffer.end(), pData, pData + curLength);

			pBuffer->Unlock();
		}
	}

	soundData.debugName = std::filesystem::path(filename).filename().string();

	return soundData;
}

// 指定サウンドに紐づく再生を止めてメモリを解放する
void Audio::SoundUnload(SoundData* soundData) {
	if (!soundData) {
		return;
	}
	StopVoicesForSound(*soundData);
	soundData->buffer.clear();
	soundData->wfex = {};
	for (auto& tracked : editorTrackedSounds_) {
		if (tracked.soundData == soundData) {
			tracked.soundData = nullptr;
		}
	}
}

// サウンドを再生する(必要ならループ再生)
void Audio::SoundPlayWave(const SoundData& soundData, bool isLoop) {
	TrackSoundForEditor(const_cast<SoundData*>(&soundData));
	if (!xAudio2_) {
		OutputDebugStringA("SoundPlayWave: xAudio2 is null!\n");
		return;
	}
	if (soundData.buffer.empty()) {
		OutputDebugStringA("SoundPlayWave: sound buffer is empty!\n");
		return;
	}
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result_ = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result_));
	pSourceVoice->SetVolume(soundData.volume);
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.buffer.data();
	buf.AudioBytes = static_cast<UINT32>(soundData.buffer.size());

	if (isLoop) {
		buf.LoopBegin = 0;                     // バッファ先頭から
		buf.LoopLength = 0;                    // 0 = 全体をループ
		buf.LoopCount = XAUDIO2_LOOP_INFINITE; // 無限ループ
		buf.Flags = 0;                         // ループ時は END_OF_STREAM にしない
	} else {
		buf.Flags = XAUDIO2_END_OF_STREAM; // 1回再生のみ
	}

	result_ = pSourceVoice->SubmitSourceBuffer(&buf);
	assert(SUCCEEDED(result_));

	result_ = pSourceVoice->Start();
	assert(SUCCEEDED(result_));
	activeVoices_.push_back({pSourceVoice, soundData.buffer.data(), isLoop});
}

// 特定サウンドを再生しているボイスだけを停止する
void Audio::StopVoicesForSound(const SoundData& soundData) {
	const BYTE* targetData = soundData.buffer.data();
	if (!targetData) {
		return;
	}

	for (auto& active : activeVoices_) {
		if (active.voice && active.audioData == targetData) {
			active.voice->Stop();
			active.voice->DestroyVoice();
			active.voice = nullptr;
		}
	}

	activeVoices_.erase(std::remove_if(activeVoices_.begin(), activeVoices_.end(), [](const ActiveVoice& active) { return active.voice == nullptr; }), activeVoices_.end());
}
// サウンド音量を更新し、再生中ボイスにも反映する
void Audio::SetSoundVolume(SoundData* soundData, float volume) {
	TrackSoundForEditor(soundData);
	if (!soundData) {
		return;
	}
	soundData->volume = std::clamp(volume, 0.0f, 1.0f);
	const BYTE* targetData = soundData->buffer.data();
	if (!targetData) {
		return;
	}

	for (auto& active : activeVoices_) {
		if (active.voice && active.audioData == targetData) {
			active.voice->SetVolume(soundData->volume);
		}
	}
}

std::vector<Audio::EditorSoundEntry> Audio::GetEditorSoundEntries() const {
	std::vector<EditorSoundEntry> entries;
	entries.reserve(editorTrackedSounds_.size());
	for (const auto& tracked : editorTrackedSounds_) {
		if (!tracked.soundData || tracked.soundData->buffer.empty()) {
			continue;
		}
		EditorSoundEntry entry{};
		entry.soundData = tracked.soundData;
		entry.name = tracked.soundData->debugName.empty() ? "(unnamed sound)" : tracked.soundData->debugName;
		for (const auto& active : activeVoices_) {
			if (!active.voice || active.audioData != tracked.soundData->buffer.data()) {
				continue;
			}
			entry.isPlaying = true;
			entry.isLoop = entry.isLoop || active.isLoop;
		}
		entries.push_back(std::move(entry));
	}
	return entries;
}

// すべての再生中ボイスを停止する
void Audio::StopAllVoices() {
	for (auto& active : activeVoices_) {
		if (active.voice) {
			active.voice->Stop();
			active.voice->DestroyVoice();
			active.voice = nullptr;
		}
	}
	activeVoices_.clear();
}