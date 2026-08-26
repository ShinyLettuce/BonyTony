#pragma once
#include <vector>
#include <array>

#include "CrateUpdater.h"
#include "Enemy.h"
#include "tge/sprite/sprite.h"
#include "Timer.h"
#include <tge/text/text.h>

class InputMapper;
class Camera;

enum class AimLineType
{
    First,
    Second,
};

struct AimLineContext
{
    AimLineType type;
    Tga::Vector2f aimOrigin;
    Tga::Vector2f aimDirection;
    
    std::vector<SceneLoader::TileConfig>& tiles;
    std::vector<Enemy>& enemies;
    std::vector<CrateUpdater::Crate>& crates;
};

class HUD
{
private:
    struct ShakeData;
public:
    void Init(const int aShotgunMaxClip, const int aRevolverMaxClip, const float aAimMagnitude, Timer* aTimer);
    
    void UpdateAimLine(const AimLineContext& aContext);
    
    void RenderClips(const int aShotgunClip, const bool aRevolverReady, const int aRevolverClip);
    void RenderAimline();
    void RenderHitPoint(Camera& aCamera);
    void RenderVignette();
    void RenderTimer();
    
    void PositionElements(const int aShotgunMaxClip, const int aRevolverMaxClip);

    void Shake(const float aAmplitude, const float aFrequency, const float aDuration, ShakeData& aShakeData);
    Tga::Vector2f GetShakeOffset(ShakeData& aShakeData);
    ShakeData& GetShellShakeData(int aIndex) { return myShellShakeData[aIndex]; }
    ShakeData& GetBulletShakeData(int aIndex) { return myBulletShakeData[aIndex]; }
    
private:
    struct ShakeData
    {
        float time;
        float amplitude;
        float frequency;
        float duration;
        float remaining;
    };

    struct UIConfig
    {
        //Textures  
        const char* aimLineTexture = "textures/UI/HUD/T_AimLine_C.dds";  
        const char* crosshairTexture = "textures/UI/HUD/T_Target_C.dds";  
        const char* shellTexture = "textures/UI/HUD/T_Shell_C.dds";  
        const char* spentShellTexture = "textures/UI/HUD/T_SpentShell_C.dds";  
        const char* bulletTexture = "textures/UI/HUD/T_Bullet_C.dds";  
        const char* spentBulletTexture = "textures/UI/HUD/T_SpentBullet_C.dds";
        const char* vignetteTexture = "textures/UI/HUD/UltraTemporaryVignette.png";

        float shellSizeMultiplier = 1.0f;
        float bulletSizeMultiplier = 1.0f; 

        float shellLeftMarginRef = -10.0f;     
        float shellBottomMarginRef = 275.0f;   
        float bulletRightMarginRef = 10.0f;    
        float bulletBottomMarginRef = 250.0f;   

        //Spacing in reference units (scaled)  
        float shellSpacingRef = -50.0f;   
        float bulletSpacingRef = -70.0f;  

        //Aim line  
        float aimLineGapSize = 100.f; 

        //Hit point visual tuning  
        float hitPointScaleNormal = 0.5f;    
        float hitPointScaleHighlight = 1.0f;   

        //Misc  
        float hitboxForgiveness = 0.9f;
    };

    inline static UIConfig UI{};

    struct Aimline
    {
        static constexpr int SPRITE_AMOUNT = 100;
        std::array<Tga::Sprite2DInstanceData, SPRITE_AMOUNT> aimlineInstances;
        Tga::Sprite2DInstanceData hitPointInstance;
        Tga::Vector2f origin;
        Tga::Vector2f end;
        bool shouldRender = true;
    };
    Tga::SpriteSharedData myAimlineData;
    Tga::SpriteSharedData myHitPointData;
    Aimline myFirstAimline;
    Aimline mySecondAimline;
    
    Tga::SpriteSharedData myShellData;
    Tga::SpriteSharedData myBulletData;
    Tga::SpriteSharedData mySpentShellData;
    Tga::SpriteSharedData mySpentBulletData;
    Tga::SpriteSharedData myVignetteData;
    
    std::vector<Tga::Sprite2DInstanceData> myShellInstances;
    std::vector<Tga::Sprite2DInstanceData> myBulletInstances;

    std::vector<ShakeData> myShellShakeData;
    std::vector<ShakeData> myBulletShakeData;

    Tga::Text myTimerText;
    
    Timer* myTimer;

    float myAimMagnitude = 0;
};
