#include "BossRoomState.h"

#include <tge/engine.h>
#include <tge/graphics/GraphicsEngine.h>
#include <tge/graphics/GraphicsStateStack.h>
#include <tge/drawers/ModelDrawer.h>

#include "AudioManager.h"

#include "Cutscene.h"
#include "SceneLoader.h"
#include "Go.h"
#include "MathUtils.h"
#include "ResolutionManager.h"


void BossRoomState::Init(StateHandle aMainMenuHandle, StateHandle aCutsceneStateHandle, InputMapper* anInputMapper, Timer* aTimer)
{
    myInputMapper = anInputMapper;
    myTimer = aTimer;

	myMainMenuHandle = aMainMenuHandle;
	myCutsceneStateHandle = aCutsceneStateHandle;

	myCamera.Init(myTimer);

	myPlayer.SetInput(anInputMapper);

	const Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::TextureManager& textureManager = engine.GetTextureManager();
	const Tga::Vector2f renderSize = Tga::Vector2f{engine.GetRenderSize()};

	myFullscreenSharedData.myTexture = textureManager.GetTexture("Sprites/Pixel.png");
	
	myTutorialSharedData.myTexture = textureManager.GetTexture("textures/UI/Tutorial/TutorialText/ShotgunToturialTxT.dds");   
	myTutorialAlpha = 0.f;
	myTimings.hasTutorialStarted = false;
	myTimings.timeWhenTutorialStartsFadingIn = 10.0f;
	myTimings.tutorialDuration = 20.0f;

	myTimings.timeWhenBossStartsYapping = 0.0f;
	myTimings.gibberishDelay = 0.8f;
	
	mySpeechBubbleFlipbookHandle = myFlipBookManager.MakeNewFlipbookHandle();
	myFlipBookManager.RegisterFlipBook({.flipbookAssetPath = "Sprites/T_DialogeBubble.png", .frameAmount = 2}, mySpeechBubbleFlipbookHandle, true);
}

void BossRoomState::OnPush()
{
	SceneLoader::SceneConfig& sceneConfig = SceneLoader::GetActiveScene();

	myCamera.SetDepth(sceneConfig.cameraConfig.depth);
	myCamera.SetHeight(sceneConfig.cameraConfig.height);
	myCamera.SetFov(sceneConfig.cameraConfig.fov);
	myCamera.MoveToPosition(Tga::Vector2f{ myCameraHorizontalOffset, 0.0f });

	myPlayer.Init(sceneConfig.playerConfig);
	myPlayer.EnableShotgun();
	myPlayer.DisablePowershot();
	myPlayer.DisableRevolver();
	myPlayer.FreezeTheShotgunSoThatItCanOnlyPointRightAndCantBeMovedWithMouseOrControllerOrAnyOtherInputDeviceForThatMatter();

	Tga::ModelFactory& modelFactory = Tga::ModelFactory::GetInstance();

	myBoss.animatedModelInstance = sceneConfig.bossConfig.animatedModelInstance;
	myBoss.idleAnimationPlayer = std::make_shared<Tga::AnimationPlayer>(
		modelFactory.GetAnimationPlayer(sceneConfig.bossConfig.idleClipReference.path.GetString(), sceneConfig.bossConfig.animatedModelInstance->GetModel()));

	myBoss.idleAnimationPlayer->SetIsLooping(true);
	myBoss.idleAnimationPlayer->Play();

	AudioManager::AudioPool& ambience = AudioManager::GetAudioPoolByHandle(AudioHandles::bossRoomAmbience);
	ambience.Play();

	myBoss.isDead = false;

    myLetterbox.Activate();

	myTimings.time = 0.0f;
	myTimings.gibberishTimer = 0.0f;

	const Tga::Vector3f bossPos3D = myBoss.animatedModelInstance->GetTransform().GetPosition();
	const Tga::Vector2f bossPos2D = {bossPos3D.x, bossPos3D.y};
	const Tga::Vector2f speechBubblePos = bossPos2D + mySpeechBubbleOffset;
	myFlipBookManager.PlayAt(mySpeechBubbleFlipbookHandle, speechBubblePos, mySpeechBubbleSize ,mySpeechBubbleSpeed, 0.f);
}

void BossRoomState::OnPop()
{
	myFlipBookManager.RemoveAllLoopingInstances();
	myStartedEndSequence = false;
	myEndSequenceTime = 0.0f;
}

void BossRoomState::OnGainFocus()
{
	SetMouseCaptureEnabled(true);
}

void BossRoomState::OnLoseFocus()
{
	SetMouseCaptureEnabled(false);
}

StateUpdateResult BossRoomState::Update()
{
	constexpr float height = 150.0f;

    const float deltaTime = myTimer->GetDeltaTime();
	myFlipBookManager.Update(deltaTime);
	
	myTimings.time += deltaTime;

	if (myTimings.time >= myTimings.timeWhenDoorCloses && !myTimings.hasDoorClosed)
	{
		myTimings.hasDoorClosed = true;
	}

	if (myTimings.time >= myTimings.timeWhenBossStartsYapping && !myTimings.hasBossStartedYapping)
	{
		myTimings.hasBossStartedYapping = true;
	}
	
	if (myTimings.time >= myTimings.timeWhenTutorialStartsFadingIn && !myTimings.hasTutorialStarted)
	{
		myTimings.hasTutorialStarted = true;
	}

	if (myTimings.hasBossStartedYapping && !myBoss.isDead)
	{
		myTimings.gibberishTimer -= deltaTime;

		AudioManager::AudioPool& gibberish = AudioManager::GetAudioPoolByHandle(AudioHandles::gibberish);
		if (myTimings.gibberishTimer <= 0.0f)
		{
			myTimings.gibberishTimer = myTimings.gibberishDelay;

			gibberish.PlayRandom();
		}
	}
	
	if (myTimings.hasTutorialStarted)
	{
		myTutorialAlpha = MathUtils::Clamp01((myTimings.time - myTimings.timeWhenTutorialStartsFadingIn) / (myTimings.tutorialDuration - myTimings.timeWhenTutorialStartsFadingIn));
	}

	myCamera.MoveTowardsPosition(Tga::Vector2f{ myCameraHorizontalOffset, height }, 0.9f, deltaTime);
	myCamera.Update();

	PlayerUpdateResult playerUpdateResult = myPlayer.Update(deltaTime, myCamera);
	myPlayer.LateUpdate(deltaTime);

	myBoss.idleAnimationPlayer->Update(deltaTime);
	myBoss.animatedModelInstance->SetPose(myBoss.idleAnimationPlayer->GetLocalSpacePose());

	if (playerUpdateResult.action == PlayerUpdateResult::Action::Shotgun)
	{
		myBoss.isDead = true;

		AudioManager::AudioPool& ambience = AudioManager::GetAudioPoolByHandle(AudioHandles::bossRoomAmbience);
		ambience.Stop();

		AudioManager::AudioPool& gibberish = AudioManager::GetAudioPoolByHandle(AudioHandles::gibberish);
		gibberish.Stop();

		Cutscene::PreparedConfig preparedConfig
		{
			.filePath = "videos/Ending_WITH SHOTGUN INTRO_NO SOUND.mp4",
			.audioHandle = AudioHandles::endingCutscene,
			.nextStateHandle = myMainMenuHandle
		};

		Cutscene::Set(preparedConfig);

		return StateUpdateResult::CreateClearAndPush(myCutsceneStateHandle);
	}

    myLetterbox.Update(deltaTime);

    return StateUpdateResult::CreateContinue();
}

void BossRoomState::Render()
{
	const Tga::Engine* engine = Tga::Engine::GetInstance();
	Tga::GraphicsEngine& graphicsEngine = engine->GetGraphicsEngine();
	Tga::GraphicsStateStack& graphicsStateStack = graphicsEngine.GetGraphicsStateStack();
	Tga::ModelDrawer& modelDrawer = graphicsEngine.GetModelDrawer();
	Tga::SpriteDrawer& spriteDrawer = graphicsEngine.GetSpriteDrawer();

	Tga::Vector2ui renderSize = engine->GetRenderSize();
	Tga::Vector2f resolution = Tga::Vector2f{ renderSize };
	
	graphicsStateStack.Push();

	myCamera.Prepare();

	myPlayer.Render();

	modelDrawer.Draw(*myBoss.animatedModelInstance);

	SceneLoader::SceneConfig& sceneConfig = SceneLoader::GetActiveScene();    

	for (const auto& object : sceneConfig.tileConfigs)
	{
		if (myCamera.IsPointWithinFrustum({ object.position, 0.f }))
		{
			modelDrawer.Draw(object.modelInstance);
		}
	}

	for (const auto& object : sceneConfig.modelConfigs)
	{
		if (myCamera.IsPointWithinFrustum(object.modelInstance.GetTransform().GetPosition()))
		{
			modelDrawer.Draw(object.modelInstance);
		}
	}
	
	myFlipBookManager.Render();

	graphicsStateStack.Pop();

	if (myStartedEndSequence)
	{
		Tga::Sprite2DInstanceData blackBarInstanceData;
		blackBarInstanceData.mySize = resolution;
		blackBarInstanceData.myPosition = resolution / 2.0f;
		blackBarInstanceData.myColor = Tga::Color{ 0.0f, 0.0f, 0.0f, 1.0f };

		spriteDrawer.Draw(myFullscreenSharedData, blackBarInstanceData);
	}

	const float uiScale = ResolutionManager::GetUIScale();
	const Tga::Vector2f baseTutorialSize{ myTutorialSharedData.myTexture->CalculateTextureSize() };
	const Tga::Vector2f scaledTutorialSize = { baseTutorialSize.x * uiScale * myTutorialSizeMultiplier, baseTutorialSize.y * uiScale * myTutorialSizeMultiplier };

	Tga::Sprite2DInstanceData tutorialInstance =
	{
		.myPosition = { renderSize.x * 0.5f, renderSize.y * myTutorialVerticalOffset },
		.mySize = scaledTutorialSize,
		.myColor = Tga::Color{1.f,1.f,1.f,myTutorialAlpha},
	};

	spriteDrawer.Draw(myTutorialSharedData, tutorialInstance);

	myLetterbox.Render();
}
