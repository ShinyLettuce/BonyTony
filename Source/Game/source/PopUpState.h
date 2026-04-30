#pragma once
#include "SceneLoader.h"
#include "State.h"
#include "tge/sprite/sprite.h"

namespace PopUp
{
    inline auto locNextPopupType = SceneLoader::PickupType::None;
}

class PopUpState : public State
{
public:
    void Init(Timer* aTimer);
    
    void SetSequenceData(SceneLoader::PickupType aPickupType);
    void OnPush() override;
    
    StateUpdateResult Update() override;
    void Render() override;
    
private:
    
    float myTime;
    float myPopUpStartTime;
    float myPopUpDuration;
    int myLapAmount;
    bool myPopUpSequenceFinished;
    float myFadeDelay;
    float myFadeDuration;
    float myStopRotation;
    
    Tga::Vector2f myFinalSize;
    float mySizeMultiplier;
    
    Tga::Sprite2DInstanceData myPopUpInstance;
    Tga::SpriteSharedData myPopUpSharedData;
    
    Timer* myTimer = nullptr;
};
