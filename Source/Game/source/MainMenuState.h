#pragma once

#include <array>
#include <vector>

#include "State.h"
#include "Button.h"
#include "AudioManager.h"
#include "UIPolygonTools.h"

class InputMapper;

struct MainMenuStateHandles
{
	StateHandle gameState;
	StateHandle levelSelectState;
	StateHandle optionsState;
	StateHandle cutsceneState;
	StateHandle creditsState;
	StateHandle quitState;
};

class MainMenuState : public State
{
public:
	void Init(MainMenuStateHandles aStateHandles, InputMapper* aInputMapper);

	void PositionElements();

	void OnPush() override;
	void OnPop() override;

	StateUpdateResult Update() override;
	void Render() override;
	void OnResolutionChange() override { PositionElements(); }
	void SeedDefaultPolygons();
	
	void OnGainFocus() override;

private:
	enum class ButtonType
	{
		Start,
		LevelSelect,
		Options,
		Credits,
		Quit,
		Count,
	};

	struct UIConfig
	{
		//Textures
		const char* backgroundTexture = "textures/UI/Backgrounds/T_MainBackground_C.png";
		const char* selectionBarTexture = "textures/UI/Buttons/T_Selection_C.dds";	
		const char* startTexture = "textures/UI/Buttons/T_PlayButton_C.dds";
		const char* chaptersTexture = "textures/UI/Buttons/T_ChaptersButton_C.dds";
		const char* optionsTexture = "textures/UI/Buttons/T_SetupButton_C.dds";
		const char* creditsTexture = "textures/UI/Buttons/T_Credits_C.dds";
		const char* quitTexture = "textures/UI/Buttons/T_ExitButton_C.dds";
		
		bool backgroundUseCoverScale = true;
		bool buttonUseTextureSize = true;
		float buttonSizeMultiplier = 0.75f;
		bool buttonHasSelectionBar = true;
		bool selectionBarUseTextureSize = true;
		float selectionBarSizeMultiplier = 0.75f;

		float baseButtonOffsetX = 635.0f;//This is the top button position and all other buttons get positioned with 
		float baseButtonOffsetY = 250.0f;//x and y-axis spacing then the position is fine-tuned with the induvidual button offsets.
		float baseButtonSpacingY = 160.0f;
		float baseButtonSpacingX = 0.0f;//x is currently 0 because it was easier to position the buttons on the x-axis individualy.

		float baseButtonWidth = 170.0f;//only is in use if buttonUseTextureSize = false
		float baseButtonHeight = 100.0f;//       -------------||--------------

		//Button + selection bar fine-tuning
		float startIndividualXOffset = -150.0f;
		float startHitboxLeftOffset = 15.0f;
		float startHitboxRightOffset = 900.0f;
		float startSelectionBarXOffset = 250.0f;

		float chaptersIndividualXOffset = -115.0f;
		float chaptersHitboxLeftOffset = 90.0f;
		float chaptersHitboxRightOffset = 900.0f;
		float chaptersSelectionBarXOffset = 120.0f;

		float optionsIndividualXOffset = -285.0f;
		float optionsHitboxLeftOffset = -20.0f;
		float optionsHitboxRightOffset = 900.0f;
		float optionsSelectionBarXOffset = 210.0f;

		float creditsIndividualXOffset = -310.0f;
		float creditsHitboxLeftOffset = 150.0f;
		float creditsHitboxRightOffset = 900.0f;
		float creditsSelectionBarXOffset = 160.0f;

		float quitIndividualXOffset = -475.0f;
		float quitHitboxLeftOffset = 20.0f;
		float quitHitboxRightOffset = 900.0f;
		float quitSelectionBarXOffset = 250.0f;

		Tga::Vector2f buttonPivot = { 0.5f, 0.5f };
		Tga::Vector2f selectionBarPivot = { 0.5f, 0.5f };

		//Hover visuals
		float startHoverRotationRadians = 0.0f;
		float chaptersHoverRotationRadians = 0.0f;
		float optionsHoverRotationRadians = 0.0f;
		float creditsHoverRotationRadians = 0.0f;
		float quitHoverRotationRadians = 0.0f;

		float hoverNormalMultiplier = 1.0f;
		float hoverMultiplier = 1.08f;

		std::vector<Tga::Vector2f> startPolyOffsetsRef = {
			{ 473.8f, 101.6f },
			{ -133.4f, 99.2f },
			{ -216.2f, -65.2f },
			{ 473.8f, -59.2f },
		};

		std::vector<Tga::Vector2f> chaptersPolyOffsetsRef = {
			{ 438.8f, 99.6f },
			{ -252.4f, 92.4f },
			{ -330.4f, -62.4f },
			{ 438.8f, -58.8f },
		};

		std::vector<Tga::Vector2f> optionsPolyOffsetsRef = {
			{ -160.4f, 98.8f },
			{ 608.8f, 102.4f },
			{ 608.8f, -58.4f },
			{ -240.8f, -64.4f },
		};

		std::vector<Tga::Vector2f> creditsPolyOffsetsRef = {
			{ -217.0f, 98.0f },
			{ 633.8f, 101.6f },
			{ 633.8f, -55.6f },
			{ -293.8f, -61.6f },
		};

		std::vector<Tga::Vector2f> quitPolyOffsetsRef = {
			{ 798.8f, 102.0f },
			{ -130.0f, 98.4f },
			{ -232.0f, -98.4f },
			{ 798.8f, -98.4f },
		};
	};

	inline static UIConfig UI{};
	
	StateUpdateResult PressButton(ButtonType aButtonType) const;
	
	Tga::Sprite2DInstanceData myBackgroundSpriteInstance;
	Tga::SpriteSharedData myBackgroundSpriteData;

	Tga::Sprite2DInstanceData myBarSpriteInstance;
	Tga::SpriteSharedData myBarSpriteData;
	
	std::array<Button, static_cast<int>(ButtonType::Count)> myButtons{};
	std::array<Tga::Vector2f, static_cast<int>(ButtonType::Count)> myButtonPositions{};
	std::array<float, static_cast<int>(ButtonType::Count)> mySelectionBarXOffsets{};

	std::array<UI::PolygonHitArea, static_cast<int>(ButtonType::Count)> myPolyHitAreas{};

#ifndef _RETAIL
	UI::PolygonRecorder myPolyRecorder;
	ButtonType myPolyTarget = ButtonType::Start;
#endif

	MainMenuStateHandles myStateHandles;
	
	InputMapper* myInputMapper = nullptr;
	
	int myButtonIndex = 0;
	
	AudioManager::AudioPoolHandle myAudioPoolHandle;

	bool myButtonsEnabled = true;

	bool myRenderSelectionBar = true;
};
