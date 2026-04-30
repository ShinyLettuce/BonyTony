#include "PauseState.h"
#include "QuitState.h"

#include <tge/Engine.h>
#include <tge/drawers/SpriteDrawer.h>
#include <tge/graphics/GraphicsEngine.h>
#include <tge/texture/TextureManager.h>
#include <tge/graphics/DX11.h>
#include <imgui/imgui.h>

#include "AudioManager.h"
#include "MathUtils.h"
#include "Go.h"

void PauseState::Init(InputMapper* aInputMapper, const PauseMenuStateHandles aStateHandles)
{
    myInputMapper = aInputMapper;
	myStateHandles = aStateHandles;

	ResolutionManager::Update();

	const auto& engine = *Tga::Engine::GetInstance();

	myBackgroundSpriteData.myTexture = engine.GetTextureManager().GetTexture(UI.backgroundTexture);
	myBackgroundSpriteInstance.myPivot = { 0.5f, 0.5f };

	SeedDefaultPolygons();

	PositionElements();
}

void PauseState::PositionElements()
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
	layoutConfig.buttonSizeMultiplier = UI.buttonSizeMultiplier;
	layoutConfig.baseButtonWidth = UI.baseButtonWidth;
	layoutConfig.baseButtonHeight = UI.baseButtonHeight;

	layoutConfig.hasSelectionBar = UI.buttonHasSelectionBar;
	layoutConfig.selectionBarTexture = UI.selectionBarTexture;
	layoutConfig.selectionBarUseTextureSize = UI.selectionBarUseTextureSize;
	layoutConfig.selectedBarSizeMultiplier = UI.selectionBarSizeMultiplier;
	layoutConfig.selectionBarPivot = UI.selectionBarPivot;

	std::array<ButtonLayoutConfig, static_cast<int>(ButtonType::Count)> buttonConfigs = { {
		{
			.texturePath = UI.resumeTexture,
			.induvidualXOffset = UI.resumeIndividualXOffset,
			.hitboxLeftOffset = UI.resumeHitboxLeftOffset,
			.hitboxRightOffset = UI.resumeHitboxRightOffset,
			.selectionBarXOffset = UI.resumeSelectionBarXOffset,
			.pivot = UI.buttonPivot,
			.rotationRadians = UI.resumeHoverRotationRadians,
			.hoverNormalMultiplier = UI.hoverNormalMultiplier,
			.hoverMultiplier = UI.hoverMultiplier,
		},
		{
			.texturePath = UI.mainMenuTexture,
			.induvidualXOffset = UI.mainMenuIndividualXOffset,
			.hitboxLeftOffset = UI.mainMenuHitboxLeftOffset,
			.hitboxRightOffset = UI.mainMenuHitboxRightOffset,
			.selectionBarXOffset = UI.mainMenuSelectionBarXOffset,
			.pivot = UI.buttonPivot,
			.rotationRadians = UI.mainMenuHoverRotationRadians,
			.hoverNormalMultiplier = UI.hoverNormalMultiplier,
			.hoverMultiplier = UI.hoverMultiplier,
		},
		{
			.texturePath = UI.optionsTexture,
			.induvidualXOffset = UI.optionsIndividualXOffset,
			.hitboxLeftOffset = UI.optionsHitboxLeftOffset,
			.hitboxRightOffset = UI.optionsHitboxRightOffset,
			.selectionBarXOffset = UI.optionsSelectionBarXOffset,
			.pivot = UI.buttonPivot,
			.rotationRadians = UI.optionsHoverRotationRadians,
			.hoverNormalMultiplier = UI.hoverNormalMultiplier,
			.hoverMultiplier = UI.hoverMultiplier,
		},
		{
			.texturePath = UI.quitTexture,
			.induvidualXOffset = UI.quitIndividualXOffset,
			.hitboxLeftOffset = UI.quitHitboxLeftOffset,
			.hitboxRightOffset = UI.quitHitboxRightOffset,
			.selectionBarXOffset = UI.quitSelectionBarXOffset,
			.pivot = UI.buttonPivot,
			.rotationRadians = UI.quitHoverRotationRadians,
			.hoverNormalMultiplier = UI.hoverNormalMultiplier,
			.hoverMultiplier = UI.hoverMultiplier,
		},
	} };

	MenuLayoutHelper::PositionButtons(
		layoutConfig,
		buttonConfigs,
		myButtons,
		myButtonPositions,
		center,
		myInputMapper
	);

	for (auto& btn : myButtons)
	{
		btn.SetUseSpriteSize(false);
	}

	MenuLayoutHelper::SetupSelectionBar(
		layoutConfig,
		buttonConfigs,
		mySelectionBarXOffsets,
		myBarSpriteInstance,
		myBarSpriteData
	);

	const float uiScale = ResolutionManager::GetUIScale();
	for (int i = 0; i < static_cast<int>(ButtonType::Count); ++i)
	{
		myPolyHitAreas[i].RebuildWorld(myButtonPositions[i], uiScale);
	}
}

StateUpdateResult PauseState::Update()
{
#if !defined(_RETAIL)
	const ImGuiIO& io = ImGui::GetIO();
	const bool allowMouseUI = !io.WantCaptureMouse;
	const bool allowKeyboardUI = !io.WantCaptureKeyboard;
#else
	const bool allowMouseUI = true;
	const bool allowKeyboardUI = true;
#endif

#ifndef _RETAIL
	if (ImGui::Begin("MainMenu Polygon HitAreas", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		int target = static_cast<int>(myPolyTarget);
		ImGui::Text("Target button:");
		ImGui::RadioButton("Resume", &target, static_cast<int>(ButtonType::Resume));
		ImGui::RadioButton("MainMenu", &target, static_cast<int>(ButtonType::MainMenu));
		ImGui::RadioButton("Options", &target, static_cast<int>(ButtonType::Options));
		ImGui::RadioButton("Quit", &target, static_cast<int>(ButtonType::Quit));
		myPolyTarget = static_cast<ButtonType>(target);

		ImGui::Separator();
		ImGui::Text("Recorder window is separate below.");
	}
	ImGui::End();

	{
		const int targetIndex = static_cast<int>(myPolyTarget);
		const float uiScale = ResolutionManager::GetUIScale();

		const Tga::Vector2f originWorld = myButtonPositions[targetIndex];

		const char* targetLabel = nullptr;
		switch (myPolyTarget)
		{
		case ButtonType::Resume:      targetLabel = "Resume"; break;
		case ButtonType::MainMenu:	  targetLabel = "MainMenu"; break;
		case ButtonType::Options:     targetLabel = "Options"; break;
		case ButtonType::Quit:        targetLabel = "Quit"; break;
		default:                      targetLabel = "Unknown"; break;
		}

		myPolyRecorder.DrawWindowAndHandleRecording(
			"Polygon Recorder (PauseMenu)",
			myInputMapper,
			allowMouseUI,
			originWorld,
			uiScale,
			[this, targetIndex](const std::vector<Tga::Vector2f>& offsetsRef)
			{
				myPolyHitAreas[targetIndex].offsetsRef = offsetsRef;
				PositionElements();
			},
			targetLabel
		);
	}
#endif

	if (allowMouseUI)
	{
		const Tga::Vector2f mousePos = myInputMapper->GetMousePositionYUp();
		const bool lmbJustPressed = myInputMapper->IsActionJustActivated(GameAction::UILeftClick);

		for (int i = 0; i < static_cast<int>(ButtonType::Count); ++i)
		{
			if (!myPolyHitAreas[i].HasValidPolygon())
				continue;

			if (myPolyHitAreas[i].ContainsPointConvex(mousePos))
			{
				myButtonIndex = i;

				if (myButtonsEnabled && lmbJustPressed)
				{
					const ButtonType type = static_cast<ButtonType>(i);
					if (type == ButtonType::Quit)
					{
						myRenderSelectionBar = false;
						QuitState::SetNextOpenContext(QuitState::OpenContext::Pause);
					}
					return PressButton(type);
				}
			}
		}
	}

	if (myInputMapper->IsActionJustActivated(GameAction::UICancel))
	{
		std::cout << "[PauseState.cpp] Close" << '\n';
		myButtonIndex = 0;
		return StateUpdateResult::CreatePop();
	}

	for (int i = 0; i < static_cast<int>(ButtonType::Count); ++i)
	{
		if (myButtons[i].Update(i == myButtonIndex))
		{
			myButtonIndex = i;
		}
		if (myButtonsEnabled && myButtons[i].IsPressed())
		{
			const ButtonType type = static_cast<ButtonType>(i);
			if (type == ButtonType::Quit)
			{
				myRenderSelectionBar = false;
				QuitState::SetNextOpenContext(QuitState::OpenContext::Pause);
			}
			return PressButton(type);
		}
	}

	if(allowKeyboardUI)
	{
		if (myButtonsEnabled && myInputMapper->IsActionJustActivated(GameAction::UIConfirm))
		{
			const ButtonType type = static_cast<ButtonType>(myButtonIndex);
			if (type == ButtonType::Quit)
			{
				myRenderSelectionBar = false;
				QuitState::SetNextOpenContext(QuitState::OpenContext::Pause);
			}
			return PressButton(type);
		}
		if (myInputMapper->IsActionJustActivated(GameAction::UIDown))
		{
			myButtonIndex = MathUtils::Modulo(++myButtonIndex, static_cast<int>(ButtonType::Count));
		}
		if (myInputMapper->IsActionJustActivated(GameAction::UIUp))
		{
			myButtonIndex = MathUtils::Modulo(--myButtonIndex, static_cast<int>(ButtonType::Count));
		}
	}

	myBarSpriteInstance.myPosition = 
		myButtonPositions[myButtonIndex] + Tga::Vector2f{ mySelectionBarXOffsets[myButtonIndex], 0.0f };
	
	return StateUpdateResult::CreateContinue();
}

void PauseState::Render()
{
	const Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::SpriteDrawer& spriteDrawer = engine.GetGraphicsEngine().GetSpriteDrawer();

	Tga::DX11::SetDepthEnabled(false);

	spriteDrawer.Draw(myBackgroundSpriteData, myBackgroundSpriteInstance);

	if (myRenderSelectionBar && myBarSpriteData.myTexture)
	{
		spriteDrawer.Draw(myBarSpriteData, myBarSpriteInstance);
	}

	for (auto& button : myButtons)
	{
		button.Render();
	}

#ifndef _RETAIL
	UI::DebugDrawPolygon(myPolyHitAreas[static_cast<int>(ButtonType::Resume)].world, Tga::Color(1, 0, 0, 1));
	UI::DebugDrawPolygon(myPolyHitAreas[static_cast<int>(ButtonType::MainMenu)].world, Tga::Color(0, 1, 0, 1));
	UI::DebugDrawPolygon(myPolyHitAreas[static_cast<int>(ButtonType::Options)].world, Tga::Color(0, 0, 1, 1));
	UI::DebugDrawPolygon(myPolyHitAreas[static_cast<int>(ButtonType::Quit)].world, Tga::Color(1, 1, 0, 1));

	UI::DebugDrawPolygon(myPolyRecorder.GetRecordedWorldPoints(), Tga::Color(1, 1, 1, 1));
#endif

	Tga::DX11::SetDepthEnabled(true);
}

void PauseState::SeedDefaultPolygons()
{
	myPolyHitAreas[static_cast<int>(ButtonType::Resume)].offsetsRef = UI.resumePolyOffsetsRef;
	myPolyHitAreas[static_cast<int>(ButtonType::MainMenu)].offsetsRef = UI.mainMenuPolyOffsetsRef;
	myPolyHitAreas[static_cast<int>(ButtonType::Options)].offsetsRef = UI.optionsPolyOffsetsRef;
	myPolyHitAreas[static_cast<int>(ButtonType::Quit)].offsetsRef = UI.quitPolyOffsetsRef;
}

void PauseState::OnGainFocus()
{
	SetMouseCaptureEnabled(false);
	myRenderSelectionBar = true;
	AudioManager::GetAudioPoolByHandle(AudioHandles::windowChange).Play();
}

StateUpdateResult PauseState::PressButton(ButtonType aButtonType) const
{
	AudioManager::GetAudioPoolByHandle(AudioHandles::clickButton).Play();
	
	switch (aButtonType)
	{
		case ButtonType::Resume:
		{
			std::cout << "[PauseState.cpp] Start\n";
			return StateUpdateResult::CreatePop();
		}
		case ButtonType::MainMenu:
		{
			std::cout << "[PauseState.cpp] MainMenu\n";
			return StateUpdateResult::CreateClearAndPush(myStateHandles.menuState);
		}
		case ButtonType::Options:
		{
			std::cout << "[PauseState.cpp] Options\n";
			return StateUpdateResult::CreatePush(myStateHandles.optionsState);
		}
		case ButtonType::Quit:
		{
			std::cout << "[PauseState.cpp] Quit\n";
			return StateUpdateResult::CreatePush(myStateHandles.quitState);
		}
		default:
		{
			std::cout << "[PauseState.cpp] Invalid button index\n";
			return StateUpdateResult::CreateContinue();
		}
	}
}
