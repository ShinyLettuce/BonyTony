#include "InputMapper.h"

#include <intsafe.h>

#include "tge/engine.h"
#include "tge/drawers/SpriteDrawer.h"
#include "tge/input/InputManager.h"
#include "tge/input/XInput.h"
#include "tge/math/CommonMath.h"
#include "tge/math/FMath.h"

InputMapper::InputMapper(const Tga::InputManager* aInputManager, Tga::XInput* aXInput)
{
    myInputManager = aInputManager;
    myXInput = aXInput;
}

void InputMapper::BindKeyAction(const int aKeyCode, const GameAction aAction)
{
    myKeyActionInputs[static_cast<size_t>(aAction)].push_back({.keyCode = aKeyCode, .states = {}});
}

void InputMapper::BindMouseAction(const MouseInput aMouseInput, const GameAction aAction)
{
    myMouseActionInputs[static_cast<size_t>(aAction)].push_back({.inputType = aMouseInput, .activeValue = 0, .states = {}});
}

void InputMapper::BindGamePadAnalogAction(const GamePadAnalogInput aGamePadAnalogInput, const GameAction aAction)
{
    myGamePadAnalogInputs[static_cast<size_t>(aAction)].push_back({.inputType = aGamePadAnalogInput, .activeValue = 0, .states = {}});
}

void InputMapper::BindGamePadButtonAction(const int aButtonCode, const GameAction aAction)
{
    myGamepadActionInputs[static_cast<size_t>(aAction)].push_back({.keyCode = aButtonCode, .states = {}});
}

bool InputMapper::IsActionActive(const GameAction aAction) const
{
    const auto& keyInputs = myKeyActionInputs[static_cast<size_t>(aAction)];
    for (const auto& input : keyInputs)
    {
        if (input.states[ACTIVE_STATE])
        {
            return true;
        }
    }
    const auto& mouseInputs = myMouseActionInputs[static_cast<size_t>(aAction)];
    for (const auto& input : mouseInputs)
    {
        if (input.activeValue > DEADZONE)
        {
            return true;
        }
    }
    const auto& gamepadAnalogInputs = myGamePadAnalogInputs[static_cast<size_t>(aAction)];
    for (const auto& input : gamepadAnalogInputs)
    {
        if (input.activeValue > DEADZONE)
        {
            return true;
        }
    }
    const auto& buttonInputs = myGamepadActionInputs[static_cast<size_t>(aAction)];
    for (const auto& input : buttonInputs)
    {
        if (input.states[ACTIVE_STATE])
        {
            return true;
        }
    }
    return false;
}

bool InputMapper::IsActionJustActivated(const GameAction aAction) const
{
    const auto& keyInputs = myKeyActionInputs[static_cast<size_t>(aAction)];
    for (const auto& input : keyInputs)
    {
        if (input.states[JUST_ACTIVATED_STATE])
        {
            return true;
        }
    }
    
    const auto& buttonInputs = myGamepadActionInputs[static_cast<size_t>(aAction)];
    for (const auto& input : buttonInputs)
    {
        if (input.states[JUST_ACTIVATED_STATE])
        {
            return true;
        }
    }
    
    const auto& gamepadAnalogInputs = myGamePadAnalogInputs[static_cast<size_t>(aAction)];
    for (const auto& input : gamepadAnalogInputs)
    {
        if (input.states[JUST_ACTIVATED_STATE])
        {
            return true;
        }
    }
    
    const auto& mouseInputs = myMouseActionInputs[static_cast<size_t>(aAction)];
    for (const auto& input : mouseInputs)
    {
        if (input.states[JUST_ACTIVATED_STATE])
        {
            return true;
        }
    }
    return false;
}

bool InputMapper::IsActionJustDeactivated(const GameAction aAction) const
{
    const auto& keyInputs = myKeyActionInputs[static_cast<size_t>(aAction)];
    for (const auto& input : keyInputs)
    {
        if (input.states[JUST_DEACTIVATED_STATE])
        {
            return true;
        }
    }
    
    const auto& buttonInputs = myGamepadActionInputs[static_cast<size_t>(aAction)];
    for (const auto& input : buttonInputs)
    {
        if (input.states[JUST_DEACTIVATED_STATE])
        {
            return true;
        }
    }
    
    const auto& gamepadAnalogInputs = myGamePadAnalogInputs[static_cast<size_t>(aAction)];
    for (const auto& input : gamepadAnalogInputs)
    {
        if (input.states[JUST_DEACTIVATED_STATE])
        {
            return true;
        }
    }
    
    const auto& mouseInputs = myMouseActionInputs[static_cast<size_t>(aAction)];
    for (const auto& input : mouseInputs)
    {
        if (input.states[JUST_DEACTIVATED_STATE])
        {
            return true;
        }
    }
    return false;
}

float InputMapper::GetActionActiveValue(const GameAction aAction) const
{
    float maxValue = 0.0f;
    const auto& mouseInputs = myMouseActionInputs[static_cast<size_t>(aAction)];
    for (const auto& input : mouseInputs)
    {
        if (input.activeValue > maxValue && input.activeValue > DEADZONE)
        {
            maxValue = input.activeValue;
        }
    }
    const auto& gamepadAnalogInputs = myGamePadAnalogInputs[static_cast<size_t>(aAction)];
    for (const auto& input : gamepadAnalogInputs)
    {
        if (input.activeValue > maxValue && input.activeValue > DEADZONE)
        {
            maxValue = input.activeValue;
        }
    }
    const auto& gamepadButtonInputs = myGamepadActionInputs[static_cast<size_t>(aAction)];
    for (const auto& input : gamepadButtonInputs)
    {
        if (input.states[ACTIVE_STATE])
        {
            maxValue = 1;
        }
    }
    const auto& keyInputs = myKeyActionInputs[static_cast<size_t>(aAction)];
    for (const auto& input : keyInputs)
    {
        if (input.states[ACTIVE_STATE])
        {
            maxValue = 1;
        }
    }
    return maxValue;
}

void InputMapper::Update(const float aTimeDelta)
{
    bool mouseMovedThisFrame = false;
    bool leftStickMovedThisFrame = false;
    
    const Tga::Vector2f mousePosition = myInputManager->GetMousePosition(); 
    const Tga::Vector2f mouseDelta = myMousePosition - mousePosition;
    myMousePosition = mousePosition;
    
    if (FMath::Abs(mouseDelta.x) > 0.f || FMath::Abs(mouseDelta.y) > 0.f)
    {
        mouseMovedThisFrame = true;
    }
    
    for (auto& inputActions : myKeyActionInputs)
    {
        for (auto& inputAction : inputActions)
        {
            inputAction.states[ACTIVE_STATE] = myInputManager->IsKeyHeld(inputAction.keyCode);
            inputAction.states[JUST_ACTIVATED_STATE] = myInputManager->IsKeyPressed(inputAction.keyCode);
            inputAction.states[JUST_DEACTIVATED_STATE] = myInputManager->IsKeyReleased(inputAction.keyCode);
        }
    }

    for (auto& inputActions : myMouseActionInputs)
    {
        for (auto& inputAction : inputActions)
        {
            const float prevValue = inputAction.activeValue;
            switch (inputAction.inputType)
            {
            case MouseInput::Up:
                {
                    inputAction.activeValue = FMath::Abs(Tga::Clamp(myInputManager->GetMouseDelta().y, 0, 1));
                    break;
                }
            case MouseInput::Down:
                {
                    inputAction.activeValue = FMath::Abs(Tga::Clamp(myInputManager->GetMouseDelta().y, -1, 0));
                    break;
                }
            case MouseInput::Left:
                {
                    inputAction.activeValue = FMath::Abs(Tga::Clamp(myInputManager->GetMouseDelta().x, -1, 0));
                    break;
                }
            case MouseInput::Right:
                {
                    inputAction.activeValue = FMath::Abs(Tga::Clamp(myInputManager->GetMouseDelta().x, 0, 1));
                    break;
                }
                default:
                {
                    inputAction.activeValue = 0.0f;
                    break;
                }
            }
            inputAction.states[JUST_ACTIVATED_STATE] = !(prevValue > DEADZONE) && (inputAction.activeValue > DEADZONE);
            inputAction.states[JUST_DEACTIVATED_STATE] = (prevValue > DEADZONE) && !(inputAction.activeValue > DEADZONE);
        }
    }

    if (myXInput->CheckConnection())
    {
        auto state = myXInput->GetState();
        
        if (state->sThumbLX > STICK_USE_DEADZONE || state->sThumbLY > STICK_USE_DEADZONE)
        {
            leftStickMovedThisFrame = true;
        }
        
        float leftRumble = 0.0f;
        float rightRumble = 0.0f;
        for (size_t i = 0; i < MAX_RUMBLE_INSTANCES; ++i)
        {
            if (myRumbleInstances[i].time <= 0.0f)
            {
                continue;
            }
            myRumbleInstances[i].time -= aTimeDelta;
            leftRumble += myRumbleInstances[i].left;
            rightRumble += myRumbleInstances[i].right;
        }
        SetRumble(leftRumble, rightRumble);
        
        for (auto& inputActions : myGamepadActionInputs)
        {
            for (auto& inputAction : inputActions)
            {
                const bool currentState = myXInput->IsPressed(static_cast<unsigned short>(inputAction.keyCode));
                const bool prevState = inputAction.states[ACTIVE_STATE];
                inputAction.states[JUST_ACTIVATED_STATE] = !prevState && currentState;
                inputAction.states[JUST_DEACTIVATED_STATE] = prevState && !currentState;
                inputAction.states[ACTIVE_STATE] = currentState;
            }
        }
        
        for (auto& inputActions : myGamePadAnalogInputs)
        {
            for (auto& inputAction : inputActions)
            {
                const float prevValue = inputAction.activeValue;
                switch (inputAction.inputType)
                {
                case GamePadAnalogInput::LSLeft:
                    {
                        inputAction.activeValue = FMath::Abs(Tga::Clamp(
                                static_cast<float>(state->sThumbLX) / (SHORT_MAX + 1), -1, 0)
                        );
                        break;
                    }
                case GamePadAnalogInput::LSRight:
                    {
                        inputAction.activeValue = FMath::Abs(Tga::Clamp(
                                static_cast<float>(state->sThumbLX) / (SHORT_MAX + 1), 0, 1)
                        );
                        break;
                    }
                case GamePadAnalogInput::LSUp:
                    {
                        inputAction.activeValue = FMath::Abs(Tga::Clamp(
                                static_cast<float>(state->sThumbLY) / (SHORT_MAX + 1), 0, 1)
                        );
                        break;
                    }
                case GamePadAnalogInput::LSDown:
                    {
                        inputAction.activeValue = FMath::Abs(Tga::Clamp(
                                static_cast<float>(state->sThumbLY) / (SHORT_MAX + 1), -1, 0)
                        );
                        break;
                    }
                case GamePadAnalogInput::RSLeft:
                    {
                        inputAction.activeValue = FMath::Abs(Tga::Clamp(
                                static_cast<float>(state->sThumbRX) / (SHORT_MAX + 1), -1, 0)
                        );
                        break;
                    }
                case GamePadAnalogInput::RSRight:
                    {
                        inputAction.activeValue = FMath::Abs(Tga::Clamp(
                                static_cast<float>(state->sThumbRX) / (SHORT_MAX + 1), 0, 1)
                        );
                        break;
                    }
                case GamePadAnalogInput::RSUp:
                    {
                        inputAction.activeValue = FMath::Abs(Tga::Clamp(
                                static_cast<float>(state->sThumbRY) / (SHORT_MAX + 1), 0, 1)
                        );
                        break;
                    }
                case GamePadAnalogInput::RSDown:
                    {
                        inputAction.activeValue = FMath::Abs(Tga::Clamp(
                                static_cast<float>(state->sThumbRY) / (SHORT_MAX + 1), -1, 0)
                        );
                        break;
                    }
                case GamePadAnalogInput::LTrigger:
                    {
                        inputAction.activeValue = FMath::Abs(
                                static_cast<float>(state->bLeftTrigger) / (BYTE_MAX + 1)
                        );
                        break;
                    }
                case GamePadAnalogInput::RTrigger:
                    {
                        inputAction.activeValue = FMath::Abs(
                                static_cast<float>(state->bRightTrigger) / (BYTE_MAX + 1)
                        );
                        break;
                    }
                default:
                    {
                        inputAction.activeValue = 0.0f;
                        break;
                    }
                }
                
                inputAction.states[JUST_ACTIVATED_STATE] = !(prevValue > DEADZONE) && (inputAction.activeValue > DEADZONE);
                inputAction.states[JUST_DEACTIVATED_STATE] = (prevValue > DEADZONE) && !(inputAction.activeValue > DEADZONE);
            }
        }
    }
    
    if (mouseMovedThisFrame)
    {
        myIsUsingMouse = true;
    }
    else if (leftStickMovedThisFrame)
    {
        myIsUsingMouse = false;
    }
}

void InputMapper::AddRumble(const float aLeftAmount, const float aRightAmount, const float aTime, const int aUserIndex)
{
    myRumbleInstances[myNextRumbleIndex].left = aLeftAmount;
    myRumbleInstances[myNextRumbleIndex].right = aRightAmount;
    myRumbleInstances[myNextRumbleIndex].time = aTime;
    myRumbleInstances[myNextRumbleIndex].userIndex = aUserIndex;
    myNextRumbleIndex = (myNextRumbleIndex + 1) % MAX_RUMBLE_INSTANCES;
}

void InputMapper::SetRumble(const float aLeftAmount, const float aRightAmount, const int aUserIndex)
{
    XINPUT_VIBRATION state;
    const float leftFloat = static_cast<float>(USHORT_MAX) * FMath::Clamp(aLeftAmount, 0.0f, 1.0f);
    const float rightFloat = static_cast<float>(USHORT_MAX) * FMath::Clamp(aRightAmount, 0.0f, 1.0f);
    state.wLeftMotorSpeed = static_cast<WORD>(leftFloat);
    state.wRightMotorSpeed = static_cast<WORD>(rightFloat);
    XInputSetState(aUserIndex, &state);
}

void InputMapper::CaptureMouse() const
{
    myInputManager->CaptureMouse();   
}

void InputMapper::ReleaseMouse() const
{
    myInputManager->ReleaseMouse(); 
}

void InputMapper::ShowMouse(const bool aShow) const
{
    if (aShow)
    {
        ::SetCursor(::LoadCursor(nullptr, IDC_ARROW)); 
    }
    else
    {
        ::SetCursor(nullptr); 
    }
}

bool InputMapper::GetIsUsingMouse() const
{
    return myIsUsingMouse;
}

void InputMapper::RenderCursorSprite(Tga::SpriteDrawer& aSpriteDrawer, const Tga::SpriteSharedData& aCrosshairData, const Tga::Sprite2DInstanceData& aCrosshairInstance) const
{
    if (myIsUsingMouse)
    {
        aSpriteDrawer.Draw(aCrosshairData, aCrosshairInstance);
    }
}

Tga::Vector2f InputMapper::GetMousePositionYUp() const
{
    const auto& engine = *Tga::Engine::GetInstance();
    const auto renderSize = engine.GetRenderSize();
    Tga::Vector2f mousePosition = myInputManager->GetMousePosition();
    
    return Tga::Vector2f{ mousePosition.x, static_cast<float>(renderSize.y) - mousePosition.y};
}

Tga::Vector2f InputMapper::GetMousePositionYDown() const
{
    return myInputManager->GetMousePosition();
}

Tga::Vector2f InputMapper::GetMouseDelta() const
{
    return myInputManager->GetMouseDelta();
}

Tga::Vector2f InputMapper::GetRightStickPosition() const
{
    const auto& engine = *Tga::Engine::GetInstance();
    const auto renderSize = engine.GetRenderSize();
    
    float x = myXInput->GetState()->sThumbRX;
    float y = myXInput->GetState()->sThumbRY;
    
    return {x, y};
}

Tga::Vector2f InputMapper::GetLeftStickPosition() const
{    
    const auto& engine = *Tga::Engine::GetInstance();
    const auto renderSize = engine.GetRenderSize();
    
    float x = myXInput->GetState()->sThumbLX;
    float y = myXInput->GetState()->sThumbLY;
    
    return {x, y};
}