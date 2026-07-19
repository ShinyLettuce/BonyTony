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

#include <nlohmann/json.hpp>
#include <fstream>

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
	myPlayer.SetFlipbookManager(&myFlipbookManager);
	myStateHandles = aStateHandle;
	myDebugAnimationPlayer.Init();

	myFlipbookManager.RegisterFlipBook(FlipBookPresets::TONY_SHOTGUN_FIRE, FlipbookHandle::ShotgunFire);
	myFlipbookManager.RegisterFlipBook(FlipBookPresets::TONY_SHOTGUN_FIRE_TRAIL, FlipbookHandle::ShotgunFireTrail);
	myFlipbookManager.RegisterFlipBook(FlipBookPresets::TONY_POWERSHOT_FIRE, FlipbookHandle::PowershotFire);
	myFlipbookManager.RegisterFlipBook(FlipBookPresets::TONY_POWERSHOT_FIRE_TRAIL, FlipbookHandle::PowershotFireTrail);
	myFlipbookManager.RegisterFlipBook(FlipBookPresets::TONY_REVOLVER_FIRE, FlipbookHandle::RevolverFire);
	myFlipbookManager.RegisterFlipBook(FlipBookPresets::ENVIRONMENT_HIT, FlipbookHandle::EnvironmentHit);
	myFlipbookManager.RegisterFlipBook(FlipBookPresets::CRATE_HIT, FlipbookHandle::CrateHit);
	myFlipbookManager.RegisterFlipBook(FlipBookPresets::METAL_CRATE_HIT, FlipbookHandle::MetalCrateHit);
	myFlipbookManager.RegisterFlipBook(FlipBookPresets::ENEMY_HIT, FlipbookHandle::EnemyHit);
	myFlipbookManager.RegisterFlipBook(FlipBookPresets::STEAM_ENVIRONMENT, FlipbookHandle::SteamEnvironment, true);
	myFlipbookManager.RegisterFlipBook(FlipBookPresets::SHELL_EJECT, FlipbookHandle::ShellEject);
	myFlipbookManager.RegisterFlipBook(FlipBookPresets::BULLET_EJECT, FlipbookHandle::BulletEject);

	myFlipbookManager.InitPersistentFlipbooks();
	myFlipbookManager.SetPersistentInstanceFlipbook(PersistentInstanceHandle::ShotgunFire, FlipbookHandle::ShotgunFire);
	myFlipbookManager.SetPersistentInstanceFlipbook(PersistentInstanceHandle::PowerShotFire, FlipbookHandle::PowershotFire);
	myFlipbookManager.SetPersistentInstanceFlipbook(PersistentInstanceHandle::RevolverFire, FlipbookHandle::RevolverFire);

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
	myPlayer.SetCamera(&myCamera);

	myCamera.MoveToPosition(myPlayer.GetPosition());

	myShotgunMaxClip = sceneConfig.playerConfig.shotgunData.maxClip;
	myRevolverMaxClip = sceneConfig.playerConfig.revolverData.maxClip;

	myHUD.Init(sceneConfig.playerConfig.shotgunData.maxClip, sceneConfig.playerConfig.revolverData.maxClip, myPlayer.GetRevolverRange(), myTimer);
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

	static float cameraFollowDecay = 4.0f;
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
	if (myInputMapper->GetIsUsingMouse())
	{
		playerPosition.x *= cameraHorizontalFollowCoefficient;
		Tga::Vector2ui renderSize = Tga::Engine::GetInstance()->GetRenderSize();
		Tga::Vector2f resolution{ renderSize };
		Tga::Vector2f mousePosition = myInputMapper->GetMousePositionYUp();
		mousePosition = mousePosition / resolution;
		mousePosition *= 2.0f;
		mousePosition.x -= 1.0f;
		mousePosition.y -= 1.0f;
		myCameraTargetPosition = playerPosition + mousePosition * 50.0f;
	}
	else
	{
		myCameraTargetPosition = playerPosition;
	}

	myCamera.MoveTowardsPosition(myCameraTargetPosition, cameraFollowDecay, deltaTime);
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
				&myFlipbookManager, deltaTime, tickrate);
		}

		if (playerUpdateResult.action == PlayerUpdateResult::Action::Revolver && myPlayer.GetIsRevolverEnabled())
		{
			myFlipbookManager.PlayPersistent(PersistentInstanceHandle::RevolverFire, 0.007f);
			GameStateUpdate::RevolverRaycast(scene.tileConfigs, enemies, crates, myCrateUpdater, myPlayer, &myFlipbookManager);
		}
		if (playerUpdateResult.action == PlayerUpdateResult::Action::Shotgun || playerUpdateResult.action == PlayerUpdateResult::Action::PowerShot)
		{
			Tga::Vector2f shotgunAimDir = myPlayer.GetNormalizedShotgunAim();
			Tga::Vector2f forward{ 1.f, 0.f };
			const float angle = std::atan2f(forward.Cross(shotgunAimDir), forward.Dot(shotgunAimDir));
			const float trailOffset = 400.f;

			if (playerUpdateResult.action == PlayerUpdateResult::Action::Shotgun)
			{
				myFlipbookManager.PlayPersistent(PersistentInstanceHandle::ShotgunFire, 0.007f);
				myFlipbookManager.PlayAt(FlipbookHandle::ShotgunFireTrail, myPlayer.GetShotgunPosition() + (shotgunAimDir * trailOffset), angle);
			}
			else
			{
				myFlipbookManager.PlayPersistent(PersistentInstanceHandle::PowerShotFire, 0.007f);
				myFlipbookManager.PlayAt(FlipbookHandle::PowershotFireTrail, myPlayer.GetShotgunPosition() + (shotgunAimDir * trailOffset), angle);
			}

			GameStateUpdate::ShotgunRaycast(myPlayer, scene.tileConfigs, enemies, crates, myCrateUpdater, &myFlipbookManager);
		}

		GameStateUpdate::EnemyCollision(enemies, myPlayer, scene.tileConfigs);

		for (int i = 0; i < iterations; ++i)
		{
			GameStateUpdate::ProjectileCollision(myPlayer, *projectiles, scene.tileConfigs, crates, deltaTime, tickrate, &myFlipbookManager);
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

		myFlipbookManager.MovePersistent(PersistentInstanceHandle::RevolverFire, GetGunTransform(revolverAimDir, revolverSize, revolverPivotOffset, revolverOffset));
		myFlipbookManager.MovePersistent(PersistentInstanceHandle::ShotgunFire, GetGunTransform(shotgunAimDir, shotgunSize, shotgunPivotOffset, shotgunOffset));
		myFlipbookManager.MovePersistent(PersistentInstanceHandle::PowerShotFire, GetGunTransform(shotgunAimDir, myPowerSize, myPowerPivotOffset, myPowerOffset));
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
				myHUD.UpdateAimLine({
								.type = AimLineType::Second,
								.aimOrigin = myPlayer.GetShotOrigin(),
								.aimDirection = myPlayer.GetNormalizedRevolverAim(),
								.tiles = scene.tileConfigs,
								.enemies = enemies,
								.crates = crates,
					});
			}
			else
			{
				myHUD.UpdateAimLine({
						.type = AimLineType::First,
						.aimOrigin = myPlayer.GetShotOrigin(),
						.aimDirection = myPlayer.GetNormalizedShotgunAim(),
						.tiles = scene.tileConfigs,
						.enemies = enemies,
						.crates = crates,
					});
			}
		}
		else
		{
			myHUD.UpdateAimLine({
						.type = AimLineType::First,
						.aimOrigin = myPlayer.GetShotOrigin(),
						.aimDirection = myPlayer.GetNormalizedShotgunAim(),
						.tiles = scene.tileConfigs,
						.enemies = enemies,
						.crates = crates,
				});
		}
	}

	return StateUpdateResult::CreateContinue();
}


void GameState::Render()
{
	Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::GraphicsEngine& graphicsEngine = engine.GetGraphicsEngine();
	Tga::GraphicsStateStack& graphicsStateStack = graphicsEngine.GetGraphicsStateStack();

	SceneLoader::SceneConfig& sceneConfig = SceneLoader::GetActiveScene();

	// Draw opaque objects

	Tga::Vector2f playerPosition2d = myPlayer.GetPosition();
	Tga::Vector3f playerPosition3d(playerPosition2d, 0.0f);

	static float shotgunLightIntensity = 2.0f;
	static float revolverLightIntensity = 1.0f;
	static float powerShotLightIntensity = 2.4f;

	static float shotgunLightRange = 50.0f;
	static float revolverLightRange = 50.0f;
	static float powerShotLightRange = 50.0f;

	static float shotgunLightRadius = 200.0f;
	static float revolverLightRadius = 200.0f;
	static float powerShotLightRadius = 200.0f;

	static float shotgunLightDuration = 0.06f;
	static float revolverLightDuration = 0.05f;
	static float powerShotLightDuration = 0.07f;

	static bool initialized{ false };
	if (!initialized) {
		initialized = true;

		nlohmann::json json;

		std::filesystem::path path = Tga::Settings::GameAssetRoot() / "MuzzleFlashSettings.json";
		std::ifstream fs(path, std::ios::in);

		fs >> json;

		shotgunLightIntensity = json["Shotgun"]["Intensity"];
		shotgunLightRange = json["Shotgun"]["Range"];
		shotgunLightRadius = json["Shotgun"]["Radius"];
		shotgunLightDuration = json["Shotgun"]["Duration"];

		revolverLightIntensity = json["Revolver"]["Intensity"];
		revolverLightRange = json["Revolver"]["Range"];
		revolverLightRadius = json["Revolver"]["Radius"];
		revolverLightDuration = json["Revolver"]["Duration"];

		powerShotLightIntensity = json["PowerShot"]["Intensity"];
		powerShotLightRange = json["PowerShot"]["Range"];
		powerShotLightRadius = json["PowerShot"]["Radius"];
		powerShotLightDuration = json["PowerShot"]["Duration"];
	}

#if !defined(_RETAIL)
	if (ImGui::Begin("Muzzle Flash", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ImGui::Button("Save")) {
			nlohmann::json json;

			json["Shotgun"]["Intensity"] = shotgunLightIntensity;
			json["Shotgun"]["Range"] = shotgunLightRange;
			json["Shotgun"]["Radius"] = shotgunLightRadius;
			json["Shotgun"]["Duration"] = shotgunLightDuration;

			json["Revolver"]["Intensity"] = revolverLightIntensity;
			json["Revolver"]["Range"] = revolverLightRange;
			json["Revolver"]["Radius"] = revolverLightRadius;
			json["Revolver"]["Duration"] = revolverLightDuration;

			json["PowerShot"]["Intensity"] = powerShotLightIntensity;
			json["PowerShot"]["Range"] = powerShotLightRange;
			json["PowerShot"]["Radius"] = powerShotLightRadius;
			json["PowerShot"]["Duration"] = powerShotLightDuration;

			std::filesystem::path path = Tga::Settings::GameAssetRoot() / "MuzzleFlashSettings.json";
			std::ofstream fs(path, std::ios::out | std::ios::trunc);

			fs::permissions(path, fs::perms::all);
			fs << json.dump();
		}

		ImGui::SameLine();

		if (ImGui::Button("Load")) {
			nlohmann::json json;

			std::filesystem::path path = Tga::Settings::GameAssetRoot() / "MuzzleFlashSettings.json";
			std::ifstream fs(path, std::ios::in);

			fs >> json;

			shotgunLightIntensity = json["Shotgun"]["Intensity"];
			shotgunLightRange = json["Shotgun"]["Range"];
			shotgunLightRadius = json["Shotgun"]["Radius"];
			shotgunLightDuration = json["Shotgun"]["Duration"];

			revolverLightIntensity = json["Revolver"]["Intensity"];
			revolverLightRange = json["Revolver"]["Range"];
			revolverLightRadius = json["Revolver"]["Radius"];
			revolverLightDuration = json["Revolver"]["Duration"];

			powerShotLightIntensity = json["PowerShot"]["Intensity"];
			powerShotLightRange = json["PowerShot"]["Range"];
			powerShotLightRadius = json["PowerShot"]["Radius"];
			powerShotLightDuration = json["PowerShot"]["Duration"];
		}

		ImGui::Spacing();

		ImGui::TextDisabled("Shotgun Light Settings");
		ImGui::Separator();
		ImGui::DragFloat("Shotgun Intensity", &shotgunLightIntensity, 0.1f);
		ImGui::DragFloat("Shotgun Range", &shotgunLightRange, 0.1f);
		ImGui::DragFloat("Shotgun Radius", &shotgunLightRadius, 0.1f);
		ImGui::DragFloat("Shotgun Duration", &shotgunLightDuration, 0.01f);
		ImGui::TextDisabled("Revolver Light Settings");
		ImGui::Separator();
		ImGui::DragFloat("Revolver Intensity", &revolverLightIntensity, 0.1f);
		ImGui::DragFloat("Revolver Range", &revolverLightRange, 0.1f);
		ImGui::DragFloat("Revolver Radius", &revolverLightRadius, 0.1f);
		ImGui::DragFloat("Revolver Duration", &revolverLightDuration, 0.01f);
		ImGui::TextDisabled("PowerShot Light Settings");
		ImGui::Separator();
		ImGui::DragFloat("PowerShot Intensity", &powerShotLightIntensity, 0.1f);
		ImGui::DragFloat("PowerShot Range", &powerShotLightRange, 0.1f);
		ImGui::DragFloat("PowerShot Radius", &powerShotLightRadius, 0.1f);
		ImGui::DragFloat("PowerShot Duration", &powerShotLightDuration, 0.01f);
	}
	ImGui::End();
#endif

	graphicsStateStack.Push();
	if (myPlayer.GetTimeSinceFiredShotgun() < shotgunLightDuration)
	{
		const float intensity = shotgunLightIntensity;

		graphicsStateStack.AddPointLight(Tga::PointLight{
			.position = playerPosition3d,
			.color = Tga::Color{ intensity * 0.5f, intensity * 0.7f, intensity * 1.0f, 1.0f },
			.range = shotgunLightRange * 100.0f,
			.radius = shotgunLightRadius
			});
	}

	if (myPlayer.GetTimeSinceFiredRevolver() < revolverLightDuration)
	{
		const float intensity = revolverLightIntensity;

		graphicsStateStack.AddPointLight(Tga::PointLight{
			.position = playerPosition3d,
			.color = Tga::Color{ intensity * 0.5f, intensity * 0.7f, intensity * 1.0f, 1.0f },
			.range = revolverLightRange * 100.0f,
			.radius = revolverLightRadius
			});
	}

	if (myPlayer.GetTimeSinceFiredPowerShot() < powerShotLightDuration)
	{
		const float intensity = powerShotLightIntensity;

		graphicsStateStack.AddPointLight(Tga::PointLight{
			.position = playerPosition3d,
			.color = Tga::Color{ intensity * 0.5f, intensity * 0.7f, intensity * 1.0f, 1.0f },
			.range = powerShotLightRange * 100.0f,
			.radius = powerShotLightRadius
			});
	}

	{
		myCamera.Prepare();

		myPlayer.Render();
		myEnemyUpdater.Render();
		myCrateUpdater.Render();
		myPickupUpdater.Render();
		myLevelTrigger.Render();

		for (const auto& object : sceneConfig.tileConfigs)
		{
			Tga::Model& model = *object.modelInstance.GetModel();

			float maxRadius = 0.0f;
			for (int i = 0; i < model.GetMeshCount(); ++i)
			{
				const Tga::Model::MeshData& meshData = model.GetMeshData(i);
				const float radius = meshData.Bounds.Radius;
				if (maxRadius < radius)
				{
					maxRadius = radius;
				}
			}

			if (myCamera.IsPointWithinFrustum({ object.position, 0.f }, maxRadius))
			{
				myModelDrawer.Draw(object.modelInstance);
			}
		}

		// Draw lit models

		for (const auto& object : sceneConfig.modelConfigs)
		{
			Tga::Model& model = *object.modelInstance.GetModel();

			float maxRadius = 0.0f;
			for (int i = 0; i < model.GetMeshCount(); ++i)
			{
				const Tga::Model::MeshData& meshData = model.GetMeshData(i);
				const float radius = meshData.Bounds.Radius;
				if (maxRadius < radius)
				{
					maxRadius = radius;
				}
			}

			if (myCamera.IsPointWithinFrustum(object.modelInstance.GetTransform().GetPosition(), maxRadius))
			{
				myModelDrawer.DrawLambert(object.modelInstance);
			}
		}

		// Draw unlit models

		for (const auto& object : sceneConfig.unlitModelConfigs)
		{
			Tga::Model& model = *object.modelInstance.GetModel();

			float maxRadius = 0.0f;
			for (int i = 0; i < model.GetMeshCount(); ++i)
			{
				const Tga::Model::MeshData& meshData = model.GetMeshData(i);
				const float radius = meshData.Bounds.Radius;
				if (maxRadius < radius)
				{
					maxRadius = radius;
				}
			}

			if (myCamera.IsPointWithinFrustum(object.modelInstance.GetTransform().GetPosition(), maxRadius))
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

		// Draw hud

		graphicsStateStack.Push();
		graphicsStateStack.SetBlendState(Tga::BlendState::AlphaBlend);
		{
			myAmbienceManager.RenderDebugVisuals();
			myDebugAnimationPlayer.Render();
			myFlipbookManager.Render();

			myHUD.RenderAimline();
		}
		graphicsStateStack.Pop();

		// Draw transparent objects

		graphicsStateStack.Push();
		graphicsStateStack.SetBlendState(Tga::BlendState::AdditiveBlend);

		for (SceneLoader::ModelConfig& object : sceneConfig.transparentObjectConfig)
		{
			myModelDrawer.Draw(object.modelInstance);
		}

		graphicsStateStack.Pop();
	}
	graphicsStateStack.Pop();

	// Draw screenspace hud

	graphicsStateStack.Push();
	graphicsStateStack.SetBlendState(Tga::BlendState::AlphaBlend);
	{
		myHUD.RenderVignette();
		myHUD.RenderClips(myPlayer.GetShotgunClip(), myPlayer.GetIsRevolverEnabled(), myPlayer.GetRevolverClip());
		myHUD.RenderHitPoint(myCamera);
		if (myFadeInOut.GetAlpha() > FLT_EPSILON)
		{
			myFadeInOut.Render();
		}
	}
	graphicsStateStack.Pop();
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
