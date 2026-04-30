#include "MainMenuState.h"
#include "InputMapper.h"
#include "MathUtils.h"
#include "MenuLayoutHelper.h"
#include "QuitState.h"

#include <tge/Engine.h>
#include <tge/drawers/SpriteDrawer.h>
#include <tge/graphics/GraphicsEngine.h>
#include <tge/texture/TextureManager.h>

#include <imgui/imgui.h>

#include "Cutscene.h"
#include "SceneLoader.h"
#include "ResolutionManager.h"
#include "Options.h"
#include "Go.h"

void MainMenuState::Init(MainMenuStateHandles aStateHandles, InputMapper* aInputMapper)
{
	myInputMapper = aInputMapper;
	myStateHandles = aStateHandles;

	ResolutionManager::Update();

	const auto& engine = *Tga::Engine::GetInstance();

	myBackgroundSpriteData.myTexture = engine.GetTextureManager().GetTexture(UI.backgroundTexture);
	myBackgroundSpriteInstance.myPivot = { 0.5f, 0.5f };

	SeedDefaultPolygons();

	PositionElements();

	//myAudioPoolHandle= AudioManager::MakeAudioPool();
	//AudioManager::AudioPool &testAudioPool = AudioManager::GetAudioPoolByHandle(myAudioPoolHandle);

	/*testAudioPool.AddClip("Audio/test-sfx.mp3");*/
#ifdef _RETAIL
	Options::fullscreen = true;
	Tga::Engine::GetInstance()->SetFullScreen(true);
#endif
}

void MainMenuState::PositionElements()
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
			.texturePath = UI.startTexture,
			.induvidualXOffset = UI.startIndividualXOffset,
			.hitboxLeftOffset = UI.startHitboxLeftOffset,
			.hitboxRightOffset = UI.startHitboxRightOffset,
			.selectionBarXOffset = UI.startSelectionBarXOffset,
			.pivot = UI.buttonPivot,
			.rotationRadians = UI.startHoverRotationRadians,
			.hoverNormalMultiplier = UI.hoverNormalMultiplier,
			.hoverMultiplier = UI.hoverMultiplier,
		},
		{
			.texturePath = UI.chaptersTexture,
			.induvidualXOffset = UI.chaptersIndividualXOffset,
			.hitboxLeftOffset = UI.chaptersHitboxLeftOffset,
			.hitboxRightOffset = UI.chaptersHitboxRightOffset,
			.selectionBarXOffset = UI.chaptersSelectionBarXOffset,
			.pivot = UI.buttonPivot,
			.rotationRadians = UI.chaptersHoverRotationRadians,
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
			.texturePath = UI.creditsTexture,
			.induvidualXOffset = UI.creditsIndividualXOffset,
			.hitboxLeftOffset = UI.creditsHitboxLeftOffset,
			.hitboxRightOffset = UI.creditsHitboxRightOffset,
			.selectionBarXOffset = UI.creditsSelectionBarXOffset,
			.pivot = UI.buttonPivot,
			.rotationRadians = UI.creditsHoverRotationRadians,
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

void MainMenuState::OnPush()
{
	AudioManager::GetAudioPoolByHandle(AudioHandles::mainMenuMusic).Play();
}

void MainMenuState::OnPop()
{
	AudioManager::GetAudioPoolByHandle(AudioHandles::mainMenuMusic).Stop();
}

StateUpdateResult MainMenuState::Update()
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
		ImGui::RadioButton("Start", &target, static_cast<int>(ButtonType::Start));
		ImGui::RadioButton("Chapters", &target, static_cast<int>(ButtonType::LevelSelect));
		ImGui::RadioButton("Options", &target, static_cast<int>(ButtonType::Options));
		ImGui::RadioButton("Credits", &target, static_cast<int>(ButtonType::Credits));
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
		case ButtonType::Start:       targetLabel = "Start"; break;
		case ButtonType::LevelSelect: targetLabel = "Chapters"; break;
		case ButtonType::Options:     targetLabel = "Options"; break;
		case ButtonType::Credits:     targetLabel = "Credits"; break;
		case ButtonType::Quit:        targetLabel = "Quit"; break;
		default:                      targetLabel = "Unknown"; break;
		}

		myPolyRecorder.DrawWindowAndHandleRecording(
			"Polygon Recorder (MainMenu)",
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
						QuitState::SetNextOpenContext(QuitState::OpenContext::MainMenu);
					}
					return PressButton(type);
				}
			}
		}
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
				QuitState::SetNextOpenContext(QuitState::OpenContext::MainMenu);
			}
			return PressButton(type);
		}
	}

	if (allowKeyboardUI)
	{
		if (myButtonsEnabled && myInputMapper->IsActionJustActivated(GameAction::UIConfirm))
		{
			const ButtonType type = static_cast<ButtonType>(myButtonIndex);
			if (type == ButtonType::Quit)
			{
				myRenderSelectionBar = false; 
				QuitState::SetNextOpenContext(QuitState::OpenContext::MainMenu);
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

void MainMenuState::Render()
{
	const Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::SpriteDrawer& spriteDrawer = engine.GetGraphicsEngine().GetSpriteDrawer();
	
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
	UI::DebugDrawPolygon(myPolyHitAreas[static_cast<int>(ButtonType::Start)].world, Tga::Color(1, 0, 0, 1));
	UI::DebugDrawPolygon(myPolyHitAreas[static_cast<int>(ButtonType::LevelSelect)].world, Tga::Color(0, 1, 0, 1));
	UI::DebugDrawPolygon(myPolyHitAreas[static_cast<int>(ButtonType::Options)].world, Tga::Color(0, 0, 1, 1));
	UI::DebugDrawPolygon(myPolyHitAreas[static_cast<int>(ButtonType::Credits)].world, Tga::Color(1, 0, 1, 1));
	UI::DebugDrawPolygon(myPolyHitAreas[static_cast<int>(ButtonType::Quit)].world, Tga::Color(1, 1, 0, 1));

	UI::DebugDrawPolygon(myPolyRecorder.GetRecordedWorldPoints(), Tga::Color(1, 1, 1, 1));
#endif
}

void MainMenuState::SeedDefaultPolygons()
{
	myPolyHitAreas[static_cast<int>(ButtonType::Start)].offsetsRef = UI.startPolyOffsetsRef;
	myPolyHitAreas[static_cast<int>(ButtonType::LevelSelect)].offsetsRef = UI.chaptersPolyOffsetsRef;
	myPolyHitAreas[static_cast<int>(ButtonType::Options)].offsetsRef = UI.optionsPolyOffsetsRef;
	myPolyHitAreas[static_cast<int>(ButtonType::Credits)].offsetsRef = UI.creditsPolyOffsetsRef;
	myPolyHitAreas[static_cast<int>(ButtonType::Quit)].offsetsRef = UI.quitPolyOffsetsRef;
}

void MainMenuState::OnGainFocus()
{
	SetMouseCaptureEnabled(false);
	myRenderSelectionBar = true;
	AudioManager::GetAudioPoolByHandle(AudioHandles::windowChange).Play();
}

StateUpdateResult MainMenuState::PressButton(const ButtonType aButtonType) const
{
	AudioManager::GetAudioPoolByHandle(AudioHandles::clickButton).Play();
	
	switch (aButtonType)
	{
		case ButtonType::Start:
		{
			std::cout << "[MainMenuState.cpp] Start\n";

			Cutscene::PreparedConfig preparedConfig
			{
				.filePath = "videos/Bony_Tony_CUTSCENE_NO SOUND.mp4",
				.audioHandle = AudioHandles::introCutscene,
				.nextStateHandle = myStateHandles.gameState,
			};

			Cutscene::Set(preparedConfig);

			return StateUpdateResult::CreateClearAndPush(myStateHandles.cutsceneState);
		}
		case ButtonType::LevelSelect:
		{
			std::cout << "[MainMenuState.cpp] LevelSelect\n";
			return StateUpdateResult::CreatePush(myStateHandles.levelSelectState);
		}
		case ButtonType::Options:
		{
			std::cout << "[MainMenuState.cpp] Options\n";
			return StateUpdateResult::CreatePush(myStateHandles.optionsState);
		}
		case ButtonType::Credits:
		{
			std::cout << "[MainMenuState.cpp] Credits\n";
			return StateUpdateResult::CreatePush(myStateHandles.creditsState);
		}
		case ButtonType::Quit:
		{
			std::cout << "[MainMenuState.cpp] Quit\n";
			return StateUpdateResult::CreatePush(myStateHandles.quitState);
		}
		default:
		{
			std::cout << "[MainMenuState.cpp] Invalid button index\n";
			return StateUpdateResult::CreateContinue();
		}
	}
}
