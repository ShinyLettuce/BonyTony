#pragma once
#include <array>
#include <bitset>
#include <vector>

#include "tge/math/Vector2.h"

namespace Tga
{
    struct Sprite2DInstanceData;
    struct SpriteSharedData;
    class SpriteDrawer;
    class XInput;
    class InputManager;
}

enum class GameAction
{
    UIUp,
    UIDown,
    UILeft,
    UIRight,
    UIConfirm,
    UICancel,
    UILeftClick,
    Pause,
    PlayerShootShotgun,
    PlayerShootRevolver,
    PlayerPowerShotOverride,
    DebugReload,
    DebugReleaseCursor,
    DebugCaptureCursor,
    SkipCutscene,
    LSPress,
    RSPress,
    Count,
};

enum class ActionState
{
    JustActivated,
    JustDeactivated,
    IsActive,
};

enum class MouseInput
{
    Left,
    Right,
    Up,
    Down,
};

enum class GamePadAnalogInput
{
    LSLeft,
    LSRight,
    LSUp,
    LSDown,
    RSLeft,
    RSRight,
    RSUp,
    RSDown,
    LTrigger,
    RTrigger,
};

struct RumbleInstance
{
    float left;
    float right;
    float time;
    int userIndex;
};

struct InputAction
{
    int keyCode{};
    std::bitset<3> states{};
};

struct AnalogInputAction
{
    GamePadAnalogInput inputType{};
    float activeValue{};
    std::bitset<2> states{};
};

struct MouseInputAction
{
    MouseInput inputType{};
    float activeValue{};
    std::bitset<2> states{};
};

class InputMapper
{
public:
    
    InputMapper(const Tga::InputManager* aInputManager, Tga::XInput* aXInput);
    
    void BindKeyAction(const int aKeyCode, const GameAction aAction);
    void BindMouseAction(const MouseInput aMouseInput, const GameAction aAction);
    void BindGamePadAnalogAction(const GamePadAnalogInput aGamePadAnalogInput, const GameAction aAction);
    void BindGamePadButtonAction(const int aButtonCode, const GameAction aAction);
    bool IsActionActive(const GameAction aAction) const;
    bool IsActionJustActivated(const GameAction aAction) const;
    bool IsActionJustDeactivated(const GameAction aAction) const;
    float GetActionActiveValue(const GameAction aAction) const;
    void Update(const float aTimeDelta);
    
    void AddRumble(const float aLeftAmount, const float aRightAmount, const float aTime = -1.0f, const int aUserIndex = 0);
    void SetRumble(float aLeftAmount, float aRightAmount, int aUserIndex = 0);
    
    void CaptureMouse() const;
    void ReleaseMouse() const;
    
    void ShowMouse(const bool aShow) const;
    
    bool GetIsUsingMouse() const;
    
    void RenderCursorSprite(Tga::SpriteDrawer& aSpriteDrawer, const Tga::SpriteSharedData& aCrosshairData, const Tga::Sprite2DInstanceData& aCrosshairInstance) const;
    
    Tga::Vector2f GetMousePositionYUp() const;
    Tga::Vector2f GetMousePositionYDown() const;
    Tga::Vector2f GetMouseDelta() const;
    
    Tga::Vector2f GetRightStickPosition() const;
    Tga::Vector2f GetLeftStickPosition() const;
    
private:
    static constexpr int STICK_USE_DEADZONE = 20000;
    static constexpr float MOUSE_USE_DEADZONE = 0.f;
    static constexpr float DEADZONE = 0.3f;
    
    static constexpr size_t JUST_ACTIVATED_STATE = 0;
    static constexpr size_t JUST_DEACTIVATED_STATE = 1;
    static constexpr size_t ACTIVE_STATE = 2;
    static constexpr size_t MAX_RUMBLE_INSTANCES = 8;
    std::array<RumbleInstance, MAX_RUMBLE_INSTANCES> myRumbleInstances{};
    std::array<std::vector<InputAction>, static_cast<size_t>(GameAction::Count)> myKeyActionInputs;
    std::array<std::vector<MouseInputAction>, static_cast<size_t>(GameAction::Count)> myMouseActionInputs;
    std::array<std::vector<AnalogInputAction>, static_cast<size_t>(GameAction::Count)> myGamePadAnalogInputs;
    std::array<std::vector<InputAction>, static_cast<size_t>(GameAction::Count)> myGamepadActionInputs;
    int myNextRumbleIndex = 0;
    const Tga::InputManager* myInputManager = nullptr;
    Tga::XInput* myXInput = nullptr;
    
    Tga::Vector2f myMousePosition;
    
    bool myIsUsingMouse = true;
};