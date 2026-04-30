#include "GameState.h"

#include "SceneLoader.h"
#include <tge/engine.h>
#include <tge/graphics/GraphicsEngine.h>
#include <tge/graphics/GraphicsStateStack.h>
#include <tge/drawers/LineDrawer.h>
#include "Physics.h"
#include <tge/drawers/DebugDrawer.h>
#include "PhysicsDebugDrawer.h"
#include "Enemy.h"
#include "CrateUpdater.h"
#include <imgui/imgui.h>
#include <tge/graphics/DX11.h>

#include "GroundCheck.h"
#include "PickupCheck.h"
#include "PlayerSweep.h"
#include "RevolverRaycast.h"
#include "ShotgunRaycast.h"
#include "EnemyCollision.h"
#include "ProjectileCollision.h"
#include "MathUtils.h"
#include "Go.h"

#include <vector>

#include "LevelTriggerCheck.h"
#include "Options.h"
#include "PopUpState.h"

GameState::GameState()
{
	Tga::Engine* engine = Tga::Engine::GetInstance();
	myFadeOutSharedData.myTexture = engine->GetTextureManager().GetTexture("textures/UI/Backgrounds/T_FadeOut.png");
}

void GameState::Init(GameStateHandles aStateHandle, InputMapper* aInputMapper, Timer* aTimer)
{
	myInputMapper = aInputMapper;
	myTimer = aTimer;
	myPlayer.SetInput(aInputMapper);
	myStateHandles = aStateHandle;
	myDebugAnimationPlayer.Init();
	myTonyFlipbookHandles.tonyFireShotgun = myFlipbookManager.MakeNewFlipbookHandle();
	myTonyFlipbookHandles.tonyTrailFireShotgun = myFlipbookManager.MakeNewFlipbookHandle();
	myTonyFlipbookHandles.tonyFirePowershot = myFlipbookManager.MakeNewFlipbookHandle();
	myTonyFlipbookHandles.tonyTrailFirePowershot = myFlipbookManager.MakeNewFlipbookHandle();
	myTonyFlipbookHandles.tonyFireRevolver = myFlipbookManager.MakeNewFlipbookHandle();
	myEnvironmentFlipbookHandles.environmentHit = myFlipbookManager.MakeNewFlipbookHandle();
	myEnvironmentFlipbookHandles.crateHit = myFlipbookManager.MakeNewFlipbookHandle();
	myEnvironmentFlipbookHandles.metalCrateHit = myFlipbookManager.MakeNewFlipbookHandle();
	myEnvironmentFlipbookHandles.enemyHit = myFlipbookManager.MakeNewFlipbookHandle();
	myFlipbookManager.RegisterFlipBook(FlipBookPresets::TONY_SHOTGUN_FIRE, myTonyFlipbookHandles.tonyFireShotgun);
	myFlipbookManager.RegisterFlipBook(FlipBookPresets::TONY_SHOTGUN_FIRE_TRAIL, myTonyFlipbookHandles.tonyTrailFireShotgun);
	myFlipbookManager.RegisterFlipBook(FlipBookPresets::TONY_POWERSHOT_FIRE, myTonyFlipbookHandles.tonyFirePowershot);
	myFlipbookManager.RegisterFlipBook(FlipBookPresets::TONY_POWERSHOT_FIRE_TRAIL, myTonyFlipbookHandles.tonyTrailFirePowershot);
	myFlipbookManager.RegisterFlipBook(FlipBookPresets::TONY_REVOLVER_FIRE, myTonyFlipbookHandles.tonyFireRevolver);
	myFlipbookManager.RegisterFlipBook(FlipBookPresets::ENVIRONMENT_HIT, myEnvironmentFlipbookHandles.environmentHit);
	myFlipbookManager.RegisterFlipBook(FlipBookPresets::CRATE_HIT, myEnvironmentFlipbookHandles.crateHit);
	myFlipbookManager.RegisterFlipBook(FlipBookPresets::METAL_CRATE_HIT, myEnvironmentFlipbookHandles.metalCrateHit);
	myFlipbookManager.RegisterFlipBook(FlipBookPresets::ENEMY_HIT, myEnvironmentFlipbookHandles.enemyHit);

	myLoopingFlipbookHandles.steamEnvironment = myFlipbookManager.MakeNewFlipbookHandle();
	myFlipbookManager.RegisterFlipBook(FlipBookPresets::STEAM_ENVIRONMENT, myLoopingFlipbookHandles.steamEnvironment, true);

	myTonyFlipbookHandles.tonyFireShotgunInstance = myFlipbookManager.CreatePersistentInstanceHandle();
	myTonyFlipbookHandles.tonyFirePowerShotInstance = myFlipbookManager.CreatePersistentInstanceHandle();
	myTonyFlipbookHandles.tonyFireRevolverInstance = myFlipbookManager.CreatePersistentInstanceHandle();

	myFlipbookManager.SetPersistentInstanceFlipbook(myTonyFlipbookHandles.tonyFireShotgunInstance, myTonyFlipbookHandles.tonyFireShotgun);
	myFlipbookManager.SetPersistentInstanceFlipbook(myTonyFlipbookHandles.tonyFirePowerShotInstance, myTonyFlipbookHandles.tonyFirePowershot);
	myFlipbookManager.SetPersistentInstanceFlipbook(myTonyFlipbookHandles.tonyFireRevolverInstance, myTonyFlipbookHandles.tonyFireRevolver);
}

void GameState::OnPush()
{
	SceneLoader::SceneConfig& sceneConfig = SceneLoader::GetActiveScene();
	myCamera.Init(myTimer);
	myCamera.SetDepth(sceneConfig.cameraConfig.depth);
	myCamera.SetHeight(sceneConfig.cameraConfig.height);
	myCamera.SetFov(sceneConfig.cameraConfig.fov);
	myModelDrawer.Init();

	myPlayer.Init(sceneConfig.playerConfig);

	myCamera.MoveToPosition(myPlayer.GetPosition());

	myShotgunMaxClip = sceneConfig.playerConfig.shotgunData.maxClip;
	myRevolverMaxClip = sceneConfig.playerConfig.revolverData.maxClip;

	myHUD.Init(sceneConfig.playerConfig.shotgunData.maxClip, sceneConfig.playerConfig.revolverData.maxClip, myPlayer.GetRevolverRange());
	myEnemyUpdater.Init(sceneConfig.enemieyConfigs, &sceneConfig.enemySharedConfig);
	myCrateUpdater.Init(sceneConfig.crateConfigs);
	myPickupUpdater.Init(sceneConfig.pickupConfigs);
	myAmbienceManager.Init(&sceneConfig.ambiences);

	LevelTrigger::AudioSequenceData levelTriggerAudioSequenceData{};

	//TODO: match timings with animations and sounds in level transitions 
	if (sceneConfig.metaConfig.type == SceneLoader::SceneType::Level1)
	{
		levelTriggerAudioSequenceData =
		{
			.audioPoolHandle = AudioHandles::elevatorOpen,
			.bgmFadeStart = 0.1f,
			.bgmFadeDuration = 4.f,
			.timeUntilFadeOut = 3.f,
		};

		AudioManager::GetAudioPoolByHandle(AudioHandles::level1Ambience).Play();
		AudioManager::GetAudioPoolByHandle(AudioHandles::level1Music).Play();
		myIntroMusicFinished = true;
	}
	else if (sceneConfig.metaConfig.type == SceneLoader::SceneType::Level2)
	{
		levelTriggerAudioSequenceData =
		{
			.audioPoolHandle = AudioHandles::bossDoorOpening,
			.bgmFadeStart = 0.1f,
			.bgmFadeDuration = 4.f,
			.timeUntilFadeOut = 3.f,
		};

		AudioManager::GetAudioPoolByHandle(AudioHandles::level2Ambience).Play();
		AudioManager::GetAudioPoolByHandle(AudioHandles::level2IntroMusic).Play();
		myIntroMusicFinished = false;
	}

	myLevelTrigger.Init(sceneConfig.levelTriggerConfig, levelTriggerAudioSequenceData);

	myTransitionSequenceTimer = 0.f;
	myFrameCount = 0;

	myLevelTrigger.SetHasSfxPlayed(false);
	myElevatorDingHasPlayed = false;

	myFadeInOut.Init(FullscreenImageState::Opaque, "textures/UI/Backgrounds/T_FadeOut.png");
	myFadeInOut.StartFadeIn(myFadeInTime);

	myAmbienceManager.UpdateVolume(Options::masterVolume, Options::maxVolume);
	AudioManager::UpdateVolume(Options::masterVolume, Options::musicVolume, Options::maxVolume);

	//myFlipbookManager.PlayAt(myRepeatingFlipbookHandles.steamEnvironment, sceneConfig.steamEffects,  0.02f, 0.f );
}

void GameState::OnPop()
{
	AudioManager::GetAudioPoolByHandle(AudioHandles::level1Ambience).Stop();
	AudioManager::GetAudioPoolByHandle(AudioHandles::level1Music).Stop();
	AudioManager::GetAudioPoolByHandle(AudioHandles::level2Ambience).Stop();
	AudioManager::GetAudioPoolByHandle(AudioHandles::level2Music).Stop();
	AudioManager::GetAudioPoolByHandle(AudioHandles::level2IntroMusic).Stop();

	myFlipbookManager.RemoveAllLoopingInstances();
}

void GameState::OnGainFocus()
{
	SetMouseCaptureEnabled(true);
}

void GameState::OnResolutionChange()
{
	myHUD.PositionElements(myShotgunMaxClip, myRevolverMaxClip);
}

Tga::Matrix4x4f GameState::GetGunTransform(Tga::Vector2f anAimDir, float aSize, float aPivotOffset, float aYOffset)
{
	Tga::Vector2f forward{ 1.f, 0.f };
	float revolverAngleRad = std::atan2f(forward.Cross(anAimDir), forward.Dot(anAimDir));
	float revolverAngleDeg = revolverAngleRad * (180.f / FMath::Pi);

	const float pivotX = myPlayer.GetPosition().x + aPivotOffset * std::cos(revolverAngleRad);
	const float pivotY = myPlayer.GetShotOrigin().y + aPivotOffset * std::sin(revolverAngleRad);

	Tga::Matrix4x4f S = Tga::Matrix4x4f::CreateFromScale({ aSize, aSize, 1.f });
	Tga::Matrix4x4f toPivot = Tga::Matrix4x4f::CreateFromTranslation({ (aSize * 0.5f), (-aSize * 0.5f) * 0.01f + aYOffset, 0.f });
	Tga::Matrix4x4f R = Tga::Matrix4x4f::CreateRotationAroundZ(revolverAngleDeg);
	Tga::Matrix4x4f T = Tga::Matrix4x4f::CreateFromTranslation({ pivotX, pivotY, 0.f });

	return S * toPivot * R * T;
}

StateUpdateResult GameState::Update()
{
	SceneLoader::SceneConfig& sceneConfig = SceneLoader::GetActiveScene();

	if (!myIntroMusicFinished)
	{
		if (!AudioManager::GetAudioPoolByHandle(AudioHandles::level2IntroMusic).IsAudioPlaying())
		{
			AudioManager::GetAudioPoolByHandle(AudioHandles::level2Music).Play();
			myIntroMusicFinished = true;
		}
	}

	if (myInputMapper->IsActionJustActivated(GameAction::Pause))
	{
		std::cout << "[GameState.cpp] Pause" << '\n';
		return StateUpdateResult::CreatePush(myStateHandles.pauseState);
	}

	if (myInputMapper->IsActionJustActivated(GameAction::SkipCutscene))
	{
		std::cout << "[GameState.cpp] PopUp" << '\n';
		return StateUpdateResult::CreatePush(myStateHandles.popUpState);
	}

	const float deltaTime = myTimer->GetDeltaTime();

	static float cameraFollowDecay = 2.0f;
	static float cameraHorizontalFollowCoefficient = 0.35f;
	static float cameraDepth = SceneLoader::GetActiveScene().cameraConfig.depth;

#if !defined(_RETAIL)
	ImGui::Begin("Camera");
	ImGui::DragFloat("Camera follow decay", &cameraFollowDecay);
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::BeginItemTooltip())
	{
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted("How fast the camera moves towards the target position");
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
	ImGui::DragFloat("Camera horizontal follow coefficient", &cameraHorizontalFollowCoefficient);
	ImGui::DragFloat("Camera Depth", &cameraDepth);
	ImGui::End();
	myCamera.SetDepth(cameraDepth);
#endif

	Tga::Vector2f playerPosition = myPlayer.GetPosition();
	playerPosition.x *= cameraHorizontalFollowCoefficient;
	myCamera.MoveTowardsPosition(playerPosition, cameraFollowDecay, deltaTime);
	myCamera.Update();

	myEnemyUpdater.Update(myTimer->GetDeltaTime(), myPlayer.GetPosition());
	myAmbienceManager.Update(myPlayer.GetPosition());
	myCrateUpdater.Update(myTimer->GetDeltaTime());

	const SceneLoader::PickupType nextPickupType = myPickupUpdater.Update(myPlayer);
	PopUp::locNextPopupType = nextPickupType;
	if (nextPickupType == SceneLoader::PickupType::Revolver && sceneConfig.metaConfig.type == SceneLoader::SceneType::Level1)
	{
		return StateUpdateResult::CreatePush(myStateHandles.popUpState);
	}
	else if (nextPickupType == SceneLoader::PickupType::PowerShot && sceneConfig.metaConfig.type == SceneLoader::SceneType::Level2)
	{
		return StateUpdateResult::CreatePush(myStateHandles.popUpState);
	}

	PlayerUpdateResult playerUpdateResult = myPlayer.Update(deltaTime, myCamera);
	SceneLoader::SceneConfig scene = SceneLoader::GetActiveScene();

#if !defined(_RETAIL)
	myDebugAnimationPlayer.Update();
	myCamera.DrawScreenToWorldDebugGizmos(myInputMapper->GetMousePositionYUp());
#endif

	{ // -----------Collision checks-----------------
		std::vector<CrateUpdater::Crate>& crates = myCrateUpdater.GetCrates();
		std::vector<Enemy>& enemies = *myEnemyUpdater.GetEnemies();
		std::vector<Projectile>* projectiles = myEnemyUpdater.GetProjectiles();

		GameStateUpdate::PlayerGroundCheck(crates, myPlayer, playerUpdateResult, scene.tileConfigs);

		int iterations = 5;
		float tickrate = 1 / static_cast<float>(iterations);
		for (int i = 0; i < iterations; ++i)
		{
			GameStateUpdate::PlayerSweep(scene.tileConfigs, crates, myCrateUpdater, myPlayer, playerUpdateResult,
				&myFlipbookManager, myEnvironmentFlipbookHandles.metalCrateHit, deltaTime, tickrate);
		}

		if (playerUpdateResult.action == PlayerUpdateResult::Action::Revolver && myPlayer.GetIsRevolverEnabled())
		{
			myFlipbookManager.PlayPersistent(myTonyFlipbookHandles.tonyFireRevolverInstance, 0.007f);
			GameStateUpdate::RevolverRaycast(scene.tileConfigs, enemies, crates, myCrateUpdater, myPlayer,
				&myFlipbookManager, myEnvironmentFlipbookHandles.environmentHit,
				myEnvironmentFlipbookHandles.crateHit, myEnvironmentFlipbookHandles.metalCrateHit,
				myEnvironmentFlipbookHandles.enemyHit);
		}
		if (playerUpdateResult.action == PlayerUpdateResult::Action::Shotgun || playerUpdateResult.action == PlayerUpdateResult::Action::PowerShot)
		{
			Tga::Vector2f shotgunAimDir = myPlayer.GetNormalizedShotgunAim();
			Tga::Vector2f forward{ 1.f, 0.f };
			const float angle = std::atan2f(forward.Cross(shotgunAimDir), forward.Dot(shotgunAimDir));
			const float trailOffset = 400.f;

			if (playerUpdateResult.action == PlayerUpdateResult::Action::Shotgun)
			{
				myFlipbookManager.PlayPersistent(myTonyFlipbookHandles.tonyFireShotgunInstance, 0.007f);
				myFlipbookManager.PlayAt(myTonyFlipbookHandles.tonyTrailFireShotgun, myPlayer.GetShotgunPosition() + (shotgunAimDir * trailOffset), angle);
			}
			else
			{
				myFlipbookManager.PlayPersistent(myTonyFlipbookHandles.tonyFirePowerShotInstance, 0.007f);
				myFlipbookManager.PlayAt(myTonyFlipbookHandles.tonyTrailFirePowershot, myPlayer.GetShotgunPosition() + (shotgunAimDir * trailOffset), angle);
			}

			GameStateUpdate::ShotgunRaycast(myPlayer, scene.tileConfigs, enemies, crates, myCrateUpdater, &myFlipbookManager, myEnvironmentFlipbookHandles.environmentHit,
				myEnvironmentFlipbookHandles.crateHit, myEnvironmentFlipbookHandles.metalCrateHit, myEnvironmentFlipbookHandles.enemyHit);
		}

		GameStateUpdate::EnemyCollision(enemies, myPlayer, scene.tileConfigs);

		for (int i = 0; i < iterations; ++i)
		{
			GameStateUpdate::ProjectileCollision(myPlayer, *projectiles, scene.tileConfigs, crates, deltaTime, tickrate,
				&myFlipbookManager, myEnvironmentFlipbookHandles.environmentHit,
				myEnvironmentFlipbookHandles.crateHit, myEnvironmentFlipbookHandles.metalCrateHit,
				myEnvironmentFlipbookHandles.enemyHit);
		}
	}

	myPlayer.LateUpdate(deltaTime);

	{ //-------------Updating Flipbook Locations-------------//
		Tga::Vector2f revolverAimDir = myPlayer.GetNormalizedRevolverAim();
		Tga::Vector2f shotgunAimDir = myPlayer.GetNormalizedShotgunAim();

		const float myPowerOffset = 224.f;
		const float myPowerPivotOffset = -200.f;
		const float myPowerSize = 420.f;

		const float revolverPivotOffset = -34.f;
		const float revolverOffset = 100.f;
		const float revolverSize = 200.f;

		const float shotgunOffset = 125.f;
		const float shotgunPivotOffset = -58.f;
		const float shotgunSize = 230.f;

		myFlipbookManager.MovePersistent(myTonyFlipbookHandles.tonyFireRevolverInstance, GetGunTransform(revolverAimDir, revolverSize, revolverPivotOffset, revolverOffset));
		myFlipbookManager.MovePersistent(myTonyFlipbookHandles.tonyFireShotgunInstance, GetGunTransform(shotgunAimDir, shotgunSize, shotgunPivotOffset, shotgunOffset));
		myFlipbookManager.MovePersistent(myTonyFlipbookHandles.tonyFirePowerShotInstance, GetGunTransform(shotgunAimDir, myPowerSize, myPowerPivotOffset, myPowerOffset));
	} //-----------------------------------------------------//

	myLevelTrigger.UpdateAnimation(deltaTime);

	if (GameStateUpdate::PlayerLevelTriggerCheck(myPlayer, myLevelTrigger))
	{
		myLevelTrigger.ActivateTrigger();
		myPlayer.SetFrozen(true);
	}
	if (!myLevelTrigger.GetActive())
	{
		if (myLevelTrigger.DelayTimerFinished())
		{
			UpdateTransitionSequence(scene.metaConfig.type);
			myTransitionSequenceTimer += deltaTime;

			if (TransitionSequenceFinished())
			{
				if (myFadeInOut.GetState() == FullscreenImageAnimationState::Stopped && myFadeInOut.GetAlpha() < 1.f)
				{
					myFadeInOut.StartFadeOut(myFadeOutTime);
				}

				if (myFadeInOut.GetAlpha() >= 1.f)
				{

					if (sceneConfig.metaConfig.type == SceneLoader::SceneType::Level1)
					{
						AudioManager::GetAudioPoolByHandle(AudioHandles::level1Ambience).Stop();
						AudioManager::GetAudioPoolByHandle(AudioHandles::level1Music).Stop();
					}
					else if (sceneConfig.metaConfig.type == SceneLoader::SceneType::Level2)
					{
						AudioManager::GetAudioPoolByHandle(AudioHandles::level2Ambience).Stop();
						AudioManager::GetAudioPoolByHandle(AudioHandles::level2Music).Stop();
					}

					sceneConfig = SceneLoader::LoadSceneByPath(myLevelTrigger.GetPath().GetString()); // Waiting for GameWorld Init before uncomment
					if (sceneConfig.metaConfig.type == SceneLoader::SceneType::BossScene)
					{
						return StateUpdateResult::CreateClearAndPush(myStateHandles.bossRoomState);
					}
					else
					{
						return StateUpdateResult::CreateClearAndPush(myStateHandles.gameState);
					}
				}
			}
		}
		else
		{
			myLevelTrigger.UpdateDelayTimer(deltaTime);
		}
	}
	if (myFadeInOut.GetState() != FullscreenImageAnimationState::Stopped && myFrameCount != 0)
	{
		myFadeInOut.Update(deltaTime);
	}
	if (myFrameCount == 0)
	{
		myFrameCount++;
	}

	myFlipbookManager.Update(deltaTime);

	{ //---------------- HUD Aim line-------------------------

		std::vector<CrateUpdater::Crate>& crates = myCrateUpdater.GetCrates();
		std::vector<Enemy>& enemies = *myEnemyUpdater.GetEnemies();

		if (Options::enableDualStick && !myInputMapper->GetIsUsingMouse())
		{
			if (myPlayer.GetIsRevolverEnabled())
			{
				myHUD.UpdateAimLine(
					{
								.type = AimLineType::Second,
								.aimOrigin = myPlayer.GetShotOrigin(),
								.aimDirection = myPlayer.GetNormalizedRevolverAim(),
								.tiles = scene.tileConfigs,
								.enemies = enemies,
								.crates = crates,
					}
					);
			}
			else
			{
				myHUD.UpdateAimLine(
					{
						.type = AimLineType::First,
						.aimOrigin = myPlayer.GetShotOrigin(),
						.aimDirection = myPlayer.GetNormalizedShotgunAim(),
						.tiles = scene.tileConfigs,
						.enemies = enemies,
						.crates = crates,
					}
					);
			}
		}
		else
		{
			myHUD.UpdateAimLine(
				{
						.type = AimLineType::First,
						.aimOrigin = myPlayer.GetShotOrigin(),
						.aimDirection = myPlayer.GetNormalizedShotgunAim(),
						.tiles = scene.tileConfigs,
						.enemies = enemies,
						.crates = crates,
				}
				);
		}
	}

	return StateUpdateResult::CreateContinue();
}

void GameState::Render()
{
	Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::GraphicsEngine& graphicsEngine = engine.GetGraphicsEngine();
	Tga::GraphicsStateStack& graphicsStateStack = graphicsEngine.GetGraphicsStateStack();

	graphicsStateStack.Push();

	myCamera.Prepare();

	SceneLoader::SceneConfig& sceneConfig = SceneLoader::GetActiveScene();

	myPlayer.Render();
	myEnemyUpdater.Render();
	myCrateUpdater.Render();
	myPickupUpdater.Render();
	myLevelTrigger.Render();

	for (const auto& object : sceneConfig.tileConfigs)
	{
		if (myCamera.IsPointWithinFrustum({ object.position, 0.f }))
		{
			myModelDrawer.Draw(object.modelInstance);
		}
	}

	for (const auto& object : sceneConfig.modelConfigs)
	{
		if (myCamera.IsPointWithinFrustum(object.modelInstance.GetTransform().GetPosition()))
		{
			myModelDrawer.Draw(object.modelInstance);
		}
	}

	PhysicsDebugDrawer::DrawDebugColliders(myCamera);

	PhysicsDebugDrawer::DrawDebugRayCone(
		Physics::Ray{
				.origin = myPlayer.GetShotOrigin(),
				.direction = myPlayer.GetNormalizedShotgunAim(),
				.magnitude = myPlayer.GetShotgunRange()
		},
		myPlayer.GetShotgunBulletAmount(),
		myPlayer.GetShotgunSpreadAngle()
	);

#if defined(_DEBUG)
	for (const auto& enemy : *myEnemyUpdater.GetEnemies())
	{
		if (!enemy.GetHasGun())
		{
			continue;
		}

		const Tga::Matrix2x2f rotationUp = Tga::Matrix2x2f::CreateFromRotation(FMath::DegToRad * enemy.GetDetectionAngle() * 0.5f);
		const Tga::Matrix2x2f rotationDown = Tga::Matrix2x2f::CreateFromRotation(FMath::DegToRad * -(enemy.GetDetectionAngle() * 0.5f));

		Physics::Ray peripheralUp
		{
			.origin = enemy.GetViewPosition(),
			.direction = enemy.GetFaceDirection() * rotationUp,
			.magnitude = 1.f,
		};

		Physics::Ray peripheralDown
		{
			.origin = enemy.GetViewPosition(),
			.direction = -1.f * (enemy.GetFaceDirection() * rotationDown),
			.magnitude = 1.f,
		};

		PhysicsDebugDrawer::DrawDebugRay(Physics::Ray{ peripheralUp.origin, peripheralUp.direction, enemy.GetDetectionRange() });
		PhysicsDebugDrawer::DrawDebugRay(Physics::Ray{ peripheralDown.origin - peripheralDown.direction * enemy.GetDetectionRange(), peripheralDown.direction, enemy.GetDetectionRange() });

		PhysicsDebugDrawer::DrawDebugAABB(myLevelTrigger.GetPosition(), myLevelTrigger.GetSize());

	}
#endif

	myAmbienceManager.RenderDebugVisuals();

	myDebugAnimationPlayer.Render();

	myHUD.RenderAimline();
	myFlipbookManager.Render();
	graphicsStateStack.Pop();


	if (myFadeInOut.GetAlpha() > FLT_EPSILON)
	{
		myFadeInOut.Render();
	}

	myHUD.RenderHitPoint(myCamera);
	myHUD.RenderClips(myPlayer.GetShotgunClip(), myPlayer.GetIsRevolverEnabled(), myPlayer.GetRevolverClip());
}

bool GameState::TransitionSequenceFinished() const
{
	return (myTransitionSequenceTimer > myLevelTrigger.GetAudioSequenceData().timeUntilFadeOut);
}

void GameState::UpdateTransitionSequence(SceneLoader::SceneType aSceneType)
{
	auto& sequence = myLevelTrigger.GetAudioSequenceData();

	float percentage = 1.f - (myTransitionSequenceTimer - sequence.bgmFadeStart) / sequence.bgmFadeDuration;
	percentage = std::clamp(percentage, 0.f, 1.f);

	const float masterVolume = percentage * static_cast<float>(Options::masterVolume) / static_cast<float>(Options::maxVolume);
	const float musicVolume = percentage * masterVolume * static_cast<float>(Options::musicVolume) / static_cast<float>(Options::maxVolume);

	if (sequence.bgmFadeStart <= myTransitionSequenceTimer)
	{
		if (aSceneType == SceneLoader::SceneType::Level1)
		{
			AudioManager::GetAudioPoolByHandle(AudioHandles::level1Ambience).SetVolume(masterVolume);
			AudioManager::GetAudioPoolByHandle(AudioHandles::level1Music).SetVolume(musicVolume);
		}
		else if (aSceneType == SceneLoader::SceneType::Level2)
		{
			AudioManager::GetAudioPoolByHandle(AudioHandles::level2Ambience).SetVolume(masterVolume);
			AudioManager::GetAudioPoolByHandle(AudioHandles::level2Music).SetVolume(musicVolume);
		}
	}

	if (!myLevelTrigger.GetHasSfxPlayed())
	{
		myLevelTrigger.SetHasSfxPlayed(true);
		AudioManager::GetAudioPoolByHandle(sequence.audioPoolHandle).Play();
		myLevelTrigger.OpenDoor();
	}

	if (aSceneType == SceneLoader::SceneType::Level1 && !myElevatorDingHasPlayed)
	{
		AudioManager::GetAudioPoolByHandle(AudioHandles::elevatorDing).Play();
		myElevatorDingHasPlayed = true;
	}
}
