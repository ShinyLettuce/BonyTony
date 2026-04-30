#pragma once

#include "State.h"
#include "Options.h"
#include "Button.h"
#include "MenuLayoutHelper.h"

#include <iostream>
#include <chrono>
#include <vector>

class InputManager;

class OptionsState : public State
{
public:
	void Init(InputMapper* aInputMapper, Timer* aTimer);

	StateUpdateResult Update() override;
	void Render() override;
	void OnResolutionChange() override { PositionElements(); }
	
	void PositionElements();
	
	void OnGainFocus() override;
	
	void ToggleDualStick();
	void UpdateDualStickText();

private:

	struct UIConfig
	{
		//Textures	
		const char* backgroundTexture = "textures/UI/Backgrounds/T_SetupBackground_C.png";

		const char* backTexture = "textures/UI/Buttons/T_ReturnButton_C.dds";
		const char* muteTexture = "textures/UI/Buttons/T_Box_C.dds";
		const char* fsTexture = "textures/UI/Buttons/T_Box_C.dds";

		const char* sliderBarTexture = "textures/UI/Buttons/T_Slider_C.dds";
		const char* sliderKnobTexture = "textures/UI/Buttons/T_SliderHandle_C.dds";

		const char* checkmarkTexture = "textures/UI/Buttons/T_Tick_C.dds";

		const char* muteOutlineTexture = "textures/UI/Buttons/T_BoxSelected_C.dds";
		const char* fsOutlineTexture = "textures/UI/Buttons/T_BoxSelected_C.dds";

		const char* masterSliderOutlineTexture = "textures/UI/Buttons/T_HandleSelected_C.dds";
		const char* musicSliderOutlineTexture = "textures/UI/Buttons/T_HandleSelected_C.dds";

		const char* returnSelectedTexture = "textures/UI/Buttons/T_ReturnSelected_C.dds";

		const char* windowedHeaderTexture = "textures/UI/Buttons/T_WindowedHeader_C.dds";
		const char* volumeHeaderTexture = "textures/UI/Buttons/T_VolumeHeader_C.dds";
		const char* muteHeaderTexture = "textures/UI/Buttons/T_MuteHeader_C.dds";

		const char* minVolTexture = "textures/UI/Buttons/T_MinVol_C.dds";
		const char* maxVolTexture = "textures/UI/Buttons/T_MaxVol_C.dds";

		//Multipliers
		float buttonSizeMultiplier = 0.75f;
		float boxSizeMultiplier = 0.75f;
		float sliderBarSizeMultiplier = 0.75f;
		float sliderKnobSizeMultiplier = 0.75f;
		float boxOutlineSizeMultiplier = 0.75f;
		float handleOutlineSizeMultiplier = 1.0f;
		float checkmarkSizeMultiplier = 0.75f;
		float headerSizeMultiplier = 0.75f;
		float volSizeMultiplier = 0.75f;

		//Positions (reference units from screen center)
		float backPositionX = 850.0f;
		float backPositionY = 450.0f;

		float mutePositionX = 260.0f;
		float mutePositionY = -75.0f;

		float fsPositionX = 365.0f;
		float fsPositionY = 150.0f;

		float sliderPositionX = 340.0f;
		float sliderPositionY = -400.0f;
		float sliderVerticalSpacing = 150.0f;

		float windowedHeaderPositionX = 570.0f;
		float windowedHeaderPositionY = 230.0f;

		float muteHeaderPositionX = 350.0f;
		float muteHeaderPositionY = 0.0f;

		float volumeHeaderPositionX = 290.0f;
		float volumeHeaderPositionY = -250.0f;

		float minVolPositionX = 130.0f;
		float minVolPositionY = -420.0f;

		float maxVolPositionX = 640.0f;
		float maxVolPositionY = -360.0f;

		// Offsets
		float returnBarOffsetX = 0.0f;
		float returnBarOffsetY = 0.0f;

		float checkmarkXOffset = 23.0f;
		float checkmarkYOffset = 9.0f;

		//Slider track padding in texture pixels (used for knob mapping)
		float sliderTrackPaddingLeftPx = 123.0f;
		float sliderTrackPaddingRightPx = 123.0f;
	};

	inline static UIConfig UI{};
	
	struct DualStick
	{
		Tga::Text text;
		float time;
		static constexpr float duration = 5.f;
		static constexpr float sizeMultiplier = 4.f;
	};
	DualStick myDualStick;

	InputMapper* myInputMapper = nullptr;
	Timer* myTimer = nullptr;

	Button myBackButton;
	Button myMuteButton;
	Button myFullscreenButton;

	Tga::Sprite2DInstanceData myBackgroundSpriteInstance;
	Tga::SpriteSharedData myBackgroundSpriteData;

	Tga::SpriteSharedData myMasterSliderBarData;
	Tga::Sprite2DInstanceData myMasterSliderBarInstance;
	Tga::SpriteSharedData myMasterSliderKnobData;
	Tga::Sprite2DInstanceData myMasterSliderKnobInstance;

	Tga::SpriteSharedData myMusicSliderBarData;
	Tga::Sprite2DInstanceData myMusicSliderBarInstance;
	Tga::SpriteSharedData myMusicSliderKnobData;
	Tga::Sprite2DInstanceData myMusicSliderKnobInstance;

	Tga::SpriteSharedData myMuteCheckmarkData;
	Tga::Sprite2DInstanceData myMuteCheckmarkInstance;

	Tga::SpriteSharedData myFSCheckmarkData;
	Tga::Sprite2DInstanceData myFSCheckmarkInstance;

	Tga::SpriteSharedData myMuteOutlineData;
	Tga::Sprite2DInstanceData myMuteOutlineInstance;

	Tga::SpriteSharedData myFSOutlineData;
	Tga::Sprite2DInstanceData myFSOutlineInstance;

	Tga::SpriteSharedData myMasterSliderOutlineData;
	Tga::Sprite2DInstanceData myMasterSliderOutlineInstance;

	Tga::SpriteSharedData myMusicSliderOutlineData;
	Tga::Sprite2DInstanceData myMusicSliderOutlineInstance;

	Tga::Sprite2DInstanceData myReturnBarSpriteInstance;
	Tga::SpriteSharedData myReturnBarSpriteData;

	Tga::Sprite2DInstanceData myWindowedSpriteInstance;
	Tga::SpriteSharedData myWindowedSpriteData;

	Tga::Sprite2DInstanceData myVolumeSpriteInstance;
	Tga::SpriteSharedData myVolumeSpriteData;

	Tga::Sprite2DInstanceData myMuteHeaderSpriteInstance;
	Tga::SpriteSharedData myMuteHeaderSpriteData;

	Tga::Sprite2DInstanceData myMinVolSpriteInstance;
	Tga::SpriteSharedData myMinVolSpriteData;

	Tga::Sprite2DInstanceData myMaxVolSpriteInstance;
	Tga::SpriteSharedData myMaxVolSpriteData;

	void UpdateSliderKnobPosition();
	void ChangeVolumeBy(int delta, int sliderIndex);
	void ApplyContinuousVolumeMove(float aUiDt, int aDir, float aUnitsPerSeconds, int aSliderIndex);

	float GetBarLeft(const Tga::Sprite2DInstanceData& aBarInstance) const;
	float GetBarRight(const Tga::Sprite2DInstanceData& aBarInstance) const;
	bool IsMouseOverBar(const Tga::Vector2f& aMousePos, const Tga::Sprite2DInstanceData& aBarInstance) const;
	void SetVolumeFromMouseX(float aMouseX, int sliderIndex);

	int mySelectedUI = 1;
	int myDraggingSlider = -1;

	bool isMuted = false;
	int myPrevMasterVolume = 100;
	int myPrevMusicVolume = 100;

	void ToggleMute();
	void ToggleFullscreen();

	bool myBackHover = false;
	bool myMuteHover = false;
	bool myFSHover = false;

	bool myMouseHoveringAnyUI = false;

	float mySliderRepeatTimer = 0.0f;
	float mySliderRepeatInterval = 0.03f;
	float mySliderRepeatDelay = 0.15f;
	int mySliderRepeatDirection = 0;

	std::chrono::steady_clock::time_point myUILastTick = {};
	bool myUIHasTicked = false;
	
	bool myMouseOverMasterPreviousFrame = false;
};