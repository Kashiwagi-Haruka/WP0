#pragma once
#include"Input.h"
#include<memory>
#include"Vector2.h"

enum GameKeyBind
{
	// キーボード
	K_MoveLeft = DIK_A,
	K_MoveRight = DIK_D,
	K_MoveForward = DIK_W,
	K_MoveBackward = DIK_S,
	K_Shot = DIK_SPACE,
	K_Sneak = DIK_LSHIFT,
	K_Interact = DIK_E,

	// コントローラー
	C_MoveLeft = Input::PadButton::kButtonLeft,
	C_MoveRight = Input::PadButton::kButtonRight,
	C_MoveForward = Input::PadButton::kButtonUp,
	C_MoveBackward = Input::PadButton::kButtonDown,
	C_Shot = Input::PadButton::kButtonA,
	C_Sneak = Input::PadButton::kButtonLeftShoulder,
	C_Interact = Input::PadButton::kButtonX,

	//マウス
	M_Shot = Input::MouseButton::kLeft,
};

class PlayerCommand {

private:
	static std::unique_ptr<PlayerCommand> instance_;
public:
	static PlayerCommand* GetInstance();
	PlayerCommand(const PlayerCommand&) = delete;
	PlayerCommand& operator=(const PlayerCommand&) = delete;
	PlayerCommand(PlayerCommand&&) = delete;
	PlayerCommand& operator=(PlayerCommand&&) = delete;
	bool MoveLeft();
	bool MoveRight();
	bool MoveForward();
	bool MoveBackward();
	bool Shot();
	bool Sneak();
	bool Interact();
	Vector2 Rotate(float rotateSpeed);
private:	

	bool Move(const GameKeyBind key, const GameKeyBind controller);
	PlayerCommand() = default;
};