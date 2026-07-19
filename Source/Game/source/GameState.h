#pragma once

#include "State.h"
#include "Camera.h"
#include "Player.h"
#include "EnemyUpdater.h"
#include "CrateUpdater.h"
#include "PickupUpdater.h"
#include "Timer.h"
#include "InputMapper.h"

#include "Debug/DebugAnimationPlayer.h"

#include <tge/graphics/Camera.h>
#include <tge/drawers/ModelDrawer.h>
#include <tge/texture/TextureManager.h>

#include <iostream>
#include <memory>

#include "FlipbookManager.h"
#include "HUD.h"
#include "LevelTrigger.h"
#include "FullscreenImage.h"


struct GameStateHandles
{
	StateHandle pauseState;
	StateHandle cutsceneState;
	StateHandle gameState;
	StateHandle bossRoomState;
	StateHandle popUpState;
};

class GameState : public State
{
public:
	GameState();

	void Init(GameStateHandles aStateHandle ,InputMapper* aInputMapper, Timer* aTimer);

	void OnPush() override;
	void OnPop() override;
	
	void OnGainFocus() override;

	StateUpdateResult Update() override;
	void Render() override;
	void OnResolutionChange() override;
	Tga::Matrix4x4f GetGunTransform(Tga::Vector2f anAimDir, float aSize, float aPivotOffset, float aYOffset);

private:
	Player myPlayer;
	Camera myCamera;
	HUD myHUD;

	InputMapper* myInputMapper = nullptr;
	Timer* myTimer = nullptr;

	Tga::ModelDrawer myModelDrawer;

	Tga::Vector2f myCameraTargetPosition;

	EnemyUpdater myEnemyUpdater;
	CrateUpdater myCrateUpdater;
	PickupUpdater myPickupUpdater;
	AmbienceManager myAmbienceManager;
	LevelTrigger myLevelTrigger;
	FlipbookManager myFlipbookManager;
	FullscreenImage myFadeInOut;
		
	GameStateHandles myStateHandles;

	DebugAnimationPlayer myDebugAnimationPlayer;

	Tga::SpriteSharedData myFadeOutSharedData;
	Tga::Sprite2DInstanceData myFadeOutInstance;

	int myFrameCount = 0;
		
	float myFadeInTime = 2.f;
	float myFadeOutTime = 1.f;
	float myTransitionSequenceTimer = 0.f;

	int myShotgunMaxClip = 0;
	int myRevolverMaxClip = 0;
	
	bool myIntroMusicFinished = false;
	bool myElevatorDingHasPlayed = false;
	
	bool TransitionSequenceFinished() const;
	
	void UpdateTransitionSequence(SceneLoader::SceneType aSceneType);
};
