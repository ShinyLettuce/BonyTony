#pragma once
#include "ResolutionManager.h"
#include "MenuLayoutHelper.h"
#include "InputMapper.h"
#include "State.h"
#include "UIPolygonTools.h"

#include "tge/sprite/sprite.h"
#include "Button.h"
#include <array>
#include <vector>

class OptionsState;

struct PauseMenuStateHandles
{
	StateHandle menuState;
	StateHandle optionsState;
	StateHandle quitState;
	OptionsState* optionsStateptr = nullptr;
};

class PauseState : public State
{
public:
	void Init(InputMapper* aInputMapper, const PauseMenuStateHandles aStateHandles);

	void PositionElements();

	StateUpdateResult Update() override;
	void Render() override;
	void OnResolutionChange() override { PositionElements(); }
	void SeedDefaultPolygons();
	
	void OnGainFocus() override;
	
private:
	enum class ButtonType
	{
		Resume,
		MainMenu,
		Options,
		Quit,
		Count,
	};

	struct UIConfig
	{
		//Textures
		const char* backgroundTexture = "textures/UI/Backgrounds/T_PauseBackground_C.png";
		const char* selectionBarTexture = "textures/UI/Buttons/T_Selection_C.dds";
		const char* resumeTexture = "textures/UI/Buttons/T_ResumeButton_C.dds";
		const char* mainMenuTexture = "textures/UI/Buttons/T_MenuButton_C.dds";
		const char* optionsTexture = "textures/UI/Buttons/T_SetupButton_C.dds";
		const char* quitTexture = "textures/UI/Buttons/T_ExitButton_C.dds";

		bool backgroundUseCoverScale = true;
		bool buttonUseTextureSize = true;
		float buttonSizeMultiplier = 0.75f;
		bool buttonHasSelectionBar = true;
		bool selectionBarUseTextureSize = true;
		float selectionBarSizeMultiplier = 0.75f;

		float baseButtonOffsetX = 475.0f;//This is the top button position and all other buttons get positioned with 
		float baseButtonOffsetY = 90.0f;//x and y-axis spacing then the position is fine-tuned with the induvidual button offsets.
		float baseButtonSpacingY = 160.0f;
		float baseButtonSpacingX = 0.0f;//x is currently 0 because it was easier to position the buttons on the x-axis individualy.

		float baseButtonWidth = 170.0f;//only is in use if buttonUseTextureSize = false
		float baseButtonHeight = 100.0f;//       -------------||--------------

		//Button + selection bar fine-tuning
		float resumeIndividualXOffset = 0.0f;
		float resumeHitboxLeftOffset = -5.0f;
		float resumeHitboxRightOffset = 900.0f;
		float resumeSelectionBarXOffset = 180.0f;

		float mainMenuIndividualXOffset = 10.0f;
		float mainMenuHitboxLeftOffset = 50.0f;
		float mainMenuHitboxRightOffset = 900.0f;
		float mainMenuSelectionBarXOffset = 80.0f;

		float optionsIndividualXOffset = -205.0f;
		float optionsHitboxLeftOffset = -20.0f;
		float optionsHitboxRightOffset = 900.0f;
		float optionsSelectionBarXOffset = 230.0f;

		float quitIndividualXOffset = -320.0f;
		float quitHitboxLeftOffset = 20.0f;
		float quitHitboxRightOffset = 900.0f;
		float quitSelectionBarXOffset = 265.0f;

		Tga::Vector2f buttonPivot = { 0.5f, 0.5f };
		Tga::Vector2f selectionBarPivot = { 0.5f, 0.5f };

		//Hover visuals
		float resumeHoverRotationRadians = 0.0f;
		float mainMenuHoverRotationRadians = 0.0f;
		float optionsHoverRotationRadians = 0.0f;
		float quitHoverRotationRadians = 0.0f;

		float hoverNormalMultiplier = 1.0f;
		float hoverMultiplier = 1.08f;

		std::vector<Tga::Vector2f> resumePolyOffsetsRef = {
			{ 483.8f, 99.6f },
			{ -202.6f, 96.0f },
			{ -284.2f, -63.6f },
			{ 483.8f, -58.8f },
		};

		std::vector<Tga::Vector2f> mainMenuPolyOffsetsRef = {
			{ 473.8f, 97.6f },
			{ -299.0f, 94.0f },
			{ -373.4f, -62.0f },
			{ 473.8f, -59.6f },
		};

		std::vector<Tga::Vector2f> optionsPolyOffsetsRef = {
			{ 688.8f, 98.0f },
			{ -162.0f, 95.6f },
			{ -237.6f, -61.6f },
			{ 688.8f, -62.8f },
		};

		std::vector<Tga::Vector2f> quitPolyOffsetsRef = {
			{ 803.8f, 94.8f },
			{ -125.0f, 98.4f },
			{ -225.8f, -98.4f },
			{ 803.8f, -100.8f },
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
	ButtonType myPolyTarget = ButtonType::Resume;
#endif

	PauseMenuStateHandles myStateHandles{};
	
	InputMapper* myInputMapper{};
	
	int myButtonIndex = 0;

	bool myButtonsEnabled = true;
	bool myRenderSelectionBar = true;
};
