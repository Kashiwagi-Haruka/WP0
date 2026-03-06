#include"KeyBindConfig.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"


std::unique_ptr<PlayerCommand> PlayerCommand::instance_ = nullptr;

PlayerCommand* PlayerCommand::GetInstance()
{
    if (instance_ == nullptr) {
        instance_ = std::unique_ptr<PlayerCommand>(new PlayerCommand());
    }
    return instance_.get();
}


bool PlayerCommand::MoveLeft()
{
    return Move(K_MoveLeft, C_MoveLeft);
}

bool PlayerCommand::MoveRight()
{
    return Move(K_MoveRight, C_MoveRight);
}

bool PlayerCommand::MoveForward()
{
    return  Move(K_MoveForward, C_MoveForward);
}

bool PlayerCommand::MoveBackward()
{
    return  Move(K_MoveBackward, C_MoveBackward);
}

bool PlayerCommand::Shot()
{
    auto* input = Input::GetInstance();
    return input->TriggerKey(K_Shot) || input->TriggerButton(Input::PadButton(C_Shot))||input->TriggerMouseButton(Input::MouseButton(M_Shot));
}

bool PlayerCommand::Sneak()
{
    auto* input = Input::GetInstance();
    return input->PushKey(K_Sneak) || input->PushButton(Input::PadButton(C_Sneak));
}

bool PlayerCommand::Interact()
{
    auto* input = Input::GetInstance();
    return input->PushKey(K_Interact) || input->PushButton(Input::PadButton(C_Interact));
}

Vector2 PlayerCommand::Rotate(float rotateSpeed)
{
    auto* input = Input::GetInstance();

    Vector2 inputMovePos = input->GetJoyStickRXY();
    float dPitch = 0.0f;
    float dYaw = 0.0f;

    if (fabs(inputMovePos.x) > 0.0f || fabs(inputMovePos.y) > 0.0f) {
        //スティック処理が優先される
        dYaw = inputMovePos.x * YoshidaMath::kDeltaTime * rotateSpeed * 10.0f;
        dPitch = -inputMovePos.y * YoshidaMath::kDeltaTime * rotateSpeed * 10.0f;
    } else {
        //マウス
        inputMovePos = input->GetMouseMove();
        dYaw += inputMovePos.x * YoshidaMath::kDeltaTime * rotateSpeed;
        dPitch += inputMovePos.y * YoshidaMath::kDeltaTime * rotateSpeed;
    }

    return {dPitch,dYaw };
}

bool PlayerCommand::Move(const GameKeyBind key, const GameKeyBind controller)
{
    auto* input = Input::GetInstance();
    return input->PushKey(key) || input->PushButton(Input::PadButton(controller));
}
