#include "PopUpState.h"

#include "MathUtils.h"
#include "ResolutionManager.h"
#include "tge/engine.h"
#include "tge/graphics/GraphicsEngine.h"
#include "tge/texture/TextureManager.h"

void PopUpState::Init(Timer* aTimer)
{
    myTimer = aTimer;
}

void PopUpState::SetSequenceData(const SceneLoader::PickupType aPickupType)
{
    const auto& engine = *Tga::Engine::GetInstance();
    const Tga::Vector2ui renderSize = engine.GetRenderSize();
    const Tga::Vector2f resolution = { static_cast<float>(renderSize.x), static_cast<float>(renderSize.y) };
    
    myTime = 0.f;
    myPopUpSequenceFinished = false;
    
    if (aPickupType == SceneLoader::PickupType::PowerShot)
    {
        myPopUpSharedData.myTexture = engine.GetTextureManager().GetTexture("textures/UI/SplashArt/T_PowershotSplash_C.dds");
    }
    else
    {
        myPopUpSharedData.myTexture = engine.GetTextureManager().GetTexture("textures/UI/SplashArt/T_RevolverSplash_C.dds");
    }
    myPopUpInstance.myColor.a = 1.f;
    myPopUpInstance.myPivot = 0.5f;
    myPopUpInstance.myPosition = resolution * 0.5f;
    myPopUpInstance.mySize = 0.f;
    myPopUpInstance.myRotation = 0.f;
    mySizeMultiplier = 0.6f;
    myPopUpStartTime = 1.f;
    myPopUpDuration = 2.f;
    myLapAmount = 10;
    myFadeDelay = 2.f;
    myFadeDuration = 3.f;
    myStopRotation = 0.2f;
    
    const float uiScale = ResolutionManager::GetUIScale();
    const Tga::Vector2ui base = myPopUpSharedData.myTexture->CalculateTextureSize();
    const Tga::Vector2f scaled = {static_cast<float>(base.x) * uiScale * mySizeMultiplier, static_cast<float>(base.y) * uiScale * mySizeMultiplier};
    myFinalSize = scaled;
}

void PopUpState::OnPush()
{
    SetSequenceData(PopUp::locNextPopupType);
}

StateUpdateResult PopUpState::Update()
{
    const float deltaTime = myTimer->GetDeltaTime();
    
    myTime += deltaTime;
    
    if (!(myTime >= myPopUpStartTime))
    {
        return StateUpdateResult::CreateContinue();
    }
    
    if (!myPopUpSequenceFinished)
    {
        static constexpr float Pi2f = 2.f * FMath::Pi;
        const float fullSpin = Pi2f * static_cast<float>(myLapAmount);
        const float percentage = MathUtils::Clamp01((myTime - myPopUpStartTime) / (myPopUpDuration - myPopUpStartTime));
        myPopUpInstance.mySize = myFinalSize * percentage;
        myPopUpInstance.myRotation = fullSpin * percentage + myStopRotation;
        
        if (percentage >= 1.f) 
        {
            myPopUpInstance.myRotation += myStopRotation;
            myPopUpSequenceFinished = true;
            myTime = 0.f;
        }
    }
    else
    {
        if (myTime >= myFadeDelay)
        {
            const float percentage = MathUtils::Clamp01((myTime - myFadeDelay) / (myFadeDuration - myFadeDelay));
            myPopUpInstance.myColor.a = 1.f - percentage;
            if (percentage >= 1.f)
            {
                return StateUpdateResult::CreatePop();
            }
        }
    }
    
    return StateUpdateResult::CreateContinue();
}

void PopUpState::Render()
{
    const Tga::Engine& engine = *Tga::Engine::GetInstance();
    Tga::GraphicsEngine& graphicsEngine = engine.GetGraphicsEngine();
    Tga::SpriteDrawer& spriteDrawer = graphicsEngine.GetSpriteDrawer();
    
    spriteDrawer.Draw(myPopUpSharedData, myPopUpInstance);
}