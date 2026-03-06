#define NOMINMAX
#include "Input.h"
#include <cassert>
#include <algorithm>
#include <cmath>


#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "xinput.lib")

using namespace Microsoft::WRL;
std::unique_ptr<Input> Input::instance_ = nullptr;
namespace {
float Clamp01(float value) { return std::clamp(value, 0.0f, 1.0f); }
bool SetJoystickAxisRange(LPDIRECTINPUTDEVICE8 device, DWORD objectOffset, LONG minValue, LONG maxValue) {
	if (!device) {
		return false;
	}

	DIPROPRANGE range{};
	range.diph.dwSize = sizeof(DIPROPRANGE);
	range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	range.diph.dwObj = objectOffset;
	range.diph.dwHow = DIPH_BYOFFSET;
	range.lMin = minValue;
	range.lMax = maxValue;

	return SUCCEEDED(device->SetProperty(DIPROP_RANGE, &range.diph));
}

void ConfigureJoystickAxisRanges(LPDIRECTINPUTDEVICE8 device) {
	constexpr LONG kAxisMin = 0;
	constexpr LONG kAxisMax = 65535;
	SetJoystickAxisRange(device, DIJOFS_X, kAxisMin, kAxisMax);
	SetJoystickAxisRange(device, DIJOFS_Y, kAxisMin, kAxisMax);
	SetJoystickAxisRange(device, DIJOFS_Z, kAxisMin, kAxisMax);
	SetJoystickAxisRange(device, DIJOFS_RX, kAxisMin, kAxisMax);
	SetJoystickAxisRange(device, DIJOFS_RY, kAxisMin, kAxisMax);
	SetJoystickAxisRange(device, DIJOFS_RZ, kAxisMin, kAxisMax);
	SetJoystickAxisRange(device, DIJOFS_SLIDER(0), kAxisMin, kAxisMax);
	SetJoystickAxisRange(device, DIJOFS_SLIDER(1), kAxisMin, kAxisMax);
}
float GetDigitalTrigger(const DIJOYSTATE& state, int buttonIndex) {
	if (buttonIndex < 0 || buttonIndex >= 32) {
		return 0.0f;
	}
	return (state.rgbButtons[buttonIndex] & 0x80) != 0 ? 1.0f : 0.0f;
}
float GetCombinedTriggerNegative(LONG zAxis) { return Clamp01((32767.0f - static_cast<float>(zAxis)) / 32767.0f); }

float GetCombinedTriggerPositive(LONG zAxis) { return Clamp01((static_cast<float>(zAxis) - 32767.0f) / 32767.0f); }

void GetCombinedTriggerValues(const DIJOYSTATE& state, int leftButtonIndex, int rightButtonIndex, float& outLeft, float& outRight) {
	// 共有Z軸は LT=負側, RT=正側 の固定割り当てにする
	outLeft = GetCombinedTriggerNegative(state.lZ);
	outRight = GetCombinedTriggerPositive(state.lZ);

	const float leftDigital = GetDigitalTrigger(state, leftButtonIndex);
	const float rightDigital = GetDigitalTrigger(state, rightButtonIndex);
	if (leftDigital > 0.0f && rightDigital > 0.0f) {
		outLeft = 1.0f;
		outRight = 1.0f;
	} else {
		outLeft = std::max(outLeft, leftDigital);
		outRight = std::max(outRight, rightDigital);
	}
}
float NormalizeXInputTrigger(BYTE triggerValue) { return Clamp01(static_cast<float>(triggerValue) / 255.0f); }
} // namespace

Input* Input::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = std::unique_ptr<Input>(new Input());
	}
	return instance_.get();
}
BOOL CALLBACK Input::EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext) {
	Input* input = reinterpret_cast<Input*>(pContext);
	HRESULT hr = input->directInput->CreateDevice(pdidInstance->guidInstance, &input->gamePadDevice_, NULL);
	if (FAILED(hr)) {
		return DIENUM_CONTINUE;
	}
	return DIENUM_STOP;
}

void Input::Initialize(WinApp* winApp) {

	winApp_ = winApp;

	HRESULT result;

	// DirectInput 作成
	result = DirectInput8Create(winApp_->GetHinstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(result));

	// キーボード
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));
	result = keyboard->SetCooperativeLevel(winApp_->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

	// ゲームパッド列挙 → 最初の1台を使用
	directInput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, this, DIEDFL_ATTACHEDONLY);

	if (gamePadDevice_) {
		result = gamePadDevice_->SetDataFormat(&c_dfDIJoystick);
		assert(SUCCEEDED(result));
		ConfigureJoystickAxisRanges(gamePadDevice_.Get());
		result = gamePadDevice_->SetCooperativeLevel(winApp_->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
		assert(SUCCEEDED(result));
	}

	result = directInput->CreateDevice(GUID_SysMouse, &mouseDevice_, nullptr);
	assert(SUCCEEDED(result));

	result = mouseDevice_->SetDataFormat(&c_dfDIMouse2);
	assert(SUCCEEDED(result));

	result = mouseDevice_->SetCooperativeLevel(winApp_->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
}

void Input::SetIsCursorVisible(bool isVisible) { isCursorVisibleRequested_ = isVisible; }
void Input::Update() {
	if (isCursorVisible_ != isCursorVisibleRequested_) {
		if (isCursorVisibleRequested_) {
			while (ShowCursor(TRUE) < 0) {
			}
		} else {
			while (ShowCursor(FALSE) >= 0) {
			}
		}
		isCursorVisible_ = isCursorVisibleRequested_;
	}
	// 前のフレームのキー入力を保存
	memcpy(preKey, key, sizeof(key));

	// キーボードの状態を取得
	keyboard->Acquire();
	keyboard->GetDeviceState(sizeof(key), key);

	// ゲームパッド
	prePadState_ = padState_;
	preXInputState_ = xInputState_;
	xInputState_ = {};
	isXInputConnected_ = XInputGetState(0, &xInputState_) == ERROR_SUCCESS;

	if (gamePadDevice_) {
		HRESULT hr = gamePadDevice_->Acquire();
		if (SUCCEEDED(hr)) {
			hr = gamePadDevice_->GetDeviceState(sizeof(DIJOYSTATE), &padState_);
			if (FAILED(hr)) {
				if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
					// 再度 Acquire を試みる
					gamePadDevice_->Acquire();
				} else {
					// 本当にダメな場合だけリセット
					gamePadDevice_.Reset();
				}
			}
		} else {
			if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
				// 未取得なら再度 Acquire を試みる
				gamePadDevice_->Acquire();
			} else {
				// それ以外の致命的なエラーならリセット
				gamePadDevice_.Reset();
			}
		}
	}

	if (winApp_->GetIsPad()) {

		// --- もしデバイスが切れていたら再列挙 ---
		if (!gamePadDevice_) {
			directInput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, this, DIEDFL_ATTACHEDONLY);
			if (gamePadDevice_) {
				gamePadDevice_->SetDataFormat(&c_dfDIJoystick);
				ConfigureJoystickAxisRanges(gamePadDevice_.Get());
				gamePadDevice_->SetCooperativeLevel(GetActiveWindow(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
			}
		}
		winApp_->SetIsPad(false);
	}

	prevMouseState_ = mouseState_;

	// ２）DirectInput で相対移動だけ取得（マウスホイールやボタンは要るなら使う）
	if (mouseDevice_) {
		mouseDevice_->Acquire();
		mouseDevice_->GetDeviceState(sizeof(mouseState_), &mouseState_);
	}

	// ３）Windows API でマウスの絶対位置を取得
	POINT pt;
	GetCursorPos(&pt);                       // スクリーン座標
	ScreenToClient(winApp_->GetHwnd(), &pt); // クライアント座標に変換
	mouseX_ = pt.x;                          // ここではクランプせずそのまま代入
	mouseY_ = pt.y;
	if (isCursorStability) {
		if (GetForegroundWindow() == winApp_->GetHwnd()) {
			RECT clientRect{};
			if (GetClientRect(winApp_->GetHwnd(), &clientRect)) {
				POINT center{(clientRect.right - clientRect.left) / 2, (clientRect.bottom - clientRect.top) / 2};
				POINT screenCenter = center;
				ClientToScreen(winApp_->GetHwnd(), &screenCenter);
				SetCursorPos(screenCenter.x, screenCenter.y);
				mouseX_ = center.x;
				mouseY_ = center.y;
			}
		}
	}
}

bool Input::PushKey(BYTE keyNumber) {

	if (key[keyNumber]) {

		return true;
	}

	return false;
}

bool Input::TriggerKey(BYTE keyNumber) {

	if (key[keyNumber] && !preKey[keyNumber]) {

		return true;
	}

	return false;
}

bool Input::ReleaseKey(BYTE keyNumber) {

	if (!key[keyNumber] && preKey[keyNumber]) {

		return true;
	}

	return false;
}

bool Input::PushButton(PadButton button) {
	if (!gamePadDevice_)
		return false;
	int index = static_cast<int>(button);

	if (index <= static_cast<int>(PadButton::kButtonRightThumb)) {
		// 通常ボタン
		return (padState_.rgbButtons[index] & 0x80) != 0;
	} else {
		// POV（十字キー）
		DWORD pov = padState_.rgdwPOV[0];
		if (pov == 0 && button == PadButton::kButtonUp)
			return true;
		if (pov == 9000 && button == PadButton::kButtonRight)
			return true;
		if (pov == 18000 && button == PadButton::kButtonDown)
			return true;
		if (pov == 27000 && button == PadButton::kButtonLeft)
			return true;
	}
	return false;
}

bool Input::TriggerButton(PadButton button) {
	if (!gamePadDevice_)
		return false;
	int index = static_cast<int>(button);

	if (index <= static_cast<int>(PadButton::kButtonRightThumb)) {
		// 通常ボタン
		bool now = (padState_.rgbButtons[index] & 0x80) != 0;
		bool prev = (prePadState_.rgbButtons[index] & 0x80) != 0;
		return (now && !prev);
	} else {
		// POV（十字キー）
		DWORD povNow = padState_.rgdwPOV[0];
		DWORD povPrev = prePadState_.rgdwPOV[0];

		if (button == PadButton::kButtonUp)
			return (povNow == 0 && povPrev != 0);
		if (button == PadButton::kButtonRight)
			return (povNow == 9000 && povPrev != 9000);
		if (button == PadButton::kButtonDown)
			return (povNow == 18000 && povPrev != 18000);
		if (button == PadButton::kButtonLeft)
			return (povNow == 27000 && povPrev != 27000);
	}
	return false;
}
bool Input::ReleaseButton(PadButton button) {
	if (!gamePadDevice_)
		return false;
	int index = static_cast<int>(button);
	if (index <= static_cast<int>(PadButton::kButtonRightThumb)) {
		// 通常ボタン
		bool now = (padState_.rgbButtons[index] & 0x80) != 0;
		bool prev = (prePadState_.rgbButtons[index] & 0x80) != 0;
		return (!now && prev);
	} else {
		// POV（十字キー）
		DWORD povNow = padState_.rgdwPOV[0];
		DWORD povPrev = prePadState_.rgdwPOV[0];
		if (button == PadButton::kButtonUp)
			return (povNow != 0 && povPrev == 0);
		if (button == PadButton::kButtonRight)
			return (povNow != 9000 && povPrev == 9000);
		if (button == PadButton::kButtonDown)
			return (povNow != 18000 && povPrev == 18000);
		if (button == PadButton::kButtonLeft)
			return (povNow != 27000 && povPrev == 27000);
	}
	return false;
}
float Input::GetJoyStickLX() const {
	if (!gamePadDevice_)
		return 0.0f;
	float norm = (padState_.lX - 32767.0f) / 32767.0f;
	if (fabs(norm) < deadZone_)
		norm = 0.0f;
	return norm;
}

float Input::GetJoyStickLY() const {
	if (!gamePadDevice_)
		return 0.0f;
	float norm = (padState_.lY - 32767.0f) / 32767.0f;
	if (fabs(norm) < deadZone_)
		norm = 0.0f;
	// DirectInputは上が小さい値なので上下逆にしたいならマイナスをかける
	return -norm;
}

Vector2 Input::GetJoyStickLXY() const { return Vector2(GetJoyStickLX(), GetJoyStickLY()); }

float Input::GetJoyStickRX() const {
	if (!gamePadDevice_)
		return 0.0f;
	float norm = (padState_.lRx - 32767.0f) / 32767.0f; // 右スティックX
	if (fabs(norm) < deadZone_)
		norm = 0.0f;
	return norm;
}

float Input::GetJoyStickRY() const {
	if (!gamePadDevice_)
		return 0.0f;
	float norm = (padState_.lRy - 32767.0f) / 32767.0f; // 右スティックY
	if (fabs(norm) < deadZone_)
		norm = 0.0f;
	return -norm; // 上をプラスにしたいなら符号を反転
}

Vector2 Input::GetJoyStickRXY() const { return Vector2(GetJoyStickRX(), GetJoyStickRY()); }
float Input::GetLeftTrigger() const {
	if (isXInputConnected_) {
		return NormalizeXInputTrigger(xInputState_.Gamepad.bLeftTrigger);
	}

	if (!gamePadDevice_)
		return 0.0f;

	if (padState_.lRz == 0 && prePadState_.lRz == 0) {
		float left = 0.0f;
		float right = 0.0f;
		GetCombinedTriggerValues(padState_, leftTriggerButtonIndex_, rightTriggerButtonIndex_, left, right);
		return left;
	}

	const float analog = Clamp01(static_cast<float>(padState_.lZ) / 65535.0f);
	return std::max(analog, GetDigitalTrigger(padState_, leftTriggerButtonIndex_));
}

float Input::GetRightTrigger() const {
	if (isXInputConnected_) {
		return NormalizeXInputTrigger(xInputState_.Gamepad.bRightTrigger);
	}

	if (!gamePadDevice_)
		return 0.0f;

	if (padState_.lRz == 0 && prePadState_.lRz == 0) {
		float left = 0.0f;
		float right = 0.0f;
		GetCombinedTriggerValues(padState_, leftTriggerButtonIndex_, rightTriggerButtonIndex_, left, right);
		return right;
	}

	const float analog = Clamp01(static_cast<float>(padState_.lRz) / 65535.0f);
	return std::max(analog, GetDigitalTrigger(padState_, rightTriggerButtonIndex_));
}
bool Input::PushLeftTrigger(float threshold) const { return GetLeftTrigger() >= threshold; }

bool Input::PushRightTrigger(float threshold) const { return GetRightTrigger() >= threshold; }

bool Input::TriggerLeftTrigger(float threshold) const {
	if (isXInputConnected_) {
		const float now = NormalizeXInputTrigger(xInputState_.Gamepad.bLeftTrigger);
		const float prev = NormalizeXInputTrigger(preXInputState_.Gamepad.bLeftTrigger);
		return now >= threshold && prev < threshold;
	}

	float prevNorm = 0.0f;
	if (padState_.lRz == 0 && prePadState_.lRz == 0) {
		float prevLeft = 0.0f;
		float prevRight = 0.0f;
		GetCombinedTriggerValues(prePadState_, leftTriggerButtonIndex_, rightTriggerButtonIndex_, prevLeft, prevRight);
		prevNorm = prevLeft;
	} else {
		prevNorm = std::max(Clamp01(static_cast<float>(prePadState_.lZ) / 65535.0f), GetDigitalTrigger(prePadState_, leftTriggerButtonIndex_));
	}
	return GetLeftTrigger() >= threshold && prevNorm < threshold;
}

bool Input::TriggerRightTrigger(float threshold) const {
	if (isXInputConnected_) {
		const float now = NormalizeXInputTrigger(xInputState_.Gamepad.bRightTrigger);
		const float prev = NormalizeXInputTrigger(preXInputState_.Gamepad.bRightTrigger);
		return now >= threshold && prev < threshold;
	}

	float prevNorm = 0.0f;
	if (padState_.lRz == 0 && prePadState_.lRz == 0) {
		float prevLeft = 0.0f;
		float prevRight = 0.0f;
		GetCombinedTriggerValues(prePadState_, leftTriggerButtonIndex_, rightTriggerButtonIndex_, prevLeft, prevRight);
		prevNorm = prevRight;
	} else {
		prevNorm = std::max(Clamp01(static_cast<float>(prePadState_.lRz) / 65535.0f), GetDigitalTrigger(prePadState_, rightTriggerButtonIndex_));
	}
	return GetRightTrigger() >= threshold && prevNorm < threshold;
}
void Input::SetDeadZone(float deadZone) {
	if (deadZone < 0.0f)
		deadZone = 0.0f;
	if (deadZone > 1.0f)
		deadZone = 1.0f;

	deadZone_ = deadZone;
}

float Input::GetMouseX() const { return static_cast<float>(mouseX_); }

float Input::GetMouseY() const { return static_cast<float>(mouseY_); }

Vector2 Input::GetMouseMove() const {
	// DIMOUSESTATE2 の lX, lY は相対移動量
	return Vector2(static_cast<float>(mouseState_.lX), static_cast<float>(mouseState_.lY));
}
float Input::GetMouseWheelDelta() const { return static_cast<float>(mouseState_.lZ); }

bool Input::PushMouseButton(MouseButton button) const {
	int index = static_cast<int>(button);
	if (index < 0 || index >= static_cast<int>(MouseButton::kMaxButtons)) {
		return false;
	}
	return (mouseState_.rgbButtons[index] & 0x80) != 0;
}

bool Input::TriggerMouseButton(MouseButton button) const {
	int index = static_cast<int>(button);
	if (index < 0 || index >= static_cast<int>(MouseButton::kMaxButtons)) {
		return false;
	}
	bool now = (mouseState_.rgbButtons[index] & 0x80) != 0;
	bool prev = (prevMouseState_.rgbButtons[index] & 0x80) != 0;
	return (now && !prev);
}
bool Input::ReleaseMouseButton(MouseButton button) const {
	int index = static_cast<int>(button);
	if (index < 0 || index >= static_cast<int>(MouseButton::kMaxButtons)) {
		return false;
	}
	bool now = (mouseState_.rgbButtons[index] & 0x80) != 0;
	bool prev = (prevMouseState_.rgbButtons[index] & 0x80) != 0;
	return (!now && prev);
}
