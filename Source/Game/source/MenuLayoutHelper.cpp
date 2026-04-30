#include "MenuLayoutHelper.h"
#include "ResolutionManager.h"
#include <tge/engine.h>

Tga::Vector2f MenuLayoutHelper::CalculateButtonPosition(
	const Tga::Vector2f center,
    float scaledOffsetX,
    float scaledOffsetY,
    float scaledSpacingY,
    float scaledSpacingX,
    int index,
    float individualXOffset
)
{
    float buttonYOffset = scaledOffsetY - (static_cast<float>(index) * scaledSpacingY);
    float buttonXOffset = scaledOffsetX + (static_cast<float>(index) * scaledSpacingX) + individualXOffset;

    return center + Tga::Vector2f{ buttonXOffset, buttonYOffset };
}

void MenuLayoutHelper::InitSpriteFromTexture(
    const char* texturePath,
    Tga::SpriteSharedData& outShared,
    Tga::Sprite2DInstanceData& outInstance,
    Tga::Vector2f pivot,
    float sizeMultiplier
)
{
    const auto& engine = *Tga::Engine::GetInstance();

    outShared.myTexture = (texturePath && texturePath[0] != '\0')
        ? engine.GetTextureManager().GetTexture(texturePath)
        : nullptr;

    outInstance.myPivot = pivot;

    if (outShared.myTexture)
    {
        const Tga::Vector2f texSize{ outShared.myTexture->CalculateTextureSize() };
        outInstance.mySize = ResolutionManager::ScaleSize(
            texSize.x * sizeMultiplier,
            texSize.y * sizeMultiplier
        );
    }
    else
    {
        outInstance.mySize = { 0.0f, 0.0f };
    }
}

void MenuLayoutHelper::ApplySelectionVisuals(
    const ButtonLayoutConfig& cfg,
    Tga::Sprite2DInstanceData& selectionInstance,
    const Tga::Vector2f& selectionBaseSize,
    bool isHovering
)
{
    const float mul = isHovering ? cfg.selectionHoverMultiplier : cfg.selectionHoverNormalMultiplier;
    selectionInstance.mySize = selectionBaseSize * mul;
    selectionInstance.myRotation = isHovering ? cfg.selectionRotationRadians : 0.0f;
}

void MenuLayoutHelper::SetSpritePositionFromOffsets(
    Tga::Sprite2DInstanceData& instance,
    const Tga::Vector2f& center,
    float baseOffsetX,
    float baseOffsetY
)
{
    const float x = ResolutionManager::ScaleValue(baseOffsetX);
    const float y = ResolutionManager::ScaleValue(baseOffsetY);
    instance.myPosition = center + Tga::Vector2f{ x, y };
}

void MenuLayoutHelper::SyncOutlineToTarget(
    const Tga::Sprite2DInstanceData& target,
    Tga::Sprite2DInstanceData& outline,
    float sizeMultiplier
)
{
    outline.myPosition = target.myPosition;
    outline.myPivot = target.myPivot;
    outline.mySize = { target.mySize.x * sizeMultiplier, target.mySize.y * sizeMultiplier };
}