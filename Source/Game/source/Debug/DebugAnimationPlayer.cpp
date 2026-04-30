#include "DebugAnimationPlayer.h"

#include <tge/engine.h>
#include <tge/animation/Animation.h>
#include <tge/animation/AnimationPlayer.h>
#include <tge/graphics/Camera.h>
#include <tge/graphics/dx11.h>
#include <tge/graphics/GraphicsEngine.h>
#include <tge/drawers/ModelDrawer.h>
#include <tge/model/Modelfactory.h>
#include <tge/texture/TextureManager.h>

static Tga::AnimatedModelInstance globalAnimatedModelInstance;
static Tga::AnimationPlayer globalAnimationPlayer;

static bool globalError = false;

void DebugAnimationPlayer::Init()
{
	Tga::ModelFactory& modelFactory = Tga::ModelFactory::GetInstance();
	globalAnimatedModelInstance = modelFactory.GetAnimatedModelInstance("models/AnimationTest/SK_Player_Tied_up_01.fbx");

	if(!globalAnimatedModelInstance.IsValid())
	{
		std::cout << "[DebugAnimationPlayer.cpp] Failed while creating animated model instance" << '\n';
		globalError = true;
		return;
	}

	globalAnimatedModelInstance.GetTransform().SetPosition(Tga::Vector3f{ 0.0f, 100.0f, 0.0f });
	globalAnimatedModelInstance.GetTransform().SetRotation(Tga::Vector3f{ 270.0f, 270.0f, 0.0f });
	globalAnimatedModelInstance.GetTransform().Scale(Tga::Vector3f{ 100.0f, 100.0f, 100.0f });

	globalAnimationPlayer = modelFactory.GetAnimationPlayer("models/AnimationTest/SK_Player_Tied_up_01.fbx", globalAnimatedModelInstance.GetModel());

	if(!globalAnimationPlayer.IsValid())
	{
		std::cout << "[DebugAnimationPlayer.cpp] Failed while creating animation player" << '\n';
		globalError = true;
		return;
	}

	globalAnimationPlayer.Play();

	globalAnimationPlayer.SetIsLooping(true);
}

void DebugAnimationPlayer::Update()
{
	if (globalError)
	{
		return;
	}

	if (globalAnimationPlayer.GetState() == Tga::AnimationState::Finished)
	{
		globalAnimationPlayer.Play();
	}

	globalAnimationPlayer.Update(1.0f / 144.0f);
}

void DebugAnimationPlayer::Render()
{
	if (globalError)
	{
		return;
	}

	Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::GraphicsEngine& graphicsEngine = engine.GetGraphicsEngine();
	Tga::ModelDrawer& modelDrawer = graphicsEngine.GetModelDrawer();

	globalAnimatedModelInstance.SetPose(globalAnimationPlayer);
	modelDrawer.Draw(globalAnimatedModelInstance);
}
