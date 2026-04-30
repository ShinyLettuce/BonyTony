#pragma once
#include "InputMapper.h"

#include <tge/math/Vector2.h>
#include "tge/sprite/sprite.h"
#include "tge/text/Text.h"

#include <vector>

class Button
{
public:
    void Init(const Tga::Vector2f& aPosition, const Tga::Vector2f& aSize, const char* aTexturePath, InputMapper* aInputMapper, const Tga::Vector2f& aPivot = { 0.5f, 0.5f });
    bool IsPressed() const;
    bool Update(const bool aShouldHoverWithoutMouse);
    void Render();
    void SetHitboxOffset(float aLeftOffset, float aRightOffset)
    {
        myHitboxLeftOffset = aLeftOffset;
        myHitboxRightOffset = aRightOffset;
    }
    void SetUseSpriteSize(bool aEnabled)
    {
        myUseSpriteHitbox = aEnabled;
    }
    void SetRotationRadians(float aRadians)
    {
        myRotationRadians = aRadians;
    }

    float GetRotationRadians() const { return myRotationRadians; }

    void SetHoverScale(float aNormalMultiplier, float aHoverMultiplier)
    {
        myHoverNormalMultiplier = aNormalMultiplier;
        myHoverMultiplier = aHoverMultiplier;
    }

    bool IsHovering() const { return myIsHovering; }

    Tga::Vector2f GetPosition() const { return myPosition; }
    Tga::Vector2f GetSize() const { return mySize; }
private:
    Tga::Sprite2DInstanceData mySpriteInstance;
    Tga::SpriteSharedData mySpriteData;
    Tga::Text myText;
    Tga::Vector2f myPosition;
    Tga::Vector2f mySize;
    Tga::Vector2f myBaseSize = { 0.0f, 0.0f };

    Tga::Vector2f myHitboxSize = { 0.0f, 0.0f };

    Tga::Vector2f myPivot = { 0.5f, 0.5f };
    float myHitboxLeftOffset = 0.0f;
    float myHitboxRightOffset = 0.0f;

    float mySizeMultiplier = 1.0f;

    float myRotationRadians = 0.0f;

    float myHoverNormalMultiplier = 1.0f;
    float myHoverMultiplier = 1.0f;

    InputMapper* myInputMapper = nullptr;
    bool myUseSpriteHitbox = true;
    bool myIsHovering = false;
};