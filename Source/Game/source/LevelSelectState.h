#pragma once
#include "InputMapper.h"
#include "State.h"
#include "Button.h"
#include "AudioManager.h"
#include "MenuLayoutHelper.h"

#include <vector>
#include <array>

class LevelSelectState : public State
{
public:
	void Init(const StateHandle aGameStateHandle, const StateHandle aBossRoomStateHandle, InputMapper* aInputMapper);

	void PositionElements();

	StateUpdateResult Update() override;
	void Render() override;
	void OnResolutionChange() override { PositionElements(); }
	
	void OnGainFocus() override;

private:
	enum class ButtonType
	{
		Return,
		Act1,
		Act2,
		Count,
	};

	enum class DebugPolyTarget
	{
		Act1,
		Act2,
	};

	enum class OverlayVariant
	{
		Shared,
		MissingAct1,
		MissingAct2,
	};

	StateUpdateResult PressButton(ButtonType aButtonType) const;

	static float Cross2D(const Tga::Vector2f& a, const Tga::Vector2f& b);
	static bool PointInConvexPolygon(const Tga::Vector2f& p, const std::vector<Tga::Vector2f>& verts);

	void DebugDrawPolygon(const std::vector<Tga::Vector2f>& poly, const Tga::Color& color) const;
	void DebugHandlePolygonRecording();
	void ResetUISelection();

	//=====================================================================================
	// vv EDIT UI HERE!!! vv
	//=====================================================================================
	struct UIConfig
	{
		//Background / overlay textures
		const char* backgroundTexture = "textures/UI/Backgrounds/T_ChaptersBackground_C.png";
		const char* overlaySharedTexture = "textures/UI/Buttons/T_BothActUnselected_C.dds";
		const char* overlayMissingAct1Texture = "textures/UI/Buttons/T_Act1Unselected_C.dds";
		const char* overlayMissingAct2Texture = "textures/UI/Buttons/T_Act2Unselected_C.dds";

		//Button textures
		const char* act1Texture = "textures/UI/Buttons/T_Act1Button_C.dds";
		const char* act2Texture = "textures/UI/Buttons/T_Act2Button_C.dds";
		const char* returnTexture = "textures/UI/Buttons/T_ReturnButton_C.dds";

		//Selection textures
		const char* act1SelectionTexture = "textures/UI/Buttons/T_Act1Selected_C.dds";
		const char* act2SelectionTexture = "textures/UI/Buttons/T_Act2Selected_C.dds";
		const char* returnSelectionTexture = "textures/UI/Buttons/T_ReturnSelected_C.dds";

		//Sizing
		bool backgroundUseCoverScale = true;
		bool buttonUseTextureSize = true;
		float buttonSizeMultiplier = 0.75f;
		bool selectionUseTextureSize = true;
		float selectionSizeMultiplier = 0.75f;

		float baseButtonWidth = 170.0f;//only is in use if buttonUseTextureSize = false
		float baseButtonHeight = 100.0f;//       -------------||--------------

		//Button positions relative to screen center
		Tga::Vector2f act1ButtonOffsetRef = { -700.0f, -240.0f };
		Tga::Vector2f act2ButtonOffsetRef = { 680.0f,  200.0f };
		Tga::Vector2f returnButtonOffsetRef = { 850.0f,  450.0f };

		//Selection offsets from their button centers
		Tga::Vector2f act1SelectionOffsetRef = { -20.0f, -20.0f };
		Tga::Vector2f act2SelectionOffsetRef = { 20.0f,  20.0f };
		Tga::Vector2f returnSelectionOffsetRef = { 0.0f, 0.0f };

		//Pivots
		Tga::Vector2f act1Pivot = { 0.5f, 0.5f };
		Tga::Vector2f act2Pivot = { 0.5f, 0.5f };
		Tga::Vector2f returnPivot = { 0.5f, 0.5f };

		//Hitbox offsets
		float act1HitboxLeftOffset = 0.0f;
		float act1HitboxRightOffset = 0.0f;
		float act2HitboxLeftOffset = 0.0f;
		float act2HitboxRightOffset = 0.0f;
		float returnHitboxLeftOffset = 0.0f;
		float returnHitboxRightOffset = 0.0f;

		// Hover visual: buttons
		float act1HoverRotationRadians = 25.0f;
		float act2HoverRotationRadians = 25.0f;
		float returnHoverRotationRadians = 0.0f;

		float act1HoverNormalMultiplier = 1.0f;
		float act1HoverMultiplier = 1.08f;

		float act2HoverNormalMultiplier = 1.0f;
		float act2HoverMultiplier = 1.08f;

		float returnHoverNormalMultiplier = 1.0f;
		float returnHoverMultiplier = 1.08f;

		//Hover visuals: selection bars
		float act1SelectionHoverRotationRadians = 25.0f;
		float act2SelectionHoverRotationRadians = 25.0f;
		float returnSelectionHoverRotationRadians = 0.0f;

		float act1SelectionHoverNormalMultiplier = 1.0f;
		float act1SelectionHoverMultiplier = 1.08f;

		float act2SelectionHoverNormalMultiplier = 1.0f;
		float act2SelectionHoverMultiplier = 1.08f;

		float returnSelectionHoverNormalMultiplier = 1.0f;
		float returnSelectionHoverMultiplier = 1.0f;

		//Preview polygon area positioning (reference-unit offsets from center)
		std::vector<Tga::Vector2f> act1PreviewHitPolyOffsetsRef = {
			{ 97.2f, 345.6f },
			{ -643.2f, 489.6f },
			{ -808.8f, 337.2f },
			{ -812.4f, -264.0f },
			{ -414.0f, -262.8f },
		};

		std::vector<Tga::Vector2f> act2PreviewHitPolyOffsetsRef = {
			{ 214.8f, 321.6f },
			{ 798.6f, 211.2f },
			{ 764.4f, -345.6f },
			{ -357.6f, -343.2f },
		};
	};

	inline static UIConfig UI{};

	//=====================================================================================

	std::vector<Tga::Vector2f> myAct1PreviewHitPolyOffsetsRef;
	std::vector<Tga::Vector2f> myAct2PreviewHitPolyOffsetsRef;

	std::vector<Tga::Vector2f> myAct1PreviewHitPoly;
	std::vector<Tga::Vector2f> myAct2PreviewHitPoly;

	bool myDebugRecordPoly = false;
	DebugPolyTarget myDebugPolyTarget = DebugPolyTarget::Act1;
	std::vector<Tga::Vector2f> myDebugRecordedPoints;

	StateHandle myGameStateHandle;
	StateHandle myBossRoomStateHandle;
	InputMapper* myInputMapper = nullptr;

	std::array<Button, static_cast<int>(ButtonType::Count)> myButtons{};
	std::array<Tga::Vector2f, static_cast<int>(ButtonType::Count)> myButtonPositions{};
	std::array<ButtonLayoutConfig, static_cast<int>(ButtonType::Count)> myButtonConfigs{};

	Tga::Sprite2DInstanceData myBackgroundSpriteInstance;
	Tga::SpriteSharedData myBackgroundSpriteData;

	Tga::Sprite2DInstanceData myOverlaySpriteInstance;
	Tga::SpriteSharedData myOverlaySpriteData;
	OverlayVariant myOverlayVariant = OverlayVariant::Shared;

	Tga::Sprite2DInstanceData myAct1SelectionSpriteInstance;
	Tga::SpriteSharedData myAct1SelectionSpriteData;

	Tga::Sprite2DInstanceData myAct2SelectionSpriteInstance;
	Tga::SpriteSharedData myAct2SelectionSpriteData;

	Tga::Sprite2DInstanceData myReturnSelectionSpriteInstance;
	Tga::SpriteSharedData myReturnSelectionSpriteData;

	int myButtonIndex = static_cast<int>(ButtonType::Act1);
	int mySelectedUI = static_cast<int>(ButtonType::Act1);

	bool myMouseHoveringAnyUI = false;

	ButtonType myReturnBackTarget = ButtonType::Act1;

	Tga::Vector2f myAct1Pos;
	Tga::Vector2f myAct2Pos;
	Tga::Vector2f myReturnPos;

	Tga::Vector2f myAct1SelectionBaseSize{ 0.0f, 0.0f };
	Tga::Vector2f myAct2SelectionBaseSize{ 0.0f, 0.0f };
	Tga::Vector2f myReturnSelectionBaseSize{ 0.0f, 0.0f };
};