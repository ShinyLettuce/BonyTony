#include "CutsceneState.h"
#include "ResolutionManager.h"
#include "SceneLoader.h"
#include "tge/graphics/GraphicsEngine.h"
#include "tge/graphics/TextureResource.h"
#include "tge/settings/settings.h"
#include "Cutscene.h"
#include "Go.h"

CutsceneState::~CutsceneState()
{
}

void CutsceneState::Init(CutsceneStateHandles someCutsceneStateHandles, InputMapper* anInputMapper, Timer* aTimer)
{
	myCutsceneStateHandles = someCutsceneStateHandles;
	myInputMapper = anInputMapper;
	myTimer = aTimer;
}

void CutsceneState::OnPush()
{
	Cutscene::PreparedConfig preparedConfig = Cutscene::Get();

	std::string path = Tga::Settings::ResolveAssetPath(preparedConfig.filePath);

	myVideo = Tga::Video{};

	if (!myVideo.Init(path.data(), false))
	{
		std::cout << "[CutsceneState.cpp] Error initializing video.\n";
		return;
	}

	AudioManager::GetAudioPoolByHandle(preparedConfig.audioHandle).Play();
	myVideo.Play();
}

void CutsceneState::OnPop()
{
	myVideo.Stop();
}

void CutsceneState::OnGainFocus()
{
	SetMouseCaptureEnabled(true);
}

void CutsceneState::OnLoseFocus()
{
	SetMouseCaptureEnabled(false);
}

StateUpdateResult CutsceneState::Update()
{
	Cutscene::PreparedConfig preparedConfig = Cutscene::Get();

	myVideo.Update(myTimer->GetDeltaTime());
	myVideoData.myTexture = myVideo.GetTexture();

	if (myVideo.GetStatus() == Tga::VideoStatus::ReachedEnd)
	{
		AudioManager::GetAudioPoolByHandle(preparedConfig.audioHandle).Stop();
		SceneLoader::LoadSceneByPath("levels/Level1.tgs");
		return StateUpdateResult::CreateClearAndPush(preparedConfig.nextStateHandle);
	}

	if (myInputMapper->IsActionJustActivated(GameAction::SkipCutscene))
	{
		AudioManager::GetAudioPoolByHandle(preparedConfig.audioHandle).Stop();
		SceneLoader::LoadSceneByPath("levels/Level1.tgs");
		return StateUpdateResult::CreateClearAndPush(preparedConfig.nextStateHandle);
	}

	return StateUpdateResult::CreateContinue();
}

void CutsceneState::Render()
{
	Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::SpriteDrawer& spriteDrawer = engine.GetGraphicsEngine().GetSpriteDrawer();

	Tga::Vector2ui renderSize = engine.GetRenderSize();
	Tga::Vector2f resolution{ renderSize };

	Tga::TextureResource* videoTexture = myVideo.GetTexture();
	Tga::Vector2f videoTextureResolution{ videoTexture->CalculateTextureSize() };
	Tga::Vector2f videoResolution{ myVideo.GetVideoSize() };

	Tga::Vector2f stretchedVideoResolution{ resolution * (videoTextureResolution / videoResolution) };

	Tga::Sprite2DInstanceData instanceData;
	instanceData.myPivot = Tga::Vector2f{ 0.0f, 0.0f };
	instanceData.myPosition = Tga::Vector2f{ 0.0f, resolution.y };
	instanceData.mySize = stretchedVideoResolution;

	spriteDrawer.Draw(myVideoData, instanceData);
}
