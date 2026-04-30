#include "QuitState.h"
#include "ResolutionManager.h"

#include "SceneLoader.h"
#include <tge/Engine.h>
#include <tge/drawers/SpriteDrawer.h>
#include <tge/graphics/GraphicsEngine.h>
#include <tge/texture/TextureManager.h>

#include "MathUtils.h"
#include "AudioManager.h"
#include "Go.h"

void QuitState::Init(InputMapper* aInputMapper, OpenContext aContext)
{
	myInputMapper = aInputMapper;
	myButtonIndex = 0;
	myMouseHoveringAnyUI = false;

	myOpenContext = aContext;

	const auto& engine = *Tga::Engine::GetInstance();

	const char* bg = (myOpenContext == OpenContext::Pause)
		? UI.pausebackgroundTexture
		: UI.mainMenubackgroundTexture;

	myBackgroundSpriteData.myTexture = engine.GetTextureManager().GetTexture(bg);
	myBackgroundSpriteInstance.myPivot = { 0.5f, 0.5f };

	PositionElements();
}

void QuitState::OnPush()
{
	if (myInputMapper == nullptr)
	{
		return;
	}

	myOpenContext = myNextOpenContext;

	const auto& engine = *Tga::Engine::GetInstance();

	const char* bg = (myOpenContext == OpenContext::Pause)
		? UI.pausebackgroundTexture
		: UI.mainMenubackgroundTexture; 

	myBackgroundSpriteData.myTexture = engine.GetTextureManager().GetTexture(bg); 
	PositionElements(); 
}

void QuitState::PositionElements()
{
	const auto& engine = *Tga::Engine::GetInstance();
	const auto renderSize = engine.GetRenderSize();
	const Tga::Vector2f resolution = { static_cast<float>(renderSize.x), static_cast<float>(renderSize.y) };

	myBackgroundSpriteInstance.myPosition = resolution * 0.5f;

	if (myBackgroundSpriteData.myTexture)
	{
		const Tga::Vector2f textureSize{ myBackgroundSpriteData.myTexture->CalculateTextureSize() };
		const float texAspect = textureSize.x / textureSize.y;
		const float screenAspect = resolution.x / resolution.y;

		if (UI.backgroundUseCoverScale) 
		{
			if (screenAspect > texAspect)
			{
				myBackgroundSpriteInstance.mySize.x = resolution.x;
				myBackgroundSpriteInstance.mySize.y = resolution.x / texAspect;
			}
			else
			{
				myBackgroundSpriteInstance.mySize.y = resolution.y;
				myBackgroundSpriteInstance.mySize.x = resolution.y * texAspect;
			}
		}
		else
		{
			myBackgroundSpriteInstance.mySize.y = resolution.y;
			myBackgroundSpriteInstance.mySize.x = resolution.y * texAspect;
		}
	}

	const Tga::Vector2f center = myBackgroundSpriteInstance.myPosition;

	MenuLayoutConfig layoutConfig;
	layoutConfig.baseButtonOffsetX = UI.baseButtonOffsetX;
	layoutConfig.baseButtonOffsetY = UI.baseButtonOffsetY;
	layoutConfig.baseButtonSpacingY = UI.baseButtonSpacingY;
	layoutConfig.baseButtonSpacingX = UI.baseButtonSpacingX;

	layoutConfig.useTextureSize = UI.buttonUseTextureSize;
	layoutConfig.baseButtonWidth = UI.baseButtonWidth;
	layoutConfig.baseButtonHeight = UI.baseButtonHeight;
	layoutConfig.buttonSizeMultiplier = UI.buttonSizeMultiplier;

	layoutConfig.hasSelectionBar = UI.buttonHasSelectionBar;
	layoutConfig.selectionBarTexture = UI.selectionBarTexture;
	layoutConfig.selectionBarUseTextureSize = UI.selectionBarUseTextureSize; 
	layoutConfig.selectedBarSizeMultiplier = UI.selectionBarSizeMultiplier; 
	layoutConfig.selectionBarPivot = UI.selectionBarPivot; 

	myButtonConfigs[static_cast<int>(ButtonType::Confirm)] =
	{
		.texturePath = UI.yesButtonTexture,
		.induvidualXOffset = UI.yesButtonInduvidualXOffset,
		.hitboxLeftOffset = UI.yesButtonHitboxLeftOffset,
		.hitboxRightOffset = UI.yesButtonHitboxRightOffset,
		.selectionBarXOffset = UI.yesButtonSelectionBarXOffset,
		.pivot = UI.yesButtonPivot,
		.rotationRadians = UI.yesButtonHoverRotationRadians,
		.hoverNormalMultiplier = UI.yesButtonHoverNormalMultiplier,
		.hoverMultiplier = UI.yesButtonHoverMultiplier,
	};

	myButtonConfigs[static_cast<int>(ButtonType::Cancel)] =
	{
		.texturePath = UI.noButtonTexture,
		.induvidualXOffset = UI.noButtonInduvidualXOffset,
		.hitboxLeftOffset = UI.noButtonHitboxLeftOffset,
		.hitboxRightOffset = UI.noButtonHitboxRightOffset,
		.selectionBarXOffset = UI.noButtonSelectionBarXOffset,
		.pivot = UI.noButtonPivot,
		.rotationRadians = UI.noButtonHoverRotationRadians,
		.hoverNormalMultiplier = UI.noButtonHoverNormalMultiplier,
		.hoverMultiplier = UI.noButtonHoverMultiplier,
	};

	MenuLayoutHelper::PositionButtons(
		layoutConfig,
		myButtonConfigs,
		myButtons,
		myButtonPositions,
		center,
		myInputMapper
	);

	MenuLayoutHelper::SetupSelectionBar( 
		layoutConfig,
		myButtonConfigs,
		mySelectionBarXOffsets,
		mySelectionBarSpriteInstance,
		mySelectionBarSpriteData
	);

	if (mySelectionBarSpriteData.myTexture)
	{
		mySelectionBarSpriteInstance.myPosition =
			myButtonPositions[myButtonIndex] + Tga::Vector2f{ mySelectionBarXOffsets[myButtonIndex], 0.0f }; 
	}
}

StateUpdateResult QuitState::Update()
{
	for (int i = 0; i < static_cast<int>(ButtonType::Count); ++i)
	{
		if (myButtons[i].Update(i == myButtonIndex))
		{
			myButtonIndex = i;
		}
		if (myButtons[i].IsPressed())
		{
			const ButtonType buttonType = static_cast<ButtonType>(i);

			if (buttonType == ButtonType::Cancel)
			{
				myButtonIndex = 0;
				return StateUpdateResult::CreatePop();
			}

			return PressButton(buttonType);
		}
	}
	if (myInputMapper->IsActionJustActivated(GameAction::UIConfirm))
	{
		const ButtonType buttonType = static_cast<ButtonType>(myButtonIndex);

		if (buttonType == ButtonType::Cancel)
		{
			myButtonIndex = 0;
			AudioManager::GetAudioPoolByHandle(AudioHandles::uiBack).Play();
			return StateUpdateResult::CreatePop();
		}

		return PressButton(buttonType);
	}
	if (myInputMapper->IsActionJustActivated(GameAction::UICancel))
	{
		myButtonIndex = 0;
		return StateUpdateResult::CreatePop();
	}
	if (myInputMapper->IsActionJustActivated(GameAction::UIRight))
	{
		myButtonIndex = MathUtils::Modulo(++myButtonIndex, static_cast<int>(ButtonType::Count));
	}
	if (myInputMapper->IsActionJustActivated(GameAction::UILeft))
	{
		myButtonIndex = MathUtils::Modulo(--myButtonIndex, static_cast<int>(ButtonType::Count));
	}
	
	if (mySelectionBarSpriteData.myTexture) 
	{
		mySelectionBarSpriteInstance.myPosition =
			myButtonPositions[myButtonIndex] + Tga::Vector2f{ mySelectionBarXOffsets[myButtonIndex], 0.0f }; 
	}

	return StateUpdateResult::CreateContinue();
}

void QuitState::Render()
{
	const Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::SpriteDrawer& spriteDrawer = engine.GetGraphicsEngine().GetSpriteDrawer();

	spriteDrawer.Draw(myBackgroundSpriteData, myBackgroundSpriteInstance);

	if (mySelectionBarSpriteData.myTexture) 
	{
		spriteDrawer.Draw(mySelectionBarSpriteData, mySelectionBarSpriteInstance); 
	}

	for (auto& button : myButtons)
	{
		button.Render();
	}
}

void QuitState::OnGainFocus()
{
	SetMouseCaptureEnabled(false);
	AudioManager::GetAudioPoolByHandle(AudioHandles::windowChange).Play();
}

StateUpdateResult QuitState::PressButton(ButtonType aButtonType) const
{
	AudioManager::GetAudioPoolByHandle(AudioHandles::clickButton).Play();
	
	switch (aButtonType)
	{
		case ButtonType::Confirm:
		{
			PostQuitMessage(0);
			return StateUpdateResult::CreateContinue();
		}
		case ButtonType::Cancel:
		{
			return StateUpdateResult::CreatePop();
		}
		default:
		{
			std::cout << "[QuitState.cpp] Invalid button index\n";
			return StateUpdateResult::CreateContinue();
		}
	}
}
