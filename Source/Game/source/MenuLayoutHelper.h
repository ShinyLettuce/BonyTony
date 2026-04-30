#pragma once

#include <tge/math/vector2.h>
#include <tge/sprite/sprite.h>
#include <tge/engine.h>
#include <tge/texture/TextureManager.h>

#include <array>
#include <functional>

#include "Button.h"
#include "ResolutionManager.h"
#include "InputMapper.h"

namespace Tga
{
	struct Sprite2DInstanceData;
	struct SpriteSharedData;
}

struct ButtonLayoutConfig
{
	const char* texturePath = nullptr;
	float induvidualXOffset = 0.0f;
	float hitboxLeftOffset = 0.0f;
	float hitboxRightOffset = 0.0f;
	float hitboxTopOffset = 0.0f;
	float hitboxBottomOffset = 0.0f;
	float selectionBarXOffset = 0.0f;
	Tga::Vector2f pivot = { 0.5f, 0.5f };

	float rotationRadians = 0.0f;
	float hoverNormalMultiplier = 1.0f;
	float hoverMultiplier = 1.0f;

	float selectionRotationRadians = 0.0f;
	float selectionHoverNormalMultiplier = 1.0f;
	float selectionHoverMultiplier = 1.0f;
};

struct MenuLayoutConfig
{
	float baseButtonOffsetX = 0.0f;
	float baseButtonOffsetY = 0.0f;
	float baseButtonSpacingY = 0.0f;
	float baseButtonSpacingX = 0.0f;

	bool useTextureSize = false;
	
	float baseButtonWidth = 200.0f;
	float baseButtonHeight = 100.0f;
	float buttonSizeMultiplier = 1.0f;

	bool hasSelectionBar = false;
	const char* selectionBarTexture = nullptr;
	bool selectionBarUseTextureSize = true;
	float baseSelectionBarWidth = 1000.0f;
	float baseSelectionBarHeight = 150.0f;
	float selectedBarSizeMultiplier = 1.0f;
	Tga::Vector2f selectionBarPivot = { 0.5f, 0.5f };
};

class MenuLayoutHelper
{
public:

	template <size_t ButtonCount>
	static void PositionButtons(
		const MenuLayoutConfig& layoutConfig,
		const std::array<ButtonLayoutConfig, ButtonCount>& buttonConfigs,
		std::array<Button, ButtonCount>& buttons,
		std::array<Tga::Vector2f, ButtonCount>& buttonPositions,
		const Tga::Vector2f& center,
		InputMapper* inputMapper
	);

	template <size_t ButtonCount>
	static void ApplyButtonVisualConfig(
		const std::array<ButtonLayoutConfig, ButtonCount>& buttonConfigs,
		std::array<Button, ButtonCount>& buttons
	);

	template <size_t ButtonCount>
	static void SetupSelectionBar(
		const MenuLayoutConfig& layoutConfig,
		const std::array<ButtonLayoutConfig, ButtonCount>& buttonConfigs,
		std::array<float, ButtonCount>& selectionBarXOffsets,
		Tga::Sprite2DInstanceData& barInstance,
		Tga::SpriteSharedData& barData
	);

	static void ApplySelectionVisuals(
		const ButtonLayoutConfig& cfg,
		Tga::Sprite2DInstanceData& selectionInstance,
		const Tga::Vector2f& selectionBaseSize,
		bool isHovering
	);

	static Tga::Vector2f CalculateButtonPosition(
		const Tga::Vector2f canter,
		float scaledOffsetX,
		float scaledOffsetY,
		float scaledSpacingY,
		float scaledSpacingX,
		int index,
		float induvidualXOffset = 0.0f
	);

	static void InitSpriteFromTexture(
		const char* texturePath,
		Tga::SpriteSharedData& outShared,
		Tga::Sprite2DInstanceData& outInstance,
		Tga::Vector2f pivot = { 0.5f, 0.5f },
		float sizeMultiplier = 1.0f
	);

	static void SetSpritePositionFromOffsets(
		Tga::Sprite2DInstanceData& instance,
		const Tga::Vector2f& center,
		float baseOffsetX,
		float baseOffsetY
	);

	static void SyncOutlineToTarget(
		const Tga::Sprite2DInstanceData& target,
		Tga::Sprite2DInstanceData& outline,
		float sizeMultiplier = 0.0f
	);
};

template <size_t ButtonCount>
void MenuLayoutHelper::PositionButtons(
	const MenuLayoutConfig& layoutConfig,
	const std::array<ButtonLayoutConfig, ButtonCount>& buttonConfigs,
	std::array<Button, ButtonCount>& buttons,
	std::array<Tga::Vector2f, ButtonCount>& buttonPositions,
	const Tga::Vector2f& center,
	InputMapper* inputMapper
)
{
	const auto& engine = *Tga::Engine::GetInstance();

	float scaledOffsetX = ResolutionManager::ScaleValue(layoutConfig.baseButtonOffsetX);
	float scaledOffsetY = ResolutionManager::ScaleValue(layoutConfig.baseButtonOffsetY);
	float scaledSpacingY = ResolutionManager::ScaleValue(layoutConfig.baseButtonSpacingY);
	float scaledSpacingX = ResolutionManager::ScaleValue(layoutConfig.baseButtonSpacingX);

	for (size_t i = 0; i < ButtonCount; ++i)
	{
		const ButtonLayoutConfig& btnConfig = buttonConfigs[i];

		Tga::Vector2f  scaledButtonSize;
		if (layoutConfig.useTextureSize)
		{
			auto* buttonTexture = engine.GetTextureManager().GetTexture(btnConfig.texturePath);
			Tga::Vector2f buttonTextureSize{ buttonTexture->CalculateTextureSize() };
			scaledButtonSize = ResolutionManager::ScaleSize(
				buttonTextureSize.x * layoutConfig.buttonSizeMultiplier,
				buttonTextureSize.y * layoutConfig.buttonSizeMultiplier
			);

		}
		else
		{
			scaledButtonSize = ResolutionManager::ScaleSize(
				layoutConfig.baseButtonWidth * layoutConfig.buttonSizeMultiplier,
				layoutConfig.baseButtonHeight * layoutConfig.buttonSizeMultiplier
			);
		}

		float adjustedOffsetX = scaledOffsetX;

		if (btnConfig.pivot.x == 0.0f)
		{
			adjustedOffsetX -= (scaledButtonSize.x * 0.5f);
		}

		float scaledInduvidualXOffset = ResolutionManager::ScaleValue(btnConfig.induvidualXOffset);

		buttonPositions[i] = CalculateButtonPosition(
			center,
			adjustedOffsetX,
			scaledOffsetY,
			scaledSpacingY,
			scaledSpacingX,
			static_cast<int>(i),
			scaledInduvidualXOffset
		);

		buttons[i].Init(
			buttonPositions[i],
			scaledButtonSize,
			btnConfig.texturePath,
			inputMapper,
			btnConfig.pivot
		);

		if (btnConfig.hitboxLeftOffset != 0.0f || btnConfig.hitboxRightOffset != 0.0f ||
			btnConfig.hitboxBottomOffset != 0.0f || btnConfig.hitboxTopOffset != 0.0f)
		{
			float scaledLeftOffset = ResolutionManager::ScaleValue(btnConfig.hitboxLeftOffset);
			float scaledRightOffset = ResolutionManager::ScaleValue(btnConfig.hitboxRightOffset);
			buttons[i].SetHitboxOffset(scaledLeftOffset, scaledRightOffset);
		}

		buttons[i].SetRotationRadians(btnConfig.rotationRadians);
		buttons[i].SetHoverScale(btnConfig.hoverNormalMultiplier, btnConfig.hoverMultiplier);
	}
}

template <size_t ButtonCount>
void MenuLayoutHelper::ApplyButtonVisualConfig(
	const std::array<ButtonLayoutConfig, ButtonCount>& buttonConfigs,
	std::array<Button, ButtonCount>& buttons
)
{
	for (size_t i = 0; i < ButtonCount; ++i)
	{
		buttons[i].SetRotationRadians(buttonConfigs[i].rotationRadians);
		buttons[i].SetHoverScale(buttonConfigs[i].hoverNormalMultiplier, buttonConfigs[i].hoverMultiplier);
	}
}

template <size_t ButtonCount>
void MenuLayoutHelper::SetupSelectionBar(
	const MenuLayoutConfig& layoutConfig,
	const std::array<ButtonLayoutConfig, ButtonCount>& buttonConfigs,
	std::array<float, ButtonCount>& selectionBarXOffsets,
	Tga::Sprite2DInstanceData& barInstance,
	Tga::SpriteSharedData& barData
)
{
	if (!layoutConfig.hasSelectionBar)
		return;

	const auto& engine = *Tga::Engine::GetInstance();

	barData.myTexture = engine.GetTextureManager().GetTexture(layoutConfig.selectionBarTexture);
	barInstance.myPivot = layoutConfig.selectionBarPivot;

	if (layoutConfig.selectionBarUseTextureSize)
	{
		Tga::Vector2f barTextureSize{ barData.myTexture->CalculateTextureSize() };
		barInstance.mySize = ResolutionManager::ScaleSize(
		barTextureSize.x * layoutConfig.selectedBarSizeMultiplier,
		barTextureSize.y * layoutConfig.selectedBarSizeMultiplier
		);
	}
	else
	{
		barInstance.mySize = ResolutionManager::ScaleSize(
			layoutConfig.baseSelectionBarWidth * layoutConfig.selectedBarSizeMultiplier,
			layoutConfig.baseSelectionBarHeight * layoutConfig.selectedBarSizeMultiplier
		);
	}

	for (size_t i = 0; i < ButtonCount; ++i)
	{
		selectionBarXOffsets[i] = ResolutionManager::ScaleValue(buttonConfigs[i].selectionBarXOffset);
	}
}

