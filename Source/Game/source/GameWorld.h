#pragma once

#include <tge/sprite/sprite.h>
#include <tge/graphics/Camera.h>

#include <memory>

#include "CreditsState.h"
#include "PauseState.h"
#include "GameState.h"
#include "InputMapper.h"
#include "Timer.h"
#include "MainMenuState.h"
#include "OptionsState.h"
#include "LevelSelectState.h"
#include "QuitState.h"
#include "CutsceneState.h"
#include "StateStack.h"
#include "BossRoomState.h"
#include "PopUpState.h"
#include "SplashScreenState.h"

class GameWorld
{
public:
	GameWorld(); 
	~GameWorld();

	void Init(InputMapper* aInputMapper, Timer* aTimer);
	void Update();
	void Render();
private:
	void BindInputs();
	void InitStateStackAndStates();
	void InitAudioHandles();
	
	InputMapper* myInputMapper;
	Timer* myTimer;

	StateStack myStateStack;
	
	SplashScreenState mySplashScreenState;
	MainMenuState myMainMenuState;
	BossRoomState myBossRoomState;
	GameState myGameState;
	PopUpState myPopUpState;
	PauseState myPauseState;
	LevelSelectState myLevelSelectState;
	OptionsState myOptionsState;
	CreditsState myCreditsState;
	CutsceneState myCutsceneState;
	QuitState myQuitState;
	
    Tga::SpriteSharedData myCrosshairData;
    Tga::Sprite2DInstanceData myCrosshairInstance;
};
