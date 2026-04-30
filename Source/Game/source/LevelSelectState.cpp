#include "LevelSelectState.h"

#include <tge/Engine.h>
#include <tge/drawers/SpriteDrawer.h>
#include <tge/graphics/GraphicsEngine.h>
#include <tge/texture/TextureManager.h>


#include <imgui/imgui.h>

#include "GameState.h"
#include "MathUtils.h"
#include "ResolutionManager.h"
#include "Go.h"

#ifndef _RETAIL
#include <tge/engine.h>
#include <tge/primitives/LinePrimitive.h>
#include <iostream>
#include <tge/drawers/DebugDrawer.h>
#include <tge/drawers/LineDrawer.h>
#endif

float LevelSelectState::Cross2D(const Tga::Vector2f& a, const Tga::Vector2f& b)
{
	return a.x * b.y - a.y * b.x;
}

bool LevelSelectState::PointInConvexPolygon(const Tga::Vector2f& p, const std::vector<Tga::Vector2f>& verts)
{
	const size_t n = verts.size();
	if (n < 3)
		return false;

	int sign = 0;

	for (size_t i = 0; i < n; ++i)
	{
		const Tga::Vector2f& a = verts[i];
		const Tga::Vector2f& b = verts[(i + 1) % n];

		const Tga::Vector2f ab{ b.x - a.x, b.y - a.y };
		const Tga::Vector2f ap{ p.x - a.x, p.y - a.y };

		const float c = Cross2D(ab, ap);

		if (c == 0.0f)
			continue;

		const int currentSign = (c > 0.0f) ? 1 : -1;

		if (sign == 0)
			sign = currentSign;
		else if (currentSign != sign)
			return false;
	}

	return true;
}

void LevelSelectState::DebugDrawPolygon(const std::vector<Tga::Vector2f>& poly, const Tga::Color& color) const
{
#ifndef _RETAIL
	if (poly.size() < 2)
		return;

	Tga::LineDrawer& lineDrawer = Tga::Engine::GetInstance()->GetGraphicsEngine().GetLineDrawer();

	for (size_t i = 0; i < poly.size(); ++i)
	{
		const Tga::Vector2f& a = poly[i];
		const Tga::Vector2f& b = poly[(i + 1) % poly.size()];

		Tga::LinePrimitive line{};
		line.fromPosition = Tga::Vector3f(a, 0.0f);
		line.toPosition = Tga::Vector3f(b, 0.0f);
		line.color = color.AsVec4();

		lineDrawer.Draw(line);
	}
	
	for (size_t i = 0; i < myButtons.size(); i++)
	{
		Tga::Vector2f position = myButtons[i].GetPosition();
		Tga::Vector2f size = myButtons[i].GetSize();
		
		Tga::Vector2f tr = position + size / 2;
		Tga::Vector2f bl = position - size / 2;
		Tga::Vector2f br = {tr.x, bl.y};
		Tga::Vector2f tl = {bl.x, tr.y};
		
		Tga::LinePrimitive line1
		{
			.color = color.AsVec4(),
			.fromPosition = {tl, 0.f},
			.toPosition = {tr, 0.f},
		};
		Tga::LinePrimitive line2
		{
			.color = color.AsVec4(),
			.fromPosition = {tr, 0.f},
			.toPosition = {br, 0.f},
		};
		Tga::LinePrimitive line3
		{
			.color = color.AsVec4(),
			.fromPosition = {br, 0.f},
			.toPosition = {bl, 0.f},
		};
		Tga::LinePrimitive line4
		{
			.color = color.AsVec4(),
			.fromPosition = {bl, 0.f},
			.toPosition = {tl, 0.f},
		};
		
		lineDrawer.Draw(line1);
		lineDrawer.Draw(line2);
		lineDrawer.Draw(line3);
		lineDrawer.Draw(line4);
	}
#else
	(void)poly;
	(void)color;
#endif
}

void LevelSelectState::DebugHandlePolygonRecording()
{
#ifndef _RETAIL
	if (!myDebugRecordPoly || myInputMapper == nullptr)
		return;

	if (myInputMapper->IsActionJustActivated(GameAction::UILeftClick))
	{
		const Tga::Vector2f p = myInputMapper->GetMousePositionYUp();
		myDebugRecordedPoints.push_back(p);
		std::cout << "[LevelSelectState] Added point: (" << p.x << ", " << p.y << ")" << std::endl;
	}
#endif
}

void LevelSelectState::ResetUISelection()
{
	mySelectedUI = static_cast<int>(ButtonType::Act1);
	myButtonIndex = mySelectedUI;
	myMouseHoveringAnyUI = false;
	myReturnBackTarget = ButtonType::Act1;
}

void LevelSelectState::Init(const StateHandle aGameStateHandle, const StateHandle aBossRoomStateHandle, InputMapper* aInputMapper)
{
	myInputMapper = aInputMapper;
	myGameStateHandle = aGameStateHandle;
	myBossRoomStateHandle = aBossRoomStateHandle;

	mySelectedUI = static_cast<int>(ButtonType::Act1);
	myButtonIndex = mySelectedUI;
	myMouseHoveringAnyUI = false;
	myReturnBackTarget = ButtonType::Act1;

	const auto& engine = *Tga::Engine::GetInstance();

	myBackgroundSpriteData.myTexture = engine.GetTextureManager().GetTexture(UI.backgroundTexture);
	myBackgroundSpriteInstance.myPivot = { 0.5f, 0.5f };

	myOverlaySpriteData.myTexture = engine.GetTextureManager().GetTexture(UI.overlaySharedTexture);
	myOverlaySpriteInstance.myPivot = { 0.5f, 0.5f };
	myOverlayVariant = OverlayVariant::Shared;

	myAct1PreviewHitPolyOffsetsRef = UI.act1PreviewHitPolyOffsetsRef;
	myAct2PreviewHitPolyOffsetsRef = UI.act2PreviewHitPolyOffsetsRef;

	PositionElements();
}

void LevelSelectState::PositionElements()
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

	myOverlaySpriteInstance.myPosition = myBackgroundSpriteInstance.myPosition;
	myOverlaySpriteInstance.mySize = myBackgroundSpriteInstance.mySize;

	MenuLayoutConfig layoutConfig;
	layoutConfig.useTextureSize = UI.buttonUseTextureSize;
	layoutConfig.buttonSizeMultiplier = UI.buttonSizeMultiplier;

	//act 1 button
	myButtonConfigs[static_cast<int>(ButtonType::Act1)] =
	{
		.texturePath = UI.act1Texture,
		.induvidualXOffset = 0.0f,
		.hitboxLeftOffset = UI.act1HitboxLeftOffset,
		.hitboxRightOffset = UI.act1HitboxRightOffset,
		.selectionBarXOffset = 0.0f,
		.pivot = UI.act1Pivot,

		.rotationRadians = UI.act1HoverRotationRadians,
		.hoverNormalMultiplier = UI.act1HoverNormalMultiplier,
		.hoverMultiplier = UI.act1HoverMultiplier,

		.selectionRotationRadians = UI.act1SelectionHoverRotationRadians,
		.selectionHoverNormalMultiplier = UI.act1SelectionHoverNormalMultiplier,
		.selectionHoverMultiplier = UI.act1SelectionHoverMultiplier
	};

	//act 2 button
	myButtonConfigs[static_cast<int>(ButtonType::Act2)] =
	{
		.texturePath = UI.act2Texture,
		.induvidualXOffset = 0.0f,
		.hitboxLeftOffset = UI.act2HitboxLeftOffset,
		.hitboxRightOffset = UI.act2HitboxRightOffset,
		.selectionBarXOffset = 0.0f,
		.pivot = UI.act2Pivot,

		.rotationRadians = UI.act2HoverRotationRadians,
		.hoverNormalMultiplier = UI.act2HoverNormalMultiplier,
		.hoverMultiplier = UI.act2HoverMultiplier,

		.selectionRotationRadians = UI.act2SelectionHoverRotationRadians,
		.selectionHoverNormalMultiplier = UI.act2SelectionHoverNormalMultiplier,
		.selectionHoverMultiplier = UI.act2SelectionHoverMultiplier
	};

	//return button
	myButtonConfigs[static_cast<int>(ButtonType::Return)] =
	{
		.texturePath = UI.returnTexture,
		.induvidualXOffset = 0.0f,
		.hitboxLeftOffset = UI.returnHitboxLeftOffset,
		.hitboxRightOffset = UI.returnHitboxRightOffset,
		.selectionBarXOffset = 0.0f,
		.pivot = UI.returnPivot,

		.rotationRadians = UI.returnHoverRotationRadians,
		.hoverNormalMultiplier = UI.returnHoverNormalMultiplier,
		.hoverMultiplier = UI.returnHoverMultiplier,

		.selectionRotationRadians = UI.returnSelectionHoverRotationRadians,
		.selectionHoverNormalMultiplier = UI.returnSelectionHoverNormalMultiplier,
		.selectionHoverMultiplier = UI.returnSelectionHoverMultiplier
	};

	{
		auto& texMan = engine.GetTextureManager();

		auto initButtonAtOffset = [&](ButtonType type, const Tga::Vector2f& offsetRef)
			{
				const int idx = static_cast<int>(type);
				auto* tex = texMan.GetTexture(myButtonConfigs[idx].texturePath);

				const Tga::Vector2f texSize{ tex->CalculateTextureSize() };
				const Tga::Vector2f size = ResolutionManager::ScaleSize(
					texSize.x * UI.buttonSizeMultiplier,
					texSize.y * UI.buttonSizeMultiplier
				);

				const Tga::Vector2f pos = center + Tga::Vector2f{
					ResolutionManager::ScaleValue(offsetRef.x),
					ResolutionManager::ScaleValue(offsetRef.y)
				};

				myButtonPositions[idx] = pos;

				myButtons[idx].Init(pos, size, myButtonConfigs[idx].texturePath, myInputMapper, myButtonConfigs[idx].pivot);

				if (myButtonConfigs[idx].hitboxLeftOffset != 0.0f || myButtonConfigs[idx].hitboxRightOffset != 0.0f)
				{
					const float scaledLeftOffset = ResolutionManager::ScaleValue(myButtonConfigs[idx].hitboxLeftOffset);
					const float scaledRightOffset = ResolutionManager::ScaleValue(myButtonConfigs[idx].hitboxRightOffset);
					myButtons[idx].SetHitboxOffset(scaledLeftOffset, scaledRightOffset);
				}

				myButtons[idx].SetRotationRadians(myButtonConfigs[idx].rotationRadians);
				myButtons[idx].SetHoverScale(myButtonConfigs[idx].hoverNormalMultiplier, myButtonConfigs[idx].hoverMultiplier);
			};

		initButtonAtOffset(ButtonType::Act1, UI.act1ButtonOffsetRef);   
		initButtonAtOffset(ButtonType::Act2, UI.act2ButtonOffsetRef);   
		initButtonAtOffset(ButtonType::Return, UI.returnButtonOffsetRef); 
	}

	myAct1Pos = myButtonPositions[static_cast<int>(ButtonType::Act1)];
	myAct2Pos = myButtonPositions[static_cast<int>(ButtonType::Act2)];
	myReturnPos = myButtonPositions[static_cast<int>(ButtonType::Return)];

	MenuLayoutHelper::InitSpriteFromTexture(UI.act1SelectionTexture, myAct1SelectionSpriteData, myAct1SelectionSpriteInstance, { 0.5f, 0.5f }, UI.selectionSizeMultiplier);
	MenuLayoutHelper::InitSpriteFromTexture(UI.act2SelectionTexture, myAct2SelectionSpriteData, myAct2SelectionSpriteInstance, { 0.5f, 0.5f }, UI.selectionSizeMultiplier);
	MenuLayoutHelper::InitSpriteFromTexture(UI.returnSelectionTexture, myReturnSelectionSpriteData, myReturnSelectionSpriteInstance, { 0.5f, 0.5f }, UI.selectionSizeMultiplier);

	myAct1SelectionBaseSize = myAct1SelectionSpriteInstance.mySize;
	myAct2SelectionBaseSize = myAct2SelectionSpriteInstance.mySize;
	myReturnSelectionBaseSize = myReturnSelectionSpriteInstance.mySize;

	{
		const Tga::Vector2f act1SelOffset = {
			ResolutionManager::ScaleValue(UI.act1SelectionOffsetRef.x),
			ResolutionManager::ScaleValue(UI.act1SelectionOffsetRef.y)
		};
		const Tga::Vector2f act2SelOffset = {
			ResolutionManager::ScaleValue(UI.act2SelectionOffsetRef.x),
			ResolutionManager::ScaleValue(UI.act2SelectionOffsetRef.y)
		};
		const Tga::Vector2f returnSelOffset = {
			ResolutionManager::ScaleValue(UI.returnSelectionOffsetRef.x),
			ResolutionManager::ScaleValue(UI.returnSelectionOffsetRef.y)
		};

		myAct1SelectionSpriteInstance.myPosition = myAct1Pos + act1SelOffset;
		myAct2SelectionSpriteInstance.myPosition = myAct2Pos + act2SelOffset;
		myReturnSelectionSpriteInstance.myPosition = myReturnPos + returnSelOffset;
	}

	// Preview polygons
	myAct1PreviewHitPoly.clear();
	myAct1PreviewHitPoly.reserve(myAct1PreviewHitPolyOffsetsRef.size());
	for (const auto& o : myAct1PreviewHitPolyOffsetsRef)
	{
		myAct1PreviewHitPoly.push_back(center + Tga::Vector2f{
			ResolutionManager::ScaleValue(o.x),
			ResolutionManager::ScaleValue(o.y)
			});
	}

	myAct2PreviewHitPoly.clear();
	myAct2PreviewHitPoly.reserve(myAct2PreviewHitPolyOffsetsRef.size());
	for (const auto& o : myAct2PreviewHitPolyOffsetsRef)
	{
		myAct2PreviewHitPoly.push_back(center + Tga::Vector2f{
			ResolutionManager::ScaleValue(o.x),
			ResolutionManager::ScaleValue(o.y)
			});
	}
}

StateUpdateResult LevelSelectState::Update()
{
	if (myInputMapper == nullptr)
	{
		return StateUpdateResult::CreateContinue();
	}

#if !defined(_RETAIL)
	const ImGuiIO& io = ImGui::GetIO();
	const bool allowMouseUI = !io.WantCaptureMouse;
	const bool allowKeyboardUI = !io.WantCaptureKeyboard;
#else
	const bool allowMouseUI = true;
	const bool allowKeyboardUI = true;
#endif

#ifndef _RETAIL

	if (ImGui::Begin("LevelSelect HitArea Debug", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Checkbox("Recorded polygon points (LMB adds point)", &myDebugRecordPoly);

		const bool targetAct1 = (myDebugPolyTarget == DebugPolyTarget::Act1);
		if (ImGui::RadioButton("Target Act1", targetAct1)) myDebugPolyTarget = DebugPolyTarget::Act1;
		ImGui::SameLine();
		if (ImGui::RadioButton("Target Act2", !targetAct1)) myDebugPolyTarget = DebugPolyTarget::Act2;

		ImGui::Text("Recorded points: %d", static_cast<int>(myDebugRecordedPoints.size()));

		if (ImGui::Button("Clear recorded points"))
		{
			myDebugRecordedPoints.clear();
		}

		if (ImGui::Button("Apply recorded -> target hit poly"))
		{
			const float uiScale = ResolutionManager::GetUIScale();
			const Tga::Vector2f center = myBackgroundSpriteInstance.myPosition;

			std::vector<Tga::Vector2f> offsetsRef;
			offsetsRef.reserve(myDebugRecordedPoints.size());

			for (const auto& p : myDebugRecordedPoints)
			{
				const Tga::Vector2f offsetPx{ p.x - center.x, p.y - center.y };
				offsetsRef.push_back({ offsetPx.x / uiScale, offsetPx.y / uiScale });
			}

			if (myDebugPolyTarget == DebugPolyTarget::Act1)
				myAct1PreviewHitPolyOffsetsRef = offsetsRef;
			else
				myAct2PreviewHitPolyOffsetsRef = offsetsRef;

			PositionElements();
		}

		if (ImGui::Button("Print offsets (reference units) from menu center"))
		{
			const float uiScale = ResolutionManager::GetUIScale();
			const Tga::Vector2f center = myBackgroundSpriteInstance.myPosition;

			std::cout << "---- Reference-unit offsets from center (paste into Init()) ----\n";
			for (const auto& p : myDebugRecordedPoints)
			{
				const Tga::Vector2f offsetPx{ p.x - center.x, p.y - center.y };
				const Tga::Vector2f offsetRef{ offsetPx.x / uiScale, offsetPx.y / uiScale };
				std::cout << "{ " << offsetRef.x << "f, " << offsetRef.y << "f },\n";
			}
			std::cout << "----------------------------------------------------------------\n";
		}
	}
	ImGui::End();

	DebugHandlePolygonRecording();
#endif

	Button& act1Btn = myButtons[static_cast<int>(ButtonType::Act1)];
	Button& act2Btn = myButtons[static_cast<int>(ButtonType::Act2)];
	Button& returnBtn = myButtons[static_cast<int>(ButtonType::Return)];

	// Preview hover
	bool hoverAct1Preview = false;
	bool hoverAct2Preview = false;

	bool mouseHoverAct1 = false;
	bool mouseHoverAct2 = false;
	bool mouseHoverReturn = false;

	if (allowMouseUI)
	{
		const Tga::Vector2f mousePos = myInputMapper->GetMousePositionYUp();

		hoverAct1Preview = PointInConvexPolygon(mousePos, myAct1PreviewHitPoly);
		hoverAct2Preview = PointInConvexPolygon(mousePos, myAct2PreviewHitPoly);

		mouseHoverAct1 = hoverAct1Preview;
		mouseHoverAct2 = hoverAct2Preview;

		mouseHoverReturn = false;

		myMouseHoveringAnyUI = mouseHoverAct1 || mouseHoverAct2; 

		if (myMouseHoveringAnyUI)
		{
			if (mouseHoverAct1) myButtonIndex = static_cast<int>(ButtonType::Act1);
			else if (mouseHoverAct2) myButtonIndex = static_cast<int>(ButtonType::Act2);
		}

		const bool lmbJustPressed = myInputMapper->IsActionJustActivated(GameAction::UILeftClick);
		if (lmbJustPressed)
		{
			if (hoverAct1Preview)
			{
				ResetUISelection();
				return PressButton(ButtonType::Act1);
			}
			if (hoverAct2Preview)
			{
				ResetUISelection();
				return PressButton(ButtonType::Act2);
			}
		}
	}
	else
	{
		myMouseHoveringAnyUI = false;
	}

	if (allowKeyboardUI)
	{
		if (myInputMapper->IsActionJustActivated(GameAction::UICancel))
		{
			ResetUISelection();
			return PressButton(ButtonType::Return);
		}

		if (!myMouseHoveringAnyUI)
		{
			const ButtonType current = static_cast<ButtonType>(mySelectedUI);

			if (myInputMapper->IsActionJustActivated(GameAction::UILeft) ||
				myInputMapper->IsActionJustActivated(GameAction::UIRight))
			{
				if (current == ButtonType::Act1) mySelectedUI = static_cast<int>(ButtonType::Act2);
				else if (current == ButtonType::Act2) mySelectedUI = static_cast<int>(ButtonType::Act1);
			}

			if (myInputMapper->IsActionJustActivated(GameAction::UIUp))
			{
				if (current == ButtonType::Act1 || current == ButtonType::Act2)
				{
					myReturnBackTarget = current;
					mySelectedUI = static_cast<int>(ButtonType::Return);
				}
			}

			if (myInputMapper->IsActionJustActivated(GameAction::UIDown))
			{
				if (current == ButtonType::Return)
				{
					mySelectedUI = static_cast<int>(myReturnBackTarget);
				}
			}

			myButtonIndex = mySelectedUI;

			if (myInputMapper->IsActionJustActivated(GameAction::UIConfirm))
			{
				const ButtonType type = static_cast<ButtonType>(mySelectedUI);
				ResetUISelection();
				return PressButton(type);
			}
		}
	}

	const bool shouldHoverAct1 = myMouseHoveringAnyUI ? mouseHoverAct1 : (mySelectedUI == static_cast<int>(ButtonType::Act1));
	const bool shouldHoverAct2 = myMouseHoveringAnyUI ? mouseHoverAct2 : (mySelectedUI == static_cast<int>(ButtonType::Act2));
	const bool shouldHoverReturn = myMouseHoveringAnyUI ? mouseHoverReturn : (mySelectedUI == static_cast<int>(ButtonType::Return));

	const bool act1HoverFromSpriteOrForced = act1Btn.Update(shouldHoverAct1);
	const bool act2HoverFromSpriteOrForced = act2Btn.Update(shouldHoverAct2);
	const bool returnHoverFromSpriteOrForced = returnBtn.Update(shouldHoverReturn);

	const bool finalHoverAct1 = hoverAct1Preview || act1HoverFromSpriteOrForced;
	const bool finalHoverAct2 = hoverAct2Preview || act2HoverFromSpriteOrForced;
	const bool finalHoverReturn = returnHoverFromSpriteOrForced;

	const bool hasAnyHover = finalHoverAct1 || finalHoverAct2 || finalHoverReturn;

	myMouseHoveringAnyUI = allowMouseUI && hasAnyHover;

	if (hasAnyHover)
	{
		if (finalHoverReturn) myButtonIndex = static_cast<int>(ButtonType::Return);
		else if (finalHoverAct2) myButtonIndex = static_cast<int>(ButtonType::Act2);
		else if (finalHoverAct1) myButtonIndex = static_cast<int>(ButtonType::Act1);
	}
	else
	{
		myButtonIndex = mySelectedUI;
	}

	if (allowMouseUI)
	{
		if (act1Btn.IsPressed())
		{
			ResetUISelection();
			return PressButton(ButtonType::Act1);
		}
		if (act2Btn.IsPressed())
		{
			ResetUISelection();
			return PressButton(ButtonType::Act2);
		}
		if (returnBtn.IsPressed())
		{
			ResetUISelection();
			return PressButton(ButtonType::Return);
		}
	}

	MenuLayoutHelper::ApplySelectionVisuals(
		myButtonConfigs[static_cast<int>(ButtonType::Act1)],
		myAct1SelectionSpriteInstance,
		myAct1SelectionBaseSize,
		finalHoverAct1
	);

	MenuLayoutHelper::ApplySelectionVisuals(
		myButtonConfigs[static_cast<int>(ButtonType::Act2)],
		myAct2SelectionSpriteInstance,
		myAct2SelectionBaseSize,
		finalHoverAct2
	);

	MenuLayoutHelper::ApplySelectionVisuals(
		myButtonConfigs[static_cast<int>(ButtonType::Return)],
		myReturnSelectionSpriteInstance,
		myReturnSelectionBaseSize,
		finalHoverReturn
	);

	{
		const Tga::Vector2f act1Offset = {
			ResolutionManager::ScaleValue(UI.act1SelectionOffsetRef.x),
			ResolutionManager::ScaleValue(UI.act1SelectionOffsetRef.y)
		};
		const Tga::Vector2f act2Offset = {
			ResolutionManager::ScaleValue(UI.act2SelectionOffsetRef.x),
			ResolutionManager::ScaleValue(UI.act2SelectionOffsetRef.y)
		};
		const Tga::Vector2f returnOffset = {
			ResolutionManager::ScaleValue(UI.returnSelectionOffsetRef.x),
			ResolutionManager::ScaleValue(UI.returnSelectionOffsetRef.y)
		};

		myAct1SelectionSpriteInstance.myPosition = myAct1Pos + act1Offset;
		myAct2SelectionSpriteInstance.myPosition = myAct2Pos + act2Offset;
		myReturnSelectionSpriteInstance.myPosition = myReturnPos + returnOffset;
	}

	{
		OverlayVariant desired = OverlayVariant::Shared;

		switch (static_cast<ButtonType>(myButtonIndex))
		{
		case ButtonType::Act1: desired = OverlayVariant::MissingAct2; break;
		case ButtonType::Act2: desired = OverlayVariant::MissingAct1; break;
		case ButtonType::Return: desired = OverlayVariant::Shared; break;
		default: desired = OverlayVariant::Shared; break;
		}

		if (desired != myOverlayVariant)
		{
			myOverlayVariant = desired;

			const auto& engine = *Tga::Engine::GetInstance();
			auto& texMan = engine.GetTextureManager();

			switch (myOverlayVariant)
			{
			case OverlayVariant::Shared:
				myOverlaySpriteData.myTexture = texMan.GetTexture(UI.overlaySharedTexture);
				break;
			case OverlayVariant::MissingAct1:
				myOverlaySpriteData.myTexture = texMan.GetTexture(UI.overlayMissingAct1Texture);
				break;
			case OverlayVariant::MissingAct2:
				myOverlaySpriteData.myTexture = texMan.GetTexture(UI.overlayMissingAct2Texture);
				break;
			default:
				break;
			}
		}
	}

#if !defined(_RETAIL)
	if (ImGui::Begin("Debug scene loading", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static constexpr int size = 256;
		static char input[size]{ 0 };
		ImGui::InputText("Scene path", input, size);

		if (ImGui::Button("Load scene"))
		{
			ImGui::End();

			myButtonIndex = static_cast<int>(ButtonType::Act1);
			mySelectedUI = static_cast<int>(ButtonType::Act1);
			SceneLoader::SceneConfig& sceneConfig = SceneLoader::LoadSceneByPath(input);
			if (sceneConfig.metaConfig.type == SceneLoader::SceneType::BossScene)
			{
				return StateUpdateResult::CreateClearAndPush(myBossRoomStateHandle);
			}
			else
			{
				return StateUpdateResult::CreateClearAndPush(myGameStateHandle);
			}
		}
	}
	ImGui::End();
#endif

	return StateUpdateResult::CreateContinue();
}

void LevelSelectState::Render()
{
	const Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::SpriteDrawer& spriteDrawer = engine.GetGraphicsEngine().GetSpriteDrawer();

	spriteDrawer.Draw(myBackgroundSpriteData, myBackgroundSpriteInstance);

	if (myOverlaySpriteData.myTexture)
	{
		spriteDrawer.Draw(myOverlaySpriteData, myOverlaySpriteInstance);
	}

	switch (static_cast<ButtonType>(myButtonIndex))
	{
	case ButtonType::Act1:
	{
		if (myAct1SelectionSpriteData.myTexture)
		{
			spriteDrawer.Draw(myAct1SelectionSpriteData, myAct1SelectionSpriteInstance);
		}
		break;
	}
	case ButtonType::Act2:
	{
		if (myAct2SelectionSpriteData.myTexture)
		{
			spriteDrawer.Draw(myAct2SelectionSpriteData, myAct2SelectionSpriteInstance);
		}
		break;
	}
	case ButtonType::Return:
	{
		if (myReturnSelectionSpriteData.myTexture)
		{
			spriteDrawer.Draw(myReturnSelectionSpriteData, myReturnSelectionSpriteInstance);
		}
		break;
	}
	default:
		break;
	}

	myButtons[static_cast<int>(ButtonType::Act1)].Render();
	myButtons[static_cast<int>(ButtonType::Act2)].Render();
	myButtons[static_cast<int>(ButtonType::Return)].Render();

#ifndef _RETAIL

	DebugDrawPolygon(myAct1PreviewHitPoly, Tga::Color(1, 0, 0, 1));
	DebugDrawPolygon(myAct2PreviewHitPoly, Tga::Color(0, 1, 0, 1));

	if (myDebugRecordPoly)
	{
		DebugDrawPolygon(myDebugRecordedPoints, Tga::Color(1, 1, 0, 1));
	}
#endif
}

void LevelSelectState::OnGainFocus()
{
	SetMouseCaptureEnabled(false);
	AudioManager::GetAudioPoolByHandle(AudioHandles::windowChange).Play();
}

StateUpdateResult LevelSelectState::PressButton(ButtonType aButtonType) const
{
	AudioManager::GetAudioPoolByHandle(AudioHandles::clickButton).Play();
	
	switch (aButtonType)
    {
		case ButtonType::Act1:
        {
			std::cout << "[LevelSelectState.cpp] Level1\n";
			SceneLoader::KillPreloadProcess();
			SceneLoader::LoadSceneByPath("levels/Level2.tgs");
			SceneLoader::LoadSceneByPath("levels/Level1.tgs");
	        return StateUpdateResult::CreateClearAndPush(myGameStateHandle);
        }
        case ButtonType::Act2:
        {
			std::cout << "[LevelSelectState.cpp] Level2\n";
			SceneLoader::KillPreloadProcess();
			SceneLoader::LoadSceneByPath("levels/Level1.tgs");
			SceneLoader::LoadSceneByPath("levels/Level2.tgs");
	        return StateUpdateResult::CreateClearAndPush(myGameStateHandle);
        }
        case ButtonType::Return:
        {
			std::cout << "[LevelSelectState.cpp] Back\n";
	        return StateUpdateResult::CreatePop();
        }
		default:
		{
			std::cout << "[LevelSelectState.cpp] Invalid button index\n";
			return StateUpdateResult::CreateContinue();
		}
    }
}

