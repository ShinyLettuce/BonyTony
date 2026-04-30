#include "CreditsState.h"
#include "ResolutionManager.h"
#include "InputMapper.h"
#include "MenuLayoutHelper.h"

#include <tge/Engine.h>
#include <tge/drawers/SpriteDrawer.h>
#include <tge/graphics/GraphicsEngine.h>
#include <tge/texture/TextureManager.h>

#include "AudioManager.h"
#include "Go.h"

void CreditsState::Init(InputMapper* aInputMapper)
{
    myInputMapper = aInputMapper;

    const auto& engine = *Tga::Engine::GetInstance();

    mySpriteData.myTexture = engine.GetTextureManager().GetTexture(UI.backgroundTexture);
    mySpriteInstance.myPivot = { 0.5f, 0.5f };

    PositionElements();
}

void CreditsState::PositionElements()
{
	const auto& engine = *Tga::Engine::GetInstance();
	const auto renderSize = engine.GetRenderSize();
	const Tga::Vector2f resolution = { static_cast<float>(renderSize.x), static_cast<float>(renderSize.y) };

	//Scale to cover screen while maintaining aspect ratio
	const Tga::Vector2f textureSize{ mySpriteData.myTexture->CalculateTextureSize() };
	float texAspect = textureSize.x / textureSize.y;
	float screenAspect = resolution.x / resolution.y;

	if (screenAspect > texAspect)
	{
		//Screen is wider - fit to width
		mySpriteInstance.mySize.x = resolution.x;
		mySpriteInstance.mySize.y = resolution.x / texAspect;
	}
	else
	{
		//Screen is taller - fit to height
		mySpriteInstance.mySize.y = resolution.y;
		mySpriteInstance.mySize.x = resolution.y * texAspect;
	}

	mySpriteInstance.myPosition = resolution * 0.5f;

	const Tga::Vector2f center = mySpriteInstance.myPosition;

	MenuLayoutConfig layoutConfig;
	layoutConfig.baseButtonOffsetX = UI.baseButtonOffsetX;
	layoutConfig.baseButtonOffsetY = UI.baseButtonOffsetY;
	layoutConfig.baseButtonSpacingY = UI.baseButtonSpacingY;
	layoutConfig.baseButtonSpacingX = UI.baseButtonSpacingX;
	layoutConfig.useTextureSize = UI.buttonUseTextureSize;
	layoutConfig.buttonSizeMultiplier = UI.buttonSizeMultiplier;
	layoutConfig.hasSelectionBar = UI.buttonHasSelectionBar;
	layoutConfig.selectionBarTexture = UI.selectionBarTexture;
	layoutConfig.selectionBarUseTextureSize = UI.selectionBarUseTextureSize;
	layoutConfig.selectedBarSizeMultiplier = UI.selectionBarSizeMultiplier;
	layoutConfig.selectionBarPivot = UI.selectionBarPivot;

	std::array<ButtonLayoutConfig, 1> buttonConfigs = { {
		{
			.texturePath = UI.returnButtonTexture,
			.induvidualXOffset = UI.returnInduvidualXOffset,
			.hitboxLeftOffset = UI.returnHitboxLeftOffset,
			.hitboxRightOffset = UI.returnHitboxRightOffset,
			.selectionBarXOffset = UI.returnSelectionBarXOffset,
			.pivot = UI.returnPivot,
			.rotationRadians = UI.returnHoverRotationRadians,
			.hoverNormalMultiplier = UI.returnHoverNormalMultiplier,
			.hoverMultiplier = UI.returnHoverMultiplier,
			.selectionRotationRadians = UI.returnSelectionHoverRotationRadians,
			.selectionHoverNormalMultiplier = UI.returnSelectionHoverNormalMultiplier,
			.selectionHoverMultiplier = UI.returnSelectionHoverMultiplier
		}
	} };

	std::array<Button, 1> buttons;

	MenuLayoutHelper::PositionButtons(
		layoutConfig,
		buttonConfigs,
		buttons,
		myButtonPositions,
		center,
		myInputMapper
	);

	MenuLayoutHelper::SetupSelectionBar(
		layoutConfig,
		buttonConfigs,
		mySelectionBarXOffsets,
		myBarSpriteInstance,
		myBarSpriteData
	);

	myBackButton = buttons[0];

	if (myBarSpriteData.myTexture)
	{
		myBarSpriteInstance.myPosition = myButtonPositions[myButtonIndex] + Tga::Vector2f{ mySelectionBarXOffsets[myButtonIndex], 0.0f };
	}
}

StateUpdateResult CreditsState::Update()
{
	if (myInputMapper->IsActionJustActivated(GameAction::UICancel))
	{
		AudioManager::GetAudioPoolByHandle(AudioHandles::uiBack).Play();
		return StateUpdateResult::CreatePop();
	}

	// if (myInputMapper->IsActionJustActivated(GameAction::UIConfirm))
	// {
	//     return StateUpdateResult::CreatePop();
	// }

	myIsBackButtonHovered = myBackButton.Update(myIsBackButtonHovered);

	if (myIsBackButtonHovered && myBarSpriteData.myTexture)
	{
		myBarSpriteInstance.myPosition =
			myButtonPositions[0] + Tga::Vector2f{ mySelectionBarXOffsets[0], 0.0f };
	}

	if (myBackButton.IsPressed())
	{
		AudioManager::GetAudioPoolByHandle(AudioHandles::clickButton).Play();
		return StateUpdateResult::CreatePop();
	}

	return StateUpdateResult::CreateContinue();
}

void CreditsState::Render()
{
    const Tga::Engine& engine = *Tga::Engine::GetInstance();
    Tga::SpriteDrawer& spriteDrawer = engine.GetGraphicsEngine().GetSpriteDrawer();

    spriteDrawer.Draw(mySpriteData, mySpriteInstance);

	if (myIsBackButtonHovered && myBarSpriteData.myTexture)
	{
		spriteDrawer.Draw(myBarSpriteData, myBarSpriteInstance);
	}

    myBackButton.Render();
}

void CreditsState::OnGainFocus()
{
	SetMouseCaptureEnabled(false);
	AudioManager::GetAudioPoolByHandle(AudioHandles::windowChange).Play();
}
