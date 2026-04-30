#pragma once
#include "InputMapper.h"
#include "State.h"

#include "tge/sprite/sprite.h"
#include "Button.h"
#include <array>

class InputMapper;

class CreditsState : public State
{
public:
	void Init(InputMapper* aInputMapper);

	void PositionElements();

	StateUpdateResult Update() override;
	void Render() override;
	void OnResolutionChange() override { PositionElements(); }
	
	void OnGainFocus() override;

private:
	//=====================================================================================
	// vv EDIT UI HERE!!! vv
	//=====================================================================================
	struct UIConfig
	{
		//Textures
		const char* backgroundTexture = "textures/UI/Backgrounds/T_CreditsBackground_C.png";
		const char* returnButtonTexture = "textures/UI/Buttons/T_ReturnButton_C.dds";		
		const char* selectionBarTexture = "textures/UI/Buttons/T_ReturnSelected_C.dds";

		//Button Positioning
		float baseButtonOffsetX = 850.0f;
		float baseButtonOffsetY = 450.0f;
		float baseButtonSpacingY = 0.0f;//spacing is unused in credits
		float baseButtonSpacingX = 0.0f;//------------||-------------

		//configs or smt
		bool buttonUseTextureSize = true;
		float buttonSizeMultiplier = 0.75f;		
		bool buttonHasSelectionBar = true;
		bool selectionBarUseTextureSize = true;
		float selectionBarSizeMultiplier = 0.75f;
		
		//Hover visuals
		float returnHoverNormalMultiplier = 1.0f;
		float returnHoverMultiplier = 1.08f;
		float returnHoverRotationRadians = 0.0f;
		float returnSelectionHoverRotationRadians = 0.0f;
		float returnSelectionHoverNormalMultiplier = 1.0f;
		float returnSelectionHoverMultiplier = 1.08f;

		//Fine-tuning button + selectionbar
		float returnInduvidualXOffset = 0.0f;//unused in credits
		float returnHitboxLeftOffset = 0.0f;
		float returnHitboxRightOffset = 0.0f;
		float returnSelectionBarXOffset = 0.0f;//unused in credits
		Tga::Vector2f returnPivot = { 0.5f, 0.5f };
		Tga::Vector2f selectionBarPivot = { 0.5f, 0.5f };
	};

	inline static UIConfig UI{};

	//=====================================================================================

	InputMapper* myInputMapper = nullptr;

	Tga::Sprite2DInstanceData mySpriteInstance;
	Tga::SpriteSharedData mySpriteData;

	Tga::Sprite2DInstanceData myBarSpriteInstance;
	Tga::SpriteSharedData myBarSpriteData;

	Button myBackButton;
	std::array<Tga::Vector2f, 1> myButtonPositions{};
	std::array<float, 1> mySelectionBarXOffsets{};

	int myButtonIndex = 0;

	bool myIsBackButtonHovered = false;
};
