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
                                                  
struct TonyFlipbookHandles
{
	FlipbookManager::FlipbookHandle tonyFireShotgun;
	FlipbookManager::FlipbookHandle tonyTrailFireShotgun;
	FlipbookManager::FlipbookHandle tonyFirePowershot;
	FlipbookManager::FlipbookHandle tonyTrailFirePowershot; 
	FlipbookManager::FlipbookHandle tonyFireRevolver;

	FlipbookManager::PersistentInstanceHandle tonyFireShotgunInstance;
	FlipbookManager::PersistentInstanceHandle tonyFirePowerShotInstance;
	FlipbookManager::PersistentInstanceHandle tonyFireRevolverInstance;
};

struct EnvironmentFlipbookHandles
{
	FlipbookManager::FlipbookHandle environmentHit;
	FlipbookManager::FlipbookHandle crateHit;
	FlipbookManager::FlipbookHandle metalCrateHit;
	FlipbookManager::FlipbookHandle enemyHit;
};

struct RepeatingFlipbookHandles
{
	FlipbookManager::FlipbookHandle steamEnvironment;
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

	EnemyUpdater myEnemyUpdater;
	CrateUpdater myCrateUpdater;
	PickupUpdater myPickupUpdater;
	AmbienceManager myAmbienceManager;
	LevelTrigger myLevelTrigger;
	FlipbookManager myFlipbookManager;
	TonyFlipbookHandles myTonyFlipbookHandles;
	EnvironmentFlipbookHandles myEnvironmentFlipbookHandles;
	RepeatingFlipbookHandles myLoopingFlipbookHandles;
	FullscreenImage myFadeInOut;
		
	GameStateHandles myStateHandles;

	DebugAnimationPlayer myDebugAnimationPlayer;

	Tga::SpriteSharedData myFadeOutSharedData;
	Tga::Sprite2DInstanceData myFadeOutInstance;

	/*float myFadeOutTimer = 0.f;*/

	int myFrameCount = 0;

	/*bool myFadeInTimerStarted = false;
	float myFadeInTimer = 0.f;*/

		
	float myFadeInTime = 2.f;
	float myFadeOutTime = 1.f;
	float myTransitionSequenceTimer = 0.f;

	int myShotgunMaxClip = 0;
	int myRevolverMaxClip = 0;
	
	bool myIntroMusicFinished = false;
	bool myElevatorDingHasPlayed = false;


	//bool FadeOutTimerFinished() const;
	//void UpdateFadeOut();
	//bool FadeInTimerFinished() const;
	//void UpdateFadeIn();
	
	bool TransitionSequenceFinished() const;
	
	void UpdateTransitionSequence(SceneLoader::SceneType aSceneType);
};
