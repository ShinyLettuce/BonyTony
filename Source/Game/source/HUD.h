#pragma once
#include <vector>
#include <array>

#include "CrateUpdater.h"
#include "Enemy.h"
#include "tge/sprite/sprite.h"

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
public:
    void Init(const int aShotgunMaxClip, const int aRevolverMaxClip, const float aAimMagnitude);
    
    void UpdateAimLine(const AimLineContext& aContext);
    
    void RenderClips(const int aShotgunClip, const bool aRevolverReady, const int aRevolverClip) const;
    void RenderAimline();
    void RenderHitPoint(Camera& aCamera);
    
    void PositionElements(const int aShotgunMaxClip, const int aRevolverMaxClip);
    
private:
    struct UIConfig
    {
        //Textures  
        const char* aimLineTexture = "textures/UI/HUD/T_AimLine_C.dds";  
        const char* crosshairTexture = "textures/UI/HUD/T_Crosshair_C.dds";  
        const char* shellTexture = "textures/UI/HUD/T_Shell_C.dds";  
        const char* spentShellTexture = "textures/UI/HUD/T_SpentShell_C.dds";  
        const char* bulletTexture = "textures/UI/HUD/T_Bullet_C.dds";  
        const char* spentBulletTexture = "textures/UI/HUD/T_SpentBullet_C.dds";  

        float shellSizeMultiplier = 1.4f;
        float bulletSizeMultiplier = 1.1f; 

        float shellLeftMarginRef = -10.0f;     
        float shellBottomMarginRef = 300.0f;   
        float bulletRightMarginRef = 10.0f;    
        float bulletBottomMarginRef = 250.0f;   

        //Spacing in reference units (scaled)  
        float shellSpacingRef = -70.0f;   
        float bulletSpacingRef = -70.0f;  

        //Aim line  
        float aimLineGapSize = 100.f; 

        //Hit point visual tuning  
        float hitPointScaleNormal = 1.0f;    
        float hitPointScaleHighlight = 2.0f;   

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
    
    std::vector<Tga::Sprite2DInstanceData> myShellInstances;
    std::vector<Tga::Sprite2DInstanceData> myBulletInstances;
    
    float myAimMagnitude = 0;
};
