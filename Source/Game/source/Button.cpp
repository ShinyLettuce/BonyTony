#include "Button.h"

#include <tge/Engine.h>
#include <tge/drawers/SpriteDrawer.h>
#include <tge/graphics/GraphicsEngine.h>
#include <tge/texture/TextureManager.h>

#include "AudioManager.h"

void Button::Init(const Tga::Vector2f& aPosition, const Tga::Vector2f& aSize, const char* aTexturePath, InputMapper* aInputMapper, const Tga::Vector2f& aPivot)
{
    Tga::Engine& engine = *Tga::Engine::GetInstance();

    myPosition = aPosition;
    mySize = aSize;
    myPivot = aPivot;
    myInputMapper = aInputMapper;

    mySpriteData.myTexture = engine.GetTextureManager().GetTexture(aTexturePath);
    myBaseSize = aSize;
    mySpriteInstance.mySize = aSize;
    mySpriteInstance.myPivot = aPivot;
    mySpriteInstance.myPosition = aPosition;
    mySpriteInstance.myRotation = 0.0f;

    myHitboxSize = aSize;
}

bool Button::IsPressed() const
{
    if (!myUseSpriteHitbox)
    {
        return false;
    }

    if (myInputMapper->IsActionJustActivated(GameAction::UILeftClick))
    {
        const Tga::Vector2f mousePosition = myInputMapper->GetMousePositionYUp();

        const Tga::Vector2f sizeForHitbox = myHitboxSize;

        float leftBound = myPosition.x - (sizeForHitbox.x * myPivot.x) + myHitboxLeftOffset;
        float rightBound = myPosition.x + (sizeForHitbox.x * (1.0f - myPivot.x)) + myHitboxRightOffset;
        float topBound = myPosition.y + (sizeForHitbox.y * (1.0f - myPivot.y));
        float bottomBound = myPosition.y - (sizeForHitbox.y * myPivot.y);

        if (mousePosition.x > leftBound && mousePosition.x < rightBound &&
            mousePosition.y > bottomBound && mousePosition.y < topBound)
        {
            return true;
        }
    }
    return false;
}

bool Button::Update(const bool aShouldHoverWithoutMouse)
{
    const bool previousHoverStatus = myIsHovering;
    bool isMouseHovering = false;
    if (myUseSpriteHitbox)
    {
        const Tga::Vector2f mousePosition = myInputMapper->GetMousePositionYUp();

        const Tga::Vector2f sizeForHitbox = myHitboxSize;

        float leftBound = myPosition.x - (sizeForHitbox.x * myPivot.x) + myHitboxLeftOffset;
        float rightBound = myPosition.x + (sizeForHitbox.x * (1.0f - myPivot.x)) + myHitboxRightOffset;
        float topBound = myPosition.y + (sizeForHitbox.y * (1.0f - myPivot.y));
        float bottomBound = myPosition.y - (sizeForHitbox.y * myPivot.y);

        isMouseHovering =
            mousePosition.x > leftBound && mousePosition.x < rightBound &&
            mousePosition.y > bottomBound && mousePosition.y < topBound;
    }

    const bool shouldHover = (isMouseHovering || aShouldHoverWithoutMouse);

    if (shouldHover)
    {
        mySpriteInstance.myColor = Tga::Color(1, 1, 1, 1);
        myIsHovering = true;
    }
    else
    {
        mySpriteInstance.myColor = Tga::Color(1, 1, 1, 1);
        myIsHovering = false;
    }

    {
        const float mul = myIsHovering ? myHoverMultiplier : myHoverNormalMultiplier;
        mySpriteInstance.mySize = myBaseSize * mul;
        mySize = mySpriteInstance.mySize;
    }

    mySpriteInstance.myRotation = myIsHovering ? myRotationRadians : 0.0f;

    if (!previousHoverStatus && myIsHovering)
    {
        AudioManager::GetAudioPoolByHandle(AudioHandles::hoverButton).Play();
    }

    return myIsHovering;
}

void Button::Render()
{
    const Tga::Engine& engine = *Tga::Engine::GetInstance();
    Tga::SpriteDrawer& spriteDrawer = engine.GetGraphicsEngine().GetSpriteDrawer();

    myText.Render();
    spriteDrawer.Draw(mySpriteData, mySpriteInstance);
}