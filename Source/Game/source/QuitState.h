#pragma once
#include "InputMapper.h"
#include "State.h"
#include "Button.h"
#include "MenuLayoutHelper.h"

#include <array>
#include <vector>

class QuitState : public State
{
public:
	enum class OpenContext
	{
		MainMenu,
		Pause,
	};

	void Init(InputMapper* aInputMapper, OpenContext aContext = OpenContext::MainMenu);

	static void SetNextOpenContext(OpenContext aContext) { myNextOpenContext = aContext; }

	void OnPush() override;

	void PositionElements();

	StateUpdateResult Update() override;
	void Render() override;
	void OnResolutionChange() override { PositionElements(); }

	void OnGainFocus() override;

private:
	enum class ButtonType
	{
		Confirm,
		Cancel,
		Count,
	};

	//=====================================================================================
	// vv EDIT UI HERE!!! vv
	//=====================================================================================
	struct UIConfig
	{
		//Textures
		const char* mainMenubackgroundTexture = "textures/UI/Backgrounds/T_MainMenuConfirmBackground_C.png";               
		const char* pausebackgroundTexture = "textures/UI/Backgrounds/T_PauseConfirmBackground_C.png";               
		const char* yesButtonTexture = "textures/UI/Buttons/T_YesButton_C.dds";   
		const char* noButtonTexture = "textures/UI/Buttons/T_NoButton_C.dds";    

		//Selection textures
		const char* selectionBarTexture = "textures/UI/Buttons/T_ConfirmSelected_C.dds";

		bool backgroundUseCoverScale = true;                                

		float baseButtonOffsetX = 520.0f;
		float baseButtonOffsetY = -380.0f;
		float baseButtonSpacingY = 0.0f;
		float baseButtonSpacingX = 250.0f;

		//Button sizing
		bool buttonUseTextureSize = true;                                   
		float buttonSizeMultiplier = 0.75f;        
		bool buttonHasSelectionBar = true;
		bool selectionBarUseTextureSize = true;
		float selectionBarSizeMultiplier = 0.80f;

		float baseButtonWidth = 170.0f;//only is in use if buttonUseTextureSize = false
		float baseButtonHeight = 100.0f;//       -------------||--------------

		//Hover visuals
		float yesButtonHoverNormalMultiplier = 1.0f;
		float yesButtonHoverMultiplier = 1.08f;
		float yesButtonHoverRotationRadians = 0.0f;

		float noButtonHoverNormalMultiplier = 1.0f;
		float noButtonHoverMultiplier = 1.08f;
		float noButtonHoverRotationRadians = 0.0f;

		float confirmSelectionHoverRotationRadians = 0.0f;
		float confirmSelectionHoverNormalMultiplier = 1.0f;
		float confirmSelectionHoverMultiplier = 1.08f;

		//Fine tuning
		float yesButtonInduvidualXOffset = 0.0f;
		float yesButtonHitboxLeftOffset = 0.0f;
		float yesButtonHitboxRightOffset = 0.0f;
		Tga::Vector2f yesButtonPivot = { 0.5f, 0.5f };

		float noButtonInduvidualXOffset = 0.0f;
		float noButtonHitboxLeftOffset = 0.0f;
		float noButtonHitboxRightOffset = 0.0f;
		Tga::Vector2f noButtonPivot = { 0.5f, 0.5f };

		float yesButtonSelectionBarXOffset = 0.0f;
		float noButtonSelectionBarXOffset = 0.0f;
		Tga::Vector2f selectionBarPivot = { 0.5f, 0.5f };
	};

	inline static UIConfig UI{}; 

	//=====================================================================================

	inline static OpenContext myNextOpenContext = OpenContext::MainMenu;

	OpenContext myOpenContext = OpenContext::MainMenu;

	Tga::Sprite2DInstanceData myBackgroundSpriteInstance;
	Tga::SpriteSharedData myBackgroundSpriteData;

	Tga::Sprite2DInstanceData mySelectionBarSpriteInstance;
	Tga::SpriteSharedData mySelectionBarSpriteData;
	std::array<float, static_cast<int>(ButtonType::Count)> mySelectionBarXOffsets{};

	StateUpdateResult PressButton(ButtonType aButtonType) const;

	std::array<Button, static_cast<int>(ButtonType::Count)> myButtons{};
	std::array<Tga::Vector2f, static_cast<int>(ButtonType::Count)> myButtonPositions{};
	std::array<ButtonLayoutConfig, static_cast<int>(ButtonType::Count)> myButtonConfigs{}; 

	InputMapper* myInputMapper = nullptr;

	int myButtonIndex = 0;
	bool myMouseHoveringAnyUI = false; 
};