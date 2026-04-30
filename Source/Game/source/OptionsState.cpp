#include "OptionsState.h"
#include "ResolutionManager.h"

#include <tge/Engine.h>
#include <tge/drawers/SpriteDrawer.h>
#include <tge/graphics/GraphicsEngine.h>
#include <tge/texture/TextureManager.h>
#include <tge/input/InputManager.h>

#include <limits>
#include <algorithm>

#include "AudioManager.h"
#include "Go.h"
#include "MathUtils.h"
#include "imgui/imgui.h"

static constexpr bool kEnableMusicSlider = false;

static bool IsMouseOverSprite(const Tga::Vector2f& mousePos, const Tga::Sprite2DInstanceData& instance)
{
	const float left = instance.myPosition.x - (instance.mySize.x * instance.myPivot.x);
	const float right = instance.myPosition.x + (instance.mySize.x * (1.0f - instance.myPivot.x));
	const float top = instance.myPosition.y + (instance.mySize.y * (1.0f - instance.myPivot.y));
	const float bottom = instance.myPosition.y - (instance.mySize.y * instance.myPivot.y);

	return (mousePos.x >= left && mousePos.x <= right) &&
		(mousePos.y >= bottom && mousePos.y <= top);
}

void OptionsState::Init(InputMapper* aInputMapper, Timer* aTimer)
{
	myInputMapper = aInputMapper;
	myTimer = aTimer;
	mySelectedUI = 1;

	const auto& engine = *Tga::Engine::GetInstance();

	myBackgroundSpriteData.myTexture = engine.GetTextureManager().GetTexture(UI.backgroundTexture);
	myBackgroundSpriteInstance.myPivot = { 0.5f, 0.5f };

	myPrevMasterVolume = Options::masterVolume;
	myPrevMusicVolume = Options::musicVolume;

	myUILastTick = std::chrono::steady_clock::now();
	myUIHasTicked = true;

	PositionElements();
}

void OptionsState::PositionElements()
{
	const auto& engine = *Tga::Engine::GetInstance();
	const auto renderSize = engine.GetRenderSize();
	const Tga::Vector2f resolution = { static_cast<float>(renderSize.x), static_cast<float>(renderSize.y) };

	const Tga::Vector2f textureSize{ myBackgroundSpriteData.myTexture->CalculateTextureSize() };
	float texAspect = textureSize.x / textureSize.y;
	float screenAspect = resolution.x / resolution.y;

	if (screenAspect > texAspect)
	{
		//Screen is wider - fit to width
		myBackgroundSpriteInstance.mySize.x = resolution.x;
		myBackgroundSpriteInstance.mySize.y = resolution.x / texAspect;
	}
	else
	{
		//Screen is taller - fit to height
		myBackgroundSpriteInstance.mySize.y = resolution.y;
		myBackgroundSpriteInstance.mySize.x = resolution.y * texAspect;
	}

	myBackgroundSpriteInstance.myPosition = resolution * 0.5f;
	const Tga::Vector2f center = myBackgroundSpriteInstance.myPosition;

	MenuLayoutHelper::InitSpriteFromTexture(UI.sliderBarTexture, myMasterSliderBarData, myMasterSliderBarInstance, { 0.5f, 0.5f }, UI.sliderBarSizeMultiplier);
	MenuLayoutHelper::InitSpriteFromTexture(UI.sliderKnobTexture, myMasterSliderKnobData, myMasterSliderKnobInstance, { 0.5f, 0.5f }, UI.sliderKnobSizeMultiplier);

	if constexpr (kEnableMusicSlider)
	{
		myMusicSliderBarData = myMasterSliderBarData;
		myMusicSliderKnobData = myMasterSliderKnobData;
		myMusicSliderBarInstance.myPivot = { 0.5f, 0.5f };
		myMusicSliderBarInstance.mySize = myMasterSliderBarInstance.mySize;
		myMusicSliderKnobInstance.myPivot = { 0.5f, 0.5f };
		myMusicSliderKnobInstance.mySize = myMasterSliderKnobInstance.mySize;
	}

	myMasterSliderKnobInstance.myColor = Tga::Color(1, 1, 1, 1);
	myMusicSliderKnobInstance.myColor = Tga::Color(1, 1, 1, 1);

	MenuLayoutHelper::InitSpriteFromTexture(UI.checkmarkTexture, myMuteCheckmarkData, myMuteCheckmarkInstance, { 0.5f, 0.5f }, UI.checkmarkSizeMultiplier);
	MenuLayoutHelper::InitSpriteFromTexture(UI.checkmarkTexture, myFSCheckmarkData, myFSCheckmarkInstance, { 0.5f, 0.5f }, UI.checkmarkSizeMultiplier);

	MenuLayoutHelper::InitSpriteFromTexture(UI.muteOutlineTexture, myMuteOutlineData, myMuteOutlineInstance, { 0.5f, 0.5f }, UI.boxOutlineSizeMultiplier);
	MenuLayoutHelper::InitSpriteFromTexture(UI.fsOutlineTexture, myFSOutlineData, myFSOutlineInstance, { 0.5f, 0.5f }, UI.boxOutlineSizeMultiplier);

	MenuLayoutHelper::InitSpriteFromTexture(UI.masterSliderOutlineTexture, myMasterSliderOutlineData, myMasterSliderOutlineInstance, { 0.5f, 0.5f }, UI.handleOutlineSizeMultiplier);

	if constexpr (kEnableMusicSlider)
	{
		MenuLayoutHelper::InitSpriteFromTexture(UI.musicSliderOutlineTexture, myMusicSliderOutlineData, myMusicSliderOutlineInstance, { 0.5f, 0.5f }, UI.handleOutlineSizeMultiplier);
	}

	MenuLayoutHelper::InitSpriteFromTexture(UI.returnSelectedTexture, myReturnBarSpriteData, myReturnBarSpriteInstance, { 0.5f, 0.5f }, UI.buttonSizeMultiplier);

	MenuLayoutHelper::InitSpriteFromTexture(UI.windowedHeaderTexture, myWindowedSpriteData, myWindowedSpriteInstance, { 0.5f, 0.5f }, UI.headerSizeMultiplier);
	MenuLayoutHelper::InitSpriteFromTexture(UI.volumeHeaderTexture, myVolumeSpriteData, myVolumeSpriteInstance, { 0.5f, 0.5f }, UI.headerSizeMultiplier);
	MenuLayoutHelper::InitSpriteFromTexture(UI.muteHeaderTexture, myMuteHeaderSpriteData, myMuteHeaderSpriteInstance, { 0.5f, 0.5f }, UI.headerSizeMultiplier);

	MenuLayoutHelper::InitSpriteFromTexture(UI.minVolTexture, myMinVolSpriteData, myMinVolSpriteInstance, { 0.5f, 0.5f }, UI.volSizeMultiplier);
	MenuLayoutHelper::InitSpriteFromTexture(UI.maxVolTexture, myMaxVolSpriteData, myMaxVolSpriteInstance, { 0.5f, 0.5f }, UI.volSizeMultiplier);

	//buttons
	{
		auto& texMan = engine.GetTextureManager();

		//back
		{
			auto* tex = texMan.GetTexture(UI.backTexture);
			const Tga::Vector2f texSize{ tex->CalculateTextureSize() };
			const Tga::Vector2f size = ResolutionManager::ScaleSize(texSize.x * UI.buttonSizeMultiplier, texSize.y * UI.buttonSizeMultiplier);

			myBackButton.Init(
				center + Tga::Vector2f{ ResolutionManager::ScaleValue(UI.backPositionX), ResolutionManager::ScaleValue(UI.backPositionY) },
				size,
				UI.backTexture,
				myInputMapper
			);

			myReturnBarSpriteInstance.myPosition = myBackButton.GetPosition() + Tga::Vector2f{
				ResolutionManager::ScaleValue(UI.returnBarOffsetX),
				ResolutionManager::ScaleValue(UI.returnBarOffsetY)
			};
		}

		//mute
		{
			auto* tex = texMan.GetTexture(UI.muteTexture);
			const Tga::Vector2f texSize{ tex->CalculateTextureSize() };
			const Tga::Vector2f size = ResolutionManager::ScaleSize(texSize.x * UI.boxSizeMultiplier, texSize.y * UI.boxSizeMultiplier);

			myMuteButton.Init(
				center + Tga::Vector2f{ ResolutionManager::ScaleValue(UI.mutePositionX), ResolutionManager::ScaleValue(UI.mutePositionY) },
				size,
				UI.muteTexture,
				myInputMapper
			);
		}

		//windowed/fullscreen box
		{
			auto* tex = texMan.GetTexture(UI.fsTexture);
			const Tga::Vector2f texSize{ tex->CalculateTextureSize() };
			const Tga::Vector2f size = ResolutionManager::ScaleSize(texSize.x * UI.boxSizeMultiplier, texSize.y * UI.boxSizeMultiplier);

			myFullscreenButton.Init(
				center + Tga::Vector2f{ ResolutionManager::ScaleValue(UI.fsPositionX), ResolutionManager::ScaleValue(UI.fsPositionY) },
				size,
				UI.fsTexture,
				myInputMapper
			);
		}
	}

	MenuLayoutHelper::SetSpritePositionFromOffsets(myMasterSliderBarInstance, center, UI.sliderPositionX, UI.sliderPositionY);

	if constexpr (kEnableMusicSlider)
	{
		MenuLayoutHelper::SetSpritePositionFromOffsets(myMusicSliderBarInstance, center, UI.sliderPositionX, UI.sliderPositionY - UI.sliderVerticalSpacing);
	}

	myMuteCheckmarkInstance.myPosition = myMuteButton.GetPosition();
	myFSCheckmarkInstance.myPosition = myFullscreenButton.GetPosition();

	myMuteOutlineInstance.myPosition = myMuteButton.GetPosition();
	myFSOutlineInstance.myPosition = myFullscreenButton.GetPosition();

	const float dx = ResolutionManager::ScaleValue(UI.checkmarkXOffset);
	const float dy = ResolutionManager::ScaleValue(UI.checkmarkYOffset);

	myMuteCheckmarkInstance.myPosition.x += dx;
	myMuteCheckmarkInstance.myPosition.y += dy;

	myFSCheckmarkInstance.myPosition.x += dx;
	myFSCheckmarkInstance.myPosition.y += dy;

	// header sprites
	myWindowedSpriteInstance.myPosition = center + Tga::Vector2f{
		ResolutionManager::ScaleValue(UI.windowedHeaderPositionX),
		ResolutionManager::ScaleValue(UI.windowedHeaderPositionY)
	};

	myMuteHeaderSpriteInstance.myPosition = center + Tga::Vector2f{
		ResolutionManager::ScaleValue(UI.muteHeaderPositionX),
		ResolutionManager::ScaleValue(UI.muteHeaderPositionY)
	};

	myVolumeSpriteInstance.myPosition = center + Tga::Vector2f{
		ResolutionManager::ScaleValue(UI.volumeHeaderPositionX),
		ResolutionManager::ScaleValue(UI.volumeHeaderPositionY)
	};

	myMinVolSpriteInstance.myPosition = center + Tga::Vector2f{
		ResolutionManager::ScaleValue(UI.minVolPositionX),
		ResolutionManager::ScaleValue(UI.minVolPositionY)
	};

	myMaxVolSpriteInstance.myPosition = center + Tga::Vector2f{
		ResolutionManager::ScaleValue(UI.maxVolPositionX),
		ResolutionManager::ScaleValue(UI.maxVolPositionY)
	};

	UpdateSliderKnobPosition();
}

void OptionsState::OnGainFocus()
{
	SetMouseCaptureEnabled(false);
	AudioManager::GetAudioPoolByHandle(AudioHandles::windowChange).Play();
}

void OptionsState::ToggleDualStick()
{
	const auto& engine = *Tga::Engine::GetInstance();
	const auto renderSize = engine.GetRenderSize();
	const Tga::Vector2f resolution = { static_cast<float>(renderSize.x), static_cast<float>(renderSize.y) };
	
	Options::enableDualStick = !Options::enableDualStick;
	Options::shotgunOnRS = Options::enableDualStick;
	myDualStick.text.SetText(Options::enableDualStick ? "DualStick Enabled" : "DualStick Disabled");
	myDualStick.text.SetScale(ResolutionManager::GetUIScale() * myDualStick.sizeMultiplier);
	myDualStick.text.SetPosition({(resolution.x - myDualStick.text.GetWidth()) * 0.5f , resolution.y * 0.5f});
	myDualStick.text.SetColor({1.f,1.f,1.f,1.f});
	myDualStick.time = 0.f;
}

void OptionsState::UpdateDualStickText()
{
	const auto& engine = *Tga::Engine::GetInstance();
	const auto renderSize = engine.GetRenderSize();
	const Tga::Vector2f resolution = { static_cast<float>(renderSize.x), static_cast<float>(renderSize.y) };	
	
	const float deltaTime = myTimer->GetDeltaTime();
	myDualStick.time += deltaTime;
	const float dualStickAlphaPercentage = 1.f - MathUtils::Clamp01(myDualStick.time / myDualStick.duration);
	myDualStick.text.SetColor({1.f, 1.f, 1.f, dualStickAlphaPercentage});
	myDualStick.text.SetScale(ResolutionManager::GetUIScale() * myDualStick.sizeMultiplier);
	myDualStick.text.SetPosition({(resolution.x - myDualStick.text.GetWidth()) * 0.5f , resolution.y * 0.5f});
}

StateUpdateResult OptionsState::Update()
{
	AudioManager::UpdateVolume(Options::masterVolume, Options::musicVolume, Options::maxVolume);

	if (myInputMapper == nullptr)
	{
		return StateUpdateResult::CreateContinue();
	}
	
	if (myInputMapper->IsActionJustActivated(GameAction::LSPress))
	{
		if (myInputMapper->IsActionActive(GameAction::RSPress))
		{
			ToggleDualStick();
		}
	}
	else if (myInputMapper->IsActionJustActivated(GameAction::RSPress))
	{
		if (myInputMapper->IsActionActive(GameAction::LSPress))
		{
			ToggleDualStick();
		}
	}
	
	UpdateDualStickText();
	
	if (myInputMapper->IsActionJustActivated(GameAction::UICancel))
	{
		mySelectedUI = 1;
		AudioManager::GetAudioPoolByHandle(AudioHandles::uiBack).Play();
		return StateUpdateResult::CreatePop();
	}

	const Tga::Vector2f mousePos = myInputMapper->GetMousePositionYUp();

	const bool mouseOverMasterBar = IsMouseOverBar(mousePos, myMasterSliderBarInstance);
	const bool mouseOverMasterKnob = IsMouseOverSprite(mousePos, myMasterSliderKnobInstance);
	const bool mouseOverMaster = mouseOverMasterBar || mouseOverMasterKnob;

	if (mouseOverMaster && !myMouseOverMasterPreviousFrame)
	{
		AudioManager::GetAudioPoolByHandle(AudioHandles::hoverButton).Play();
	}

	myMouseOverMasterPreviousFrame = mouseOverMaster;

	bool mouseOverMusic = false;
	if constexpr (kEnableMusicSlider)
	{
		const bool mouseOverMusicBar = IsMouseOverBar(mousePos, myMusicSliderBarInstance);
		const bool mouseOverMusicKnob = IsMouseOverSprite(mousePos, myMusicSliderKnobInstance);
		mouseOverMusic = mouseOverMusicBar || mouseOverMusicKnob;
	}

	const bool myMouseBackHover = myBackButton.Update(false);
	const bool myMouseMuteHover = myMuteButton.Update(false);
	const bool myMouseFSHover = myFullscreenButton.Update(false);

	myMouseHoveringAnyUI = mouseOverMaster || mouseOverMusic || myMouseBackHover || myMouseMuteHover || myMouseFSHover;

	if (myMouseHoveringAnyUI)
	{
		if (myMouseBackHover) mySelectedUI = 0;
		else if (myMouseFSHover) mySelectedUI = 1;
		else if (myMouseMuteHover) mySelectedUI = 2;
		else if (mouseOverMaster) mySelectedUI = 3;
		else if (mouseOverMusic) mySelectedUI = 4;
	}

	myBackHover = myMouseBackHover;
	myMuteHover = myMouseMuteHover;
	myFSHover = myMouseFSHover;

	if (myBackButton.IsPressed())
	{
		mySelectedUI = 1;
		return StateUpdateResult::CreatePop();
	}

	if (myMuteButton.IsPressed())
	{
		ToggleMute();
	}

	if (myFullscreenButton.IsPressed())
	{
		ToggleFullscreen();
	}

	bool upPressed = (myInputMapper && myInputMapper->IsActionJustActivated(GameAction::UIUp));
	bool downPressed = (myInputMapper && myInputMapper->IsActionJustActivated(GameAction::UIDown));

	if (upPressed)
	{
		int previous = mySelectedUI;
		mySelectedUI = std::clamp(mySelectedUI - 1, 0, 3);
		if (previous != mySelectedUI)
		{
			AudioManager::GetAudioPoolByHandle(AudioHandles::hoverButton).Play();
		}
	}
	if (downPressed)
	{
		int previous = mySelectedUI;
		mySelectedUI = std::clamp(mySelectedUI + 1, 0, 3);
		if (previous != mySelectedUI)
		{
			AudioManager::GetAudioPoolByHandle(AudioHandles::hoverButton).Play();
		}
	}

	float uiDt = 0.0f;
	{
		const auto now = std::chrono::steady_clock::now();
		if (!myUIHasTicked)
		{
			myUILastTick = now;
			myUIHasTicked = true;
			uiDt = 0.0f;
		}
		else
		{
			const std::chrono::duration<float> elapsed = now - myUILastTick;
			myUILastTick = now;
			uiDt = elapsed.count();
		}

		uiDt = (std::min)(uiDt, 0.1f);
	}

	const float kVolumeUnitsPerSecond = 300.0f;

	const bool leftJustPressed = myInputMapper->IsActionJustActivated(GameAction::UILeft);
	const bool rightJustPressed = myInputMapper->IsActionJustActivated(GameAction::UIRight);
	const bool leftHeld = myInputMapper->IsActionActive(GameAction::UILeft);
	const bool rightHeld = myInputMapper->IsActionActive(GameAction::UIRight);

	int dir = 0;
	if (leftHeld && !rightHeld) dir = -1;
	if (rightHeld && !leftHeld) dir = +1;

	if (dir == 0)
	{
		mySliderRepeatDirection = 0;
		mySliderRepeatTimer = 0.0f;
	}
	if (leftJustPressed)
	{
		if (mySelectedUI == 3) ChangeVolumeBy(-3, 0);
		if (mySelectedUI == 4) ChangeVolumeBy(-3, 1);
	}
	else if (rightJustPressed)
	{
		if (mySelectedUI == 3) ChangeVolumeBy(+3, 0);
		if (mySelectedUI == 4) ChangeVolumeBy(+3, 1);
	}

	if (dir != 0)
	{
		if (mySelectedUI == 3) ApplyContinuousVolumeMove(uiDt, dir, kVolumeUnitsPerSecond, 0);
		if (mySelectedUI == 4) ApplyContinuousVolumeMove(uiDt, dir, kVolumeUnitsPerSecond, 1);
	}

	if (myInputMapper->IsActionJustActivated(GameAction::UILeftClick))
	{
		if (mouseOverMaster && !isMuted)
		{
			myDraggingSlider = 0;
			mySelectedUI = 3;
			SetVolumeFromMouseX(mousePos.x, 0);
		}
		else if constexpr (kEnableMusicSlider)
		{
			if (mouseOverMusic)
			{
				myDraggingSlider = 1;
				mySelectedUI = 4;
				SetVolumeFromMouseX(mousePos.x, 1);
			}
		}
	}

	if (myDraggingSlider != -1 && myInputMapper->IsActionActive(GameAction::UILeftClick))
	{
		SetVolumeFromMouseX(mousePos.x, myDraggingSlider);
	}

	if (myDraggingSlider != -1 && myInputMapper->IsActionJustDeactivated(GameAction::UILeftClick))
	{
		myDraggingSlider = -1;
		AudioManager::GetAudioPoolByHandle(AudioHandles::revolverShot).Play();
	}

	if (myInputMapper->IsActionJustActivated(GameAction::UIConfirm))
	{
		if (mySelectedUI == 0)
		{
			mySelectedUI = 1;
			return StateUpdateResult::CreatePop();
		}

		if (mySelectedUI == 3 && !isMuted)
		{
			myDraggingSlider = 0;
			SetVolumeFromMouseX(mousePos.x, 0);
		}
		else if (mySelectedUI == 1 && !mouseOverMaster)
		{
			ToggleFullscreen();
		}
		else if (mySelectedUI == 2 && !mouseOverMaster)
		{
			ToggleMute();
		}
	}

	//Highlight
	auto setKnobColor = [](Tga::Sprite2DInstanceData& knob, bool hoveringOrDragging, bool selected)
		{
			if (hoveringOrDragging)
			{
				knob.myColor = Tga::Color(1.0f, 1.0f, 1.0f, 1.0f);
			}
			else if (selected)
			{
				knob.myColor = Tga::Color(1.0f, 1.0f, 1.0f, 1.0f);
			}
			else
			{
				knob.myColor = Tga::Color(1.0f, 1.0f, 1.0f, 1.0f);
			}
		};

	const bool masterSelected = (mySelectedUI == 3) && !myMouseHoveringAnyUI;
	const bool musicSelected = (mySelectedUI == 4) && !myMouseHoveringAnyUI;

	setKnobColor(myMasterSliderKnobInstance, (mouseOverMaster || myDraggingSlider == 0), masterSelected);
	setKnobColor(myMusicSliderKnobInstance, (mouseOverMusic || myDraggingSlider == 1), musicSelected);

	return StateUpdateResult::CreateContinue();
}

void OptionsState::Render()
{
	const Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::SpriteDrawer& spriteDrawer = engine.GetGraphicsEngine().GetSpriteDrawer();

	//ui/background
	spriteDrawer.Draw(myBackgroundSpriteData, myBackgroundSpriteInstance);

	spriteDrawer.Draw(myWindowedSpriteData, myWindowedSpriteInstance);
	spriteDrawer.Draw(myVolumeSpriteData, myVolumeSpriteInstance);
	spriteDrawer.Draw(myMuteHeaderSpriteData, myMuteHeaderSpriteInstance);

	spriteDrawer.Draw(myMinVolSpriteData, myMinVolSpriteInstance);
	spriteDrawer.Draw(myMaxVolSpriteData, myMaxVolSpriteInstance);

	const Tga::Vector2f mousePos = (myInputMapper) ? myInputMapper->GetMousePositionYUp() : Tga::Vector2f{ -99999.0f, -99999.0f };

	const bool mouseOverMasterBar = IsMouseOverBar(mousePos, myMasterSliderBarInstance);
	const bool mouseOverMasterKnob = IsMouseOverSprite(mousePos, myMasterSliderKnobInstance);
	const bool mouseOverMaster = mouseOverMasterBar || mouseOverMasterKnob;

	const bool mouseOverMusicBar = IsMouseOverBar(mousePos, myMusicSliderBarInstance);
	const bool mouseOverMusicKnob = IsMouseOverSprite(mousePos, myMusicSliderKnobInstance);
	const bool mouseOverMusic = mouseOverMusicBar || mouseOverMusicKnob;

	const bool showMasterOutline =
		(myDraggingSlider == 0) ||
		(myMouseHoveringAnyUI ? mouseOverMaster : (mySelectedUI == 3));

	const bool showMusicOutline =
		(myDraggingSlider == 1) ||
		(myMouseHoveringAnyUI ? mouseOverMusic : (mySelectedUI == 4));

	const bool showMuteOutline =
		(myMuteOutlineData.myTexture && (myMouseHoveringAnyUI ? myMuteHover : (mySelectedUI == 2)));

	const bool showFSOutline =
		(myFSOutlineData.myTexture && (myMouseHoveringAnyUI ? myFSHover : (mySelectedUI == 1)));

	const bool showReturnBar =
		(myReturnBarSpriteData.myTexture && (myBackHover || (!myMouseHoveringAnyUI && mySelectedUI == 0)));

	if (myMasterSliderBarData.myTexture)
		spriteDrawer.Draw(myMasterSliderBarData, myMasterSliderBarInstance);
	if (myMasterSliderOutlineData.myTexture && showMasterOutline)
		spriteDrawer.Draw(myMasterSliderOutlineData, myMasterSliderOutlineInstance);
	if (myMasterSliderKnobData.myTexture)
		spriteDrawer.Draw(myMasterSliderKnobData, myMasterSliderKnobInstance);

	if constexpr (kEnableMusicSlider)
	{
		if (myMusicSliderBarData.myTexture)
			spriteDrawer.Draw(myMusicSliderBarData, myMusicSliderBarInstance);
		if (myMusicSliderOutlineData.myTexture && showMusicOutline)
			spriteDrawer.Draw(myMusicSliderOutlineData, myMusicSliderOutlineInstance);
		if (myMusicSliderKnobData.myTexture)
			spriteDrawer.Draw(myMusicSliderKnobData, myMusicSliderKnobInstance);
	}

	if (showMuteOutline)
		spriteDrawer.Draw(myMuteOutlineData, myMuteOutlineInstance);
	myMuteButton.Render();
	if (myMuteCheckmarkData.myTexture && isMuted)
		spriteDrawer.Draw(myMuteCheckmarkData, myMuteCheckmarkInstance);

	if (showFSOutline)
		spriteDrawer.Draw(myFSOutlineData, myFSOutlineInstance);
	myFullscreenButton.Render();
	if (myFSCheckmarkData.myTexture && Options::fullscreen)
		spriteDrawer.Draw(myFSCheckmarkData, myFSCheckmarkInstance);

	if (showReturnBar)
		spriteDrawer.Draw(myReturnBarSpriteData, myReturnBarSpriteInstance);

	myBackButton.Render();
	
	myDualStick.text.Render();
}

float OptionsState::GetBarLeft(const Tga::Sprite2DInstanceData& aBarInstance) const
{
	return aBarInstance.myPosition.x - (aBarInstance.mySize.x * 0.5f);
}

float OptionsState::GetBarRight(const Tga::Sprite2DInstanceData& aBarInstance) const
{
	return GetBarLeft(aBarInstance) + aBarInstance.mySize.x;
}

bool OptionsState::IsMouseOverBar(const Tga::Vector2f& aMousePos, const Tga::Sprite2DInstanceData& aBarInstance) const
{
	const float barLeft = GetBarLeft(aBarInstance);
	const float barRight = GetBarRight(aBarInstance);
	const float barTop = aBarInstance.myPosition.y + (aBarInstance.mySize.y * 0.5f);
	const float barBottom = aBarInstance.myPosition.y - (aBarInstance.mySize.y * 0.5f);

	return (aMousePos.x >= barLeft && aMousePos.x <= barRight) &&
		(aMousePos.y >= barBottom && aMousePos.y <= barTop);
}

void OptionsState::UpdateSliderKnobPosition()
{
	//master knob
	{
		const int minV = Options::minVolume;
		const int maxV = Options::maxVolume;
		const int value = Options::masterVolume;

		const float t = (maxV > minV) ? (static_cast<float>(value - minV) / static_cast<float>(maxV - minV)) : 0.0f;

		const Tga::Vector2f texSize{ myMasterSliderBarData.myTexture->CalculateTextureSize() };
		const float worldPerPx = (texSize.x > 0.0f) ? (myMasterSliderBarInstance.mySize.x / texSize.x) : 0.0f;

		const float barLeft = GetBarLeft(myMasterSliderBarInstance);
		const float barRight = GetBarRight(myMasterSliderBarInstance);

		const float trackLeft = barLeft + UI.sliderTrackPaddingLeftPx * worldPerPx;
		const float trackRight = barRight - UI.sliderTrackPaddingRightPx * worldPerPx;

		float safeTrackLeft = trackLeft;
		float safeTrackRight = trackRight;
		if (safeTrackRight <= safeTrackLeft)
		{
			safeTrackLeft = barLeft;
			safeTrackRight = barRight;
		}

		const float trackWidth = std::max(0.0f, trackRight - trackLeft);

		const float knobX = safeTrackLeft + t * trackWidth;
		const float knobY = myMasterSliderBarInstance.myPosition.y;

		myMasterSliderKnobInstance.myPosition = { knobX, knobY };
	}

	//music knob
	if constexpr (kEnableMusicSlider)
	{
		const int minV = Options::minVolume;
		const int maxV = Options::maxVolume;
		const int value = Options::musicVolume;

		const float t = (maxV > minV) ? (static_cast<float>(value - minV) / static_cast<float>(maxV - minV)) : 0.0f;

		const Tga::Vector2f texSize{ myMusicSliderBarData.myTexture->CalculateTextureSize() };
		const float worldPerPx = (texSize.x > 0.0f) ? (myMusicSliderBarInstance.mySize.x / texSize.x) : 0.0f;

		const float barLeft = GetBarLeft(myMusicSliderBarInstance);
		const float barRight = GetBarRight(myMusicSliderBarInstance);

		const float trackLeft = barLeft + UI.sliderTrackPaddingLeftPx * worldPerPx;
		const float trackRight = barRight - UI.sliderTrackPaddingRightPx * worldPerPx;

		float safeTrackLeft = trackLeft;
		float safeTrackRight = trackRight;
		if (safeTrackRight <= safeTrackLeft)
		{
			safeTrackLeft = barLeft;
			safeTrackRight = barRight;
		}

		const float trackWidth = std::max(0.0f, trackRight - trackLeft);

		const float knobX = safeTrackLeft + t * trackWidth;
		const float knobY = myMusicSliderBarInstance.myPosition.y;

		myMusicSliderKnobInstance.myPosition = { knobX, knobY };
	}

	MenuLayoutHelper::SyncOutlineToTarget(myMasterSliderKnobInstance, myMasterSliderOutlineInstance, UI.handleOutlineSizeMultiplier);

	if constexpr (kEnableMusicSlider)
	{
		MenuLayoutHelper::SyncOutlineToTarget(myMusicSliderKnobInstance, myMusicSliderOutlineInstance, UI.handleOutlineSizeMultiplier);
	}
}

void OptionsState::ChangeVolumeBy(int delta, int sliderIndex)
{
	if (sliderIndex == 0 && isMuted)
		return;

	if (sliderIndex == 1 && isMuted)
		return;

	if (sliderIndex == 0)
	{
		Options::masterVolume += delta;
		if (Options::masterVolume < Options::minVolume) Options::masterVolume = Options::minVolume;
		if (Options::masterVolume > Options::maxVolume) Options::masterVolume = Options::maxVolume;

		myPrevMasterVolume = Options::masterVolume;
	}
	else
	{
		Options::musicVolume += delta;
		if (Options::musicVolume < Options::minVolume) Options::musicVolume = Options::minVolume;
		if (Options::musicVolume > Options::maxVolume) Options::musicVolume = Options::maxVolume;

		myPrevMusicVolume = Options::musicVolume;
	}

	UpdateSliderKnobPosition();

	if (sliderIndex == 0)
	{
		std::cout << "Master volume: " << Options::masterVolume << "%" << std::endl;
	}
	else
	{
		std::cout << "Music volume: " << Options::musicVolume << "%" << std::endl;
	}
}

void OptionsState::ApplyContinuousVolumeMove(float aUiDt, int aDir, float aUnitsPerSeconds, int aSliderIndex)
{
	if (aDir == 0)
		return;

	if (aSliderIndex == 0 && isMuted)
		return;

	if (aSliderIndex == 1 && isMuted)
		return;

	static float sMasterRemainder = 0.0f;
	static float sMusicRemainder = 0.0f;

	float& remainder = (aSliderIndex == 0) ? sMasterRemainder : sMusicRemainder;

	remainder += static_cast<float>(aDir) * aUnitsPerSeconds * aUiDt;

	const int deltaInt = static_cast<int>(remainder);
	if (deltaInt != 0)
	{
		remainder -= static_cast<float>(deltaInt);
		ChangeVolumeBy(deltaInt, aSliderIndex);
	}
}

void OptionsState::SetVolumeFromMouseX(float aMouseX, int sliderIndex)
{
	if (sliderIndex == 0 && isMuted)
		return;

	if (sliderIndex == 1 && isMuted)
		return;

	const Tga::Sprite2DInstanceData& aBarInstance = (sliderIndex == 0) ? myMasterSliderBarInstance : myMusicSliderBarInstance;
	const int minV = Options::minVolume;
	const int maxV = Options::maxVolume;

	const Tga::Vector2f texSize{ myMasterSliderBarData.myTexture->CalculateTextureSize() };
	const float worldPerPx = (texSize.x > 0.0f) ? (aBarInstance.mySize.x / texSize.x) : 0.0f;

	const float barLeft = GetBarLeft(aBarInstance);
	const float barRight = GetBarRight(aBarInstance);

	const float trackLeft = barLeft + UI.sliderTrackPaddingLeftPx * worldPerPx;
	const float trackRight = barRight - UI.sliderTrackPaddingRightPx * worldPerPx;

	float safeTrackLeft = trackLeft;
	float safeTrackRight = trackRight;
	if (safeTrackRight <= safeTrackLeft)
	{
		safeTrackLeft = barLeft;
		safeTrackRight = barRight;
	}

	const float trackWidth = std::max(0.0f, safeTrackRight - safeTrackLeft);
	const float t = (trackWidth > 0.0f) ? ((aMouseX - safeTrackLeft) / trackWidth) : 0.0f;
	const float clampedT = std::clamp(t, 0.0f, 1.0f);

	const int newValue = static_cast<int>(clampedT * static_cast<float>(maxV - minV) + 0.5f) + minV;

	if (sliderIndex == 0)
	{
		if (newValue != Options::masterVolume)
		{
			Options::masterVolume = newValue;

			if (isMuted)
			{
				isMuted = false;
			}
			myPrevMasterVolume = Options::masterVolume;

			UpdateSliderKnobPosition();
			std::cout << "Master volume: " << Options::masterVolume << "%" << std::endl;
		}
	}
	else
	{
		if (newValue != Options::musicVolume)
		{
			Options::musicVolume = newValue;

			if (isMuted)
			{
				isMuted = false;
			}
			myPrevMusicVolume = Options::musicVolume;

			UpdateSliderKnobPosition();
			std::cout << "Music volume: " << Options::musicVolume << "%" << std::endl;
		}
	}
}

void OptionsState::ToggleMute()
{
	AudioManager::GetAudioPoolByHandle(AudioHandles::clickButton).Play();
	if (!isMuted)
	{
		myPrevMasterVolume = Options::masterVolume;
		myPrevMusicVolume = Options::musicVolume;
		Options::masterVolume = 0;
		Options::musicVolume = 0;
		isMuted = true;
	}
	else
	{
		isMuted = false;
		Options::masterVolume = std::clamp(myPrevMasterVolume, Options::minVolume, Options::maxVolume);
		Options::musicVolume = std::clamp(myPrevMusicVolume, Options::minVolume, Options::maxVolume);
	}

	UpdateSliderKnobPosition();

	std::cout << "Master volume: " << Options::masterVolume << "%" << (isMuted ? " (muted)" : "") << std::endl;
	std::cout << "Music volume: " << Options::musicVolume << "%" << (isMuted ? " (muted)" : "") << std::endl;
}

void OptionsState::ToggleFullscreen()
{
	AudioManager::GetAudioPoolByHandle(AudioHandles::clickButton).Play();
	Options::fullscreen = !Options::fullscreen;

	auto& engine = *Tga::Engine::GetInstance();

	engine.SetFullScreen(Options::fullscreen);

	std::cout << "Fullscreen mode: " << (Options::fullscreen ? "ON" : "OFF") << std::endl;
}