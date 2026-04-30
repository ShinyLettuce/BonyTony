#include "GameWorld.h"

#include <Xinput.h>
#include <tge/graphics/GraphicsEngine.h>
#include <tge/drawers/SpriteDrawer.h>
#include <tge/drawers/ModelDrawer.h>
#include <tge/texture/TextureManager.h>
#include <tge/drawers/DebugDrawer.h>
#include <tge/engine.h>
#include <tge/scene/Scene.h>
#include <tge/scene/SceneSerialize.h>
#include <tge/scene/SceneObjectDefinitionManager.h>
#include <tge/scene/ScenePropertyTypes.h>
#include <tge/script/BaseProperties.h>
#include <tge/settings/settings.h>
#include <tge/graphics/GraphicsStateStack.h>

#include <imgui/imgui.h>

#include "SceneLoader.h"

GameWorld::GameWorld()
{
}

GameWorld::~GameWorld()
{
}

void GameWorld::Init(InputMapper* aInputMapper, Timer* aTimer)
{
	myTimer = aTimer;
	myInputMapper = aInputMapper;
	
	myCrosshairData.myTexture = Tga::Engine::GetInstance()->GetTextureManager().GetTexture("textures/UI/HUD/T_Crosshair_C.dds");
	myCrosshairInstance.myPivot = { 0.5, 0.5f };
	myCrosshairInstance.mySize = Tga::Vector2f{ myCrosshairData.myTexture->CalculateTextureSize() } * ResolutionManager::GetUIScale();
	myCrosshairInstance.myPosition = { myInputMapper->GetMousePositionYUp() };
	
	BindInputs();
	InitAudioHandles();
	InitStateStackAndStates();
}

void GameWorld::Update()
{
#if !defined(_RETAIL)
	if (myInputMapper->IsActionJustActivated(GameAction::DebugReleaseCursor))
	{
		myInputMapper->ReleaseMouse();
	}
	if (myInputMapper->IsActionJustActivated(GameAction::DebugCaptureCursor))
	{
		myInputMapper->CaptureMouse();
	}
	
	ImGui::Begin("Options");
	
	ImGui::Checkbox("Enable DualStick: ", &Options::enableDualStick);
	ImGui::Checkbox("Shotgun on RS: ", &Options::shotgunOnRS);
	
	ImGui::End();
	
#endif
	
	myCrosshairInstance.myPosition = { myInputMapper->GetMousePositionYUp() };
	
	myStateStack.Update();
}

void GameWorld::Render()
{
	myStateStack.Render();
	
	Tga::SpriteDrawer& spriteDrawer = Tga::Engine::GetInstance()->GetGraphicsEngine().GetSpriteDrawer();
	myInputMapper->RenderCursorSprite(spriteDrawer, myCrosshairData, myCrosshairInstance);
}

void GameWorld::BindInputs()
{
	myInputMapper->BindKeyAction(VK_RETURN, GameAction::SkipCutscene);
	myInputMapper->BindGamePadButtonAction(XINPUT_GAMEPAD_START, GameAction::SkipCutscene);
	
	/*-*-*-*-*-*-DEBUG*-*-*-*-*-*-*-*-*/
#if !defined(_RETAIL)
	myInputMapper->BindKeyAction('R', GameAction::DebugReload);
	myInputMapper->BindKeyAction('O', GameAction::DebugCaptureCursor);
	myInputMapper->BindKeyAction('P', GameAction::DebugReleaseCursor);
#endif
	
	/*-*-*-*-*-*-UI*-*-*-*-*-*-*-*-*/
	
	//keyboard
	myInputMapper->BindKeyAction('W', GameAction::UIUp);
	myInputMapper->BindKeyAction('S', GameAction::UIDown);
	myInputMapper->BindKeyAction('A', GameAction::UILeft);
	myInputMapper->BindKeyAction('D', GameAction::UIRight);
	
	myInputMapper->BindKeyAction(VK_UP, GameAction::UIUp);
	myInputMapper->BindKeyAction(VK_DOWN, GameAction::UIDown);
	myInputMapper->BindKeyAction(VK_LEFT, GameAction::UILeft);
	myInputMapper->BindKeyAction(VK_RIGHT, GameAction::UIRight);
	
	myInputMapper->BindKeyAction(VK_RETURN, GameAction::UIConfirm);
	myInputMapper->BindKeyAction(VK_ESCAPE, GameAction::UICancel);
	
	myInputMapper->BindKeyAction(VK_ESCAPE, GameAction::Pause);
	myInputMapper->BindKeyAction(VK_LBUTTON, GameAction::UILeftClick);
	
	//gamepad
	
	myInputMapper->BindGamePadButtonAction(XINPUT_GAMEPAD_DPAD_UP, GameAction::UIUp);
	myInputMapper->BindGamePadButtonAction(XINPUT_GAMEPAD_DPAD_DOWN, GameAction::UIDown);
	myInputMapper->BindGamePadButtonAction(XINPUT_GAMEPAD_DPAD_LEFT, GameAction::UILeft);
	myInputMapper->BindGamePadButtonAction(XINPUT_GAMEPAD_DPAD_RIGHT, GameAction::UIRight);
	
	myInputMapper->BindGamePadAnalogAction(GamePadAnalogInput::LSUp, GameAction::UIUp);
	myInputMapper->BindGamePadAnalogAction(GamePadAnalogInput::LSDown, GameAction::UIDown);
	myInputMapper->BindGamePadAnalogAction(GamePadAnalogInput::LSLeft, GameAction::UILeft);
	myInputMapper->BindGamePadAnalogAction(GamePadAnalogInput::LSRight, GameAction::UIRight);
	
	myInputMapper->BindGamePadButtonAction(XINPUT_GAMEPAD_A, GameAction::UIConfirm);
	myInputMapper->BindGamePadButtonAction(XINPUT_GAMEPAD_B, GameAction::UICancel);
	
	myInputMapper->BindGamePadButtonAction(XINPUT_GAMEPAD_START, GameAction::Pause);
	
	myInputMapper->BindGamePadButtonAction(XINPUT_GAMEPAD_LEFT_THUMB, GameAction::LSPress);
	myInputMapper->BindGamePadButtonAction(XINPUT_GAMEPAD_RIGHT_THUMB, GameAction::RSPress);
	
	/*-*-*-*-*-*-GAMEPLAY*-*-*-*-*-*-*-*-*/
	
	//keyboard
	myInputMapper->BindKeyAction(VK_LBUTTON, GameAction::PlayerShootShotgun);
	myInputMapper->BindKeyAction(VK_SHIFT, GameAction::PlayerPowerShotOverride);
	myInputMapper->BindKeyAction(VK_RBUTTON, GameAction::PlayerShootRevolver);
	
	//gamepad
	myInputMapper->BindGamePadAnalogAction(GamePadAnalogInput::RTrigger, GameAction::PlayerShootShotgun);
	myInputMapper->BindGamePadButtonAction(XINPUT_GAMEPAD_RIGHT_SHOULDER, GameAction::PlayerShootShotgun);
	myInputMapper->BindGamePadButtonAction(XINPUT_GAMEPAD_RIGHT_SHOULDER, GameAction::PlayerPowerShotOverride);
	myInputMapper->BindGamePadAnalogAction(GamePadAnalogInput::LTrigger, GameAction::PlayerShootRevolver);

	myInputMapper->BindGamePadButtonAction(XINPUT_GAMEPAD_A, GameAction::PlayerShootShotgun);
	myInputMapper->BindGamePadButtonAction(XINPUT_GAMEPAD_X, GameAction::PlayerShootShotgun);
	myInputMapper->BindGamePadButtonAction(XINPUT_GAMEPAD_X, GameAction::PlayerPowerShotOverride);
	myInputMapper->BindGamePadButtonAction(XINPUT_GAMEPAD_B, GameAction::PlayerShootRevolver);
}

void GameWorld::InitStateStackAndStates()
{
	const StateHandle splashScreenStateHandle = myStateStack.MakeRegistryHandle();
	const StateHandle mainMenuStateHandle = myStateStack.MakeRegistryHandle();
	const StateHandle bossRoomStateHandle = myStateStack.MakeRegistryHandle();
	const StateHandle gameStateHandle = myStateStack.MakeRegistryHandle();
	const StateHandle popUpStateHandle = myStateStack.MakeRegistryHandle();
	const StateHandle pauseStateHandle = myStateStack.MakeRegistryHandle();
	const StateHandle levelSelectStateHandle = myStateStack.MakeRegistryHandle();
	const StateHandle optionsStateHandle = myStateStack.MakeRegistryHandle();
	const StateHandle creditsStateHandle = myStateStack.MakeRegistryHandle();
	const StateHandle cutsceneStateHandle = myStateStack.MakeRegistryHandle();
	const StateHandle quitStateHandle = myStateStack.MakeRegistryHandle();
	
	myStateStack.RegisterState(splashScreenStateHandle, &mySplashScreenState);
	myStateStack.RegisterState(mainMenuStateHandle, &myMainMenuState);
	myStateStack.RegisterState(bossRoomStateHandle, &myBossRoomState);
	myStateStack.RegisterState(gameStateHandle, &myGameState);
	myStateStack.RegisterState(popUpStateHandle, &myPopUpState);
	myStateStack.RegisterState(pauseStateHandle, &myPauseState);
	myStateStack.RegisterState(levelSelectStateHandle, &myLevelSelectState);
	myStateStack.RegisterState(optionsStateHandle, &myOptionsState);
	myStateStack.RegisterState(creditsStateHandle, &myCreditsState);
	myStateStack.RegisterState(cutsceneStateHandle, &myCutsceneState);
	myStateStack.RegisterState(quitStateHandle, &myQuitState);

	const SplashScreenStateHandles splashScreenStateHandles
	{
		.mainMenuState = mainMenuStateHandle
	};

	const MainMenuStateHandles mainMenuStateHandles
	{
		.gameState = gameStateHandle,
		.levelSelectState = levelSelectStateHandle,
		.optionsState = optionsStateHandle,
		.cutsceneState = cutsceneStateHandle,
		.creditsState = creditsStateHandle,
		.quitState = quitStateHandle 
	};
	const PauseMenuStateHandles pauseMenuStateHandles
	{
		.menuState = mainMenuStateHandle,
		.optionsState = optionsStateHandle,
		.quitState = quitStateHandle,
	};
	const GameStateHandles gameStateHandles
	{
		.pauseState = pauseStateHandle,
		.cutsceneState = cutsceneStateHandle,
		.gameState = gameStateHandle,
		.bossRoomState = bossRoomStateHandle,
		.popUpState = popUpStateHandle,
	};
	const CutsceneStateHandles cutsceneStateHandles
	{
		.pauseState = pauseStateHandle,
		.gameState = gameStateHandle
	};

	mySplashScreenState.Init(splashScreenStateHandles, myTimer);
	myMainMenuState.Init(mainMenuStateHandles, myInputMapper);
	myBossRoomState.Init(mainMenuStateHandle, cutsceneStateHandle, myInputMapper, myTimer);
	myGameState.Init(gameStateHandles, myInputMapper, myTimer);
	myPopUpState.Init(myTimer);
	myPauseState.Init(myInputMapper, pauseMenuStateHandles);
	myLevelSelectState.Init(gameStateHandle, bossRoomStateHandle, myInputMapper);
	myOptionsState.Init(myInputMapper, myTimer);
	myCreditsState.Init(myInputMapper);
	myCutsceneState.Init(cutsceneStateHandles, myInputMapper, myTimer); 
	myQuitState.Init(myInputMapper);
	
#if defined(_RETAIL)
	myStateStack.Initialize(splashScreenStateHandle);
#else
	myStateStack.Initialize(mainMenuStateHandle);
#endif
}

void GameWorld::InitAudioHandles()
{
	AudioHandles::shotgunShot = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& shotgunShotPool = AudioManager::GetAudioPoolByHandle(AudioHandles::shotgunShot);
	shotgunShotPool.AddClip("Audio/SFX/SFX_Weapon_Shotgun_Shot_1.wav"); 
	shotgunShotPool.AddClip("Audio/SFX/SFX_Weapon_Shotgun_Shot_2.wav");
	shotgunShotPool.AddClip("Audio/SFX/SFX_Weapon_Shotgun_Shot_3.wav");
	shotgunShotPool.AddClip("Audio/SFX/SFX_Weapon_Shotgun_Shot_4.wav");
	shotgunShotPool.SetRepeatBuffer(2);
	
    AudioHandles::reload = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& reloadPool = AudioManager::GetAudioPoolByHandle(AudioHandles::reload);
	reloadPool.AddClip("Audio/SFX/SFX_Weapon_Shotgun_Reload_1.wav");
	reloadPool.AddClip("Audio/SFX/SFX_Weapon_Shotgun_Reload_2.wav");
	reloadPool.AddClip("Audio/SFX/SFX_Weapon_Shotgun_Reload_3.wav");
	reloadPool.AddClip("Audio/SFX/SFX_Weapon_Shotgun_Reload_4.wav");
	reloadPool.SetRepeatBuffer(2);
	
    AudioHandles::shotgunShotNoAmmo = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& shotgunShotNoAmmoPool = AudioManager::GetAudioPoolByHandle(AudioHandles::shotgunShotNoAmmo);
	shotgunShotNoAmmoPool.AddClip("Audio/SFX/SFX_Weapon_Shotgun_No_Bullets_1.wav");
	shotgunShotNoAmmoPool.AddClip("Audio/SFX/SFX_Weapon_Shotgun_No_Bullets_2.wav");
	shotgunShotNoAmmoPool.AddClip("Audio/SFX/SFX_Weapon_Shotgun_No_Bullets_3.wav");
	shotgunShotNoAmmoPool.AddClip("Audio/SFX/SFX_Weapon_Shotgun_No_Bullets_4.wav");
	shotgunShotNoAmmoPool.SetRepeatBuffer(2);
	
    AudioHandles::revolverShot = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& revolverShotPool = AudioManager::GetAudioPoolByHandle(AudioHandles::revolverShot);
	revolverShotPool.AddClip("Audio/SFX/SFX_Weapon_Revolver_Shot_1.wav");
	revolverShotPool.AddClip("Audio/SFX/SFX_Weapon_Revolver_Shot_2.wav");
	revolverShotPool.AddClip("Audio/SFX/SFX_Weapon_Revolver_Shot_3.wav");
	revolverShotPool.AddClip("Audio/SFX/SFX_Weapon_Revolver_Shot_4.wav");
	revolverShotPool.SetRepeatBuffer(2);
	
    AudioHandles::revolverShotNoAmmo = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& revolverShotNoAmmoPool = AudioManager::GetAudioPoolByHandle(AudioHandles::revolverShotNoAmmo);
	revolverShotNoAmmoPool.AddClip("Audio/SFX/SFX_Weapon_Revolver_No_Bullets_1.wav");
	revolverShotNoAmmoPool.AddClip("Audio/SFX/SFX_Weapon_Revolver_No_Bullets_2.wav");
	revolverShotNoAmmoPool.AddClip("Audio/SFX/SFX_Weapon_Revolver_No_Bullets_3.wav");
	revolverShotNoAmmoPool.AddClip("Audio/SFX/SFX_Weapon_Revolver_No_Bullets_4.wav");
	revolverShotNoAmmoPool.SetRepeatBuffer(2);
	
    AudioHandles::batSwingAndHit = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& batSwingAndHitPool = AudioManager::GetAudioPoolByHandle(AudioHandles::batSwingAndHit);
	batSwingAndHitPool.AddClip("Audio/SFX/SFX_Enemy_Weapon_Bat_Swing_1.wav");
	batSwingAndHitPool.AddClip("Audio/SFX/SFX_Enemy_Weapon_Bat_Swing_2.wav");
	batSwingAndHitPool.AddClip("Audio/SFX/SFX_Enemy_Weapon_Bat_Swing_3.wav");
	batSwingAndHitPool.AddClip("Audio/SFX/SFX_Enemy_Weapon_Bat_Swing_4.wav");
	batSwingAndHitPool.SetRepeatBuffer(2);
	
	AudioHandles::pistolWhipAndHit = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& pistolWhipAndHitPool = AudioManager::GetAudioPoolByHandle(AudioHandles::pistolWhipAndHit);
	pistolWhipAndHitPool.AddClip("Audio/SFX/SFX_Enemy_Weapon_Revolver_Swing_1.wav");
	pistolWhipAndHitPool.AddClip("Audio/SFX/SFX_Enemy_Weapon_Revolver_Swing_2.wav");
	pistolWhipAndHitPool.AddClip("Audio/SFX/SFX_Enemy_Weapon_Revolver_Swing_3.wav");
	pistolWhipAndHitPool.AddClip("Audio/SFX/SFX_Enemy_Weapon_Revolver_Swing_4.wav");
	pistolWhipAndHitPool.SetRepeatBuffer(2);
	
	AudioHandles::woodenCrateDestroyed = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& woodenCrateDestroyedPool = AudioManager::GetAudioPoolByHandle(AudioHandles::woodenCrateDestroyed);
	woodenCrateDestroyedPool.AddClip("Audio/SFX/SFX_Wood_Crate_Destruction_1.wav");
	woodenCrateDestroyedPool.AddClip("Audio/SFX/SFX_Wood_Crate_Destruction_2.wav");
	woodenCrateDestroyedPool.AddClip("Audio/SFX/SFX_Wood_Crate_Destruction_3.wav");
	woodenCrateDestroyedPool.AddClip("Audio/SFX/SFX_Wood_Crate_Destruction_4.wav");
	woodenCrateDestroyedPool.SetRepeatBuffer(2);
	
	AudioHandles::bulletproofGlassDestroyed = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& bulletproofGlassDestroyedPool = AudioManager::GetAudioPoolByHandle(AudioHandles::bulletproofGlassDestroyed);
	bulletproofGlassDestroyedPool.AddClip("Audio/SFX/SFX_BulletProofGlass_Crate_Destruction_1.wav");
	bulletproofGlassDestroyedPool.AddClip("Audio/SFX/SFX_BulletProofGlass_Crate_Destruction_2.wav");
	bulletproofGlassDestroyedPool.AddClip("Audio/SFX/SFX_BulletProofGlass_Crate_Destruction_3.wav");
	bulletproofGlassDestroyedPool.AddClip("Audio/SFX/SFX_BulletProofGlass_Crate_Destruction_4.wav");
	bulletproofGlassDestroyedPool.SetRepeatBuffer(2);
	
	AudioHandles::shotGunPowerShot = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& shotGunPowerShotPool = AudioManager::GetAudioPoolByHandle(AudioHandles::shotGunPowerShot);
	shotGunPowerShotPool.AddClip("Audio/SFX/SFX_Weapon_Shotgun_Power_Shot_1.wav");
	shotGunPowerShotPool.AddClip("Audio/SFX/SFX_Weapon_Shotgun_Power_Shot_2.wav");
	shotGunPowerShotPool.AddClip("Audio/SFX/SFX_Weapon_Shotgun_Power_Shot_3.wav");
	shotGunPowerShotPool.AddClip("Audio/SFX/SFX_Weapon_Shotgun_Power_Shot_4.wav");
	shotGunPowerShotPool.SetRepeatBuffer(2);
	
    AudioHandles::playerLandingOnGround = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& playerLandingOnGroundPool = AudioManager::GetAudioPoolByHandle(AudioHandles::playerLandingOnGround);
	playerLandingOnGroundPool.AddClip("Audio/SFX/Player_Land-001.wav");
	playerLandingOnGroundPool.AddClip("Audio/SFX/Player_Land-002.wav");
	playerLandingOnGroundPool.AddClip("Audio/SFX/Player_Land-003.wav");
	playerLandingOnGroundPool.AddClip("Audio/SFX/Player_Land-004.wav");
	playerLandingOnGroundPool.SetRepeatBuffer(2);
	
    AudioHandles::playerHurt = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& playerHurtPool = AudioManager::GetAudioPoolByHandle(AudioHandles::playerHurt);
	playerHurtPool.AddClip("Audio/SFX/Player_Shot.wav");
	
    AudioHandles::enemyHurt = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& enemyHurtPool = AudioManager::GetAudioPoolByHandle(AudioHandles::enemyHurt);
	enemyHurtPool.AddClip("Audio/SFX/SFX_Enemy_Death_Rattle_1.wav");
	enemyHurtPool.AddClip("Audio/SFX/SFX_Enemy_Death_Rattle_2.wav");
	enemyHurtPool.AddClip("Audio/SFX/SFX_Enemy_Death_Rattle_3.wav");
	enemyHurtPool.AddClip("Audio/SFX/SFX_Enemy_Death_Rattle_4.wav");
	enemyHurtPool.SetRepeatBuffer(2);
	
    AudioHandles::bulletHittingIronCrate = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& bulletHittingIronCratePool = AudioManager::GetAudioPoolByHandle(AudioHandles::bulletHittingIronCrate);
	bulletHittingIronCratePool.AddClip("Audio/SFX/SFX_Weapon_Bullet_Hit_Metal_1.wav");
	bulletHittingIronCratePool.AddClip("Audio/SFX/SFX_Weapon_Bullet_Hit_Metal_2.wav");
	bulletHittingIronCratePool.AddClip("Audio/SFX/SFX_Weapon_Bullet_Hit_Metal_3.wav");
	bulletHittingIronCratePool.AddClip("Audio/SFX/SFX_Weapon_Bullet_Hit_Metal_4.wav");
	bulletHittingIronCratePool.SetRepeatBuffer(2);
	
	AudioHandles::playerCollideWithWallOrCeiling = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& playerCollideWithWallOrCeilingPool = AudioManager::GetAudioPoolByHandle(AudioHandles::playerCollideWithWallOrCeiling);
	playerCollideWithWallOrCeilingPool.AddClip("");
	playerCollideWithWallOrCeilingPool.AddClip("");
	playerCollideWithWallOrCeilingPool.AddClip("");
	playerCollideWithWallOrCeilingPool.AddClip("");
	playerCollideWithWallOrCeilingPool.SetRepeatBuffer(2);
	
    AudioHandles::bulletHittingWallOrFloor = AudioManager::MakeAudioPool(); //TODO: implement
	AudioManager::AudioPool& bulletHittingWallOrFloorPool = AudioManager::GetAudioPoolByHandle(AudioHandles::bulletHittingWallOrFloor);
	bulletHittingWallOrFloorPool.AddClip("Audio/SFX/SFX_Weapon_Bullet_Hit_Wall&Ground_1.wav");
	bulletHittingWallOrFloorPool.AddClip("Audio/SFX/SFX_Weapon_Bullet_Hit_Wall&Ground_2.wav");
	bulletHittingWallOrFloorPool.AddClip("Audio/SFX/SFX_Weapon_Bullet_Hit_Wall&Ground_3.wav");
	bulletHittingWallOrFloorPool.AddClip("Audio/SFX/SFX_Weapon_Bullet_Hit_Wall&Ground_4.wav");
	bulletHittingWallOrFloorPool.SetRepeatBuffer(2);
	
    AudioHandles::enemyDeathPoofCloud = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& enemyDeathPoofCloudPool = AudioManager::GetAudioPoolByHandle(AudioHandles::enemyDeathPoofCloud);
	enemyDeathPoofCloudPool.AddClip("Audio/SFX/SFX_Enemy_Death_Poof_1.wav");
	enemyDeathPoofCloudPool.AddClip("Audio/SFX/SFX_Enemy_Death_Poof_2.wav");
	enemyDeathPoofCloudPool.AddClip("Audio/SFX/SFX_Enemy_Death_Poof_3.wav");
	enemyDeathPoofCloudPool.AddClip("Audio/SFX/SFX_Enemy_Death_Poof_4.wav");
	enemyDeathPoofCloudPool.SetRepeatBuffer(2);
	
	AudioHandles::enemyReassemble = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& enemyReassemblePool = AudioManager::GetAudioPoolByHandle(AudioHandles::enemyReassemble);
	enemyReassemblePool.AddClip("Audio/SFX/SFX_Enemy_Death_Reanimation_1.wav");
	enemyReassemblePool.AddClip("Audio/SFX/SFX_Enemy_Death_Reanimation_2.wav");
	enemyReassemblePool.AddClip("Audio/SFX/SFX_Enemy_Death_Reanimation_3.wav");
	enemyReassemblePool.AddClip("Audio/SFX/SFX_Enemy_Death_Reanimation_4.wav");
	enemyReassemblePool.SetRepeatBuffer(2);
	
    AudioHandles::hoverButton = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& hoverButtonPool = AudioManager::GetAudioPoolByHandle(AudioHandles::hoverButton);
	hoverButtonPool.AddClip("Audio/SFX/UI_Hover-001.wav");
	hoverButtonPool.AddClip("Audio/SFX/UI_Hover-002.wav");
	hoverButtonPool.AddClip("Audio/SFX/UI_Hover-003.wav");
	hoverButtonPool.AddClip("Audio/SFX/UI_Hover-004.wav");
	hoverButtonPool.SetRepeatBuffer(2);
	
    AudioHandles::clickButton = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& clickButtonPool = AudioManager::GetAudioPoolByHandle(AudioHandles::clickButton);
	clickButtonPool.AddClip("Audio/SFX/UI_Click.wav");
	
	AudioHandles::uiBack = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& uiBackPool = AudioManager::GetAudioPoolByHandle(AudioHandles::uiBack);
	uiBackPool.AddClip("Audio/SFX/UI_Back.wav");
	
    AudioHandles::windowChange = AudioManager::MakeAudioPool(); //TODO: change
	AudioManager::GetAudioPoolByHandle(AudioHandles::windowChange).AddClip("");
	
    AudioHandles::level1Ambience = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& level1AmbiencePool = AudioManager::GetAudioPoolByHandle(AudioHandles::level1Ambience);
	level1AmbiencePool.AddClip("Audio/SFX/AMB_CellarAmbience.wav", true);
	level1AmbiencePool.SetAudioType(AudioManager::AudioType::Music);
	
    AudioHandles::level2Ambience = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& level2AmbiencePool = AudioManager::GetAudioPoolByHandle(AudioHandles::level2Ambience);
	level2AmbiencePool.AddClip("Audio/SFX/AMB_Casino.wav", true);
	level2AmbiencePool.SetAudioType(AudioManager::AudioType::Music);
	
	AudioHandles::elevatorDing = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& elevatorDingPool = AudioManager::GetAudioPoolByHandle(AudioHandles::elevatorDing);
	elevatorDingPool.AddClip("Audio/SFX/SFX_Elevator_Ding.wav");
	
	AudioHandles::elevatorOpen = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& elevatorOpenPool = AudioManager::GetAudioPoolByHandle(AudioHandles::elevatorOpen);
	elevatorOpenPool.AddClip("Audio/SFX/SFX_Elevator_Opening.wav");
	
	AudioHandles::elevatorClose = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& elevatorClosePool = AudioManager::GetAudioPoolByHandle(AudioHandles::elevatorClose);
	elevatorClosePool.AddClip("Audio/SFX/SFX_Elevator_Closing.wav");
	
	AudioHandles::bossDoorOpening = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& bossDoorOpeningPool = AudioManager::GetAudioPoolByHandle(AudioHandles::bossDoorOpening);
	bossDoorOpeningPool.AddClip("Audio/SFX/SFX_Boss_Door_Opening.wav");
	
	AudioHandles::bossRoomAmbience = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& bossRoomAmbiencePool = AudioManager::GetAudioPoolByHandle(AudioHandles::bossRoomAmbience);
	bossRoomAmbiencePool.AddClip("Audio/SFX/AMB_BossRoom.wav", true);
	bossRoomAmbiencePool.SetAudioType(AudioManager::AudioType::Music);
	
	AudioHandles::elevatorAmbienceAndMusic = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& elevatorAmbienceAndMusicPool = AudioManager::GetAudioPoolByHandle(AudioHandles::elevatorAmbienceAndMusic);
	elevatorAmbienceAndMusicPool.AddClip("Audio/SFX/SFX_Elevator_Ambience&Music.wav");
	
	AudioHandles::level1Music = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& level1MusicPool = AudioManager::GetAudioPoolByHandle(AudioHandles::level1Music);
	level1MusicPool.AddClip("Audio/Music/Bony_Tony_Level_1_Mix_3.wav", true);
	level1MusicPool.SetAudioType(AudioManager::AudioType::Music);
	
	AudioHandles::level2IntroMusic = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& level2IntroMusicPool = AudioManager::GetAudioPoolByHandle(AudioHandles::level2IntroMusic);
	level2IntroMusicPool.AddClip("Audio/Music/Bony_Tony_Lvl_2_Music_Intro.wav");
	level2IntroMusicPool.SetAudioType(AudioManager::AudioType::Music);
	
	AudioHandles::level2Music = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& level2MusicPool = AudioManager::GetAudioPoolByHandle(AudioHandles::level2Music);
	level2MusicPool.AddClip("Audio/Music/Bony_Tony_Lvl_2_Music_Loop.wav", true);
	level2MusicPool.SetAudioType(AudioManager::AudioType::Music);
	
	AudioHandles::gibberish = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& gibberishPool = AudioManager::GetAudioPoolByHandle(AudioHandles::gibberish);
	gibberishPool.AddClip("Audio/SFX/SFX_Gibberish_1.wav");
	gibberishPool.AddClip("Audio/SFX/SFX_Gibberish_2.wav");
	gibberishPool.AddClip("Audio/SFX/SFX_Gibberish_3.wav");
	gibberishPool.AddClip("Audio/SFX/SFX_Gibberish_4.wav");
	gibberishPool.AddClip("Audio/SFX/SFX_Gibberish_5.wav");
	gibberishPool.AddClip("Audio/SFX/SFX_Gibberish_6.wav");
	gibberishPool.AddClip("Audio/SFX/SFX_Gibberish_7.wav");
	gibberishPool.AddClip("Audio/SFX/SFX_Gibberish_8.wav");
	gibberishPool.SetRepeatBuffer(2);
	
	AudioHandles::introCutscene = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& introCutscenePool = AudioManager::GetAudioPoolByHandle(AudioHandles::introCutscene);
	introCutscenePool.AddClip("Audio/Bony_Tony_Cutscene_Slutmix_1_WAV.wav");
	
	AudioHandles::endingCutscene = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& endingCutscenePool = AudioManager::GetAudioPoolByHandle(AudioHandles::endingCutscene);
	endingCutscenePool.AddClip("Audio/Ending_WITH SHOTGUN INTRO_SOUND ONLY_WAV.wav");

	AudioHandles::mainMenuMusic = AudioManager::MakeAudioPool();
	AudioManager::AudioPool& mainMenuMusicPool = AudioManager::GetAudioPoolByHandle(AudioHandles::mainMenuMusic);
	mainMenuMusicPool.AddClip("Audio/Bony_Tony_MainMenu_Loop.wav");

}