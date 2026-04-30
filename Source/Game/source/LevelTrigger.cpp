#include "LevelTrigger.h"

#include "tge/Engine.h"
#include "tge/drawers/ModelDrawer.h"
#include "tge/drawers/SpriteDrawer.h"
#include "MathUtils.h"

void LevelTrigger::Init(const SceneLoader::LevelTriggerConfig& aLevelTriggerConfig, const AudioSequenceData& aAudioSequence)
{
	myLevelTriggerData =
	{
		.sceneToLoad = aLevelTriggerConfig.sceneToLoad,
		.position = aLevelTriggerConfig.position,
		.size = aLevelTriggerConfig.size,
		.modelInstance = aLevelTriggerConfig.modelInstance,
		.active = true,
		.exists = aLevelTriggerConfig.exists,
		.fadeOutTime = 1.f,
		.delayTime = 2.f,
		.animationState = AnimationState::None
	};

	myAudioSequenceData = aAudioSequence;

	myDelayTimer = 0.f;
	myFadeOutTimer = 0.f;
	
	Tga::ModelFactory& modelFactory = Tga::ModelFactory::GetInstance();
	
	myLevelTriggerData.closeAnimation = std::make_shared<Tga::AnimationPlayer>(
		modelFactory.GetAnimationPlayer(aLevelTriggerConfig.closeAnimation.path.GetString(), aLevelTriggerConfig.modelInstance->GetModel()));
	
	myLevelTriggerData.openAnimation = std::make_shared<Tga::AnimationPlayer>(
		modelFactory.GetAnimationPlayer(aLevelTriggerConfig.openAnimation.path.GetString(), aLevelTriggerConfig.modelInstance->GetModel()));

	if (!myLevelTriggerData.modelInstance->IsValid())
	{
		std::cout << "[LevelTrigger.cpp] Failed to load model instance, will not render.]\n";
	}

	ResetDoor();
}

Tga::Vector2f LevelTrigger::GetPosition() const
{
	return myLevelTriggerData.position;
}

Tga::Vector2f LevelTrigger::GetSize() const
{
	return myLevelTriggerData.size;
}

void LevelTrigger::ActivateTrigger()
{
	myLevelTriggerData.active = false;
}

void LevelTrigger::UpdateDelayTimer(float aDeltaTime)
{
	myDelayTimer += aDeltaTime;
}

void LevelTrigger::UpdateAnimation(float aDeltaTime)
{
	switch (myLevelTriggerData.animationState)
	{
		case AnimationState::Closing:
		{
			myLevelTriggerData.closeAnimation->Update(aDeltaTime);
			myLevelTriggerData.modelInstance->SetPose(myLevelTriggerData.closeAnimation->GetLocalSpacePose());
			break;
		}
		case AnimationState::Opening:
		{
			myLevelTriggerData.openAnimation->Update(aDeltaTime);
			myLevelTriggerData.modelInstance->SetPose(myLevelTriggerData.openAnimation->GetLocalSpacePose());
			break;
		}
	}
}

void LevelTrigger::OpenDoor()
{
	if (myLevelTriggerData.animationState == AnimationState::Opening)
	{
		return;
	}
	myLevelTriggerData.openAnimation->SetTime(0);
	myLevelTriggerData.openAnimation->Play();
	myLevelTriggerData.animationState = AnimationState::Opening;
}

void LevelTrigger::CloseDoor()
{
	if (myLevelTriggerData.animationState == AnimationState::Closing)
	{
		return;
	}
	myLevelTriggerData.closeAnimation->SetTime(0);
	myLevelTriggerData.closeAnimation->Play();
	myLevelTriggerData.animationState = AnimationState::Closing;
}

void LevelTrigger::ResetDoor()
{
	myDelayTimer = 0.f;
	myFadeOutTimer = 0.f;

	myLevelTriggerData.animationState = AnimationState::None;

	if (myLevelTriggerData.modelInstance.get())
	{
		myLevelTriggerData.modelInstance->ResetPose();
	}

	if (myLevelTriggerData.closeAnimation.get())
	{
		myLevelTriggerData.closeAnimation->SetTime(0.0f);
	}

	if (myLevelTriggerData.openAnimation.get())
	{
		myLevelTriggerData.openAnimation->SetTime(0.0f);
	}
}

bool LevelTrigger::DelayTimerFinished() const
{
	return (myDelayTimer > myLevelTriggerData.delayTime);
}

bool LevelTrigger::GetActive() const
{
	return myLevelTriggerData.active;
}

bool LevelTrigger::GetExists() const
{
	return myLevelTriggerData.exists;
}

const LevelTrigger::AudioSequenceData& LevelTrigger::GetAudioSequenceData() const
{
	return myAudioSequenceData;
}

bool LevelTrigger::GetHasSfxPlayed() const
{
	return mySfxHasPlayed;
}

void LevelTrigger::SetHasSfxPlayed(const bool aHasPlayed)
{
	mySfxHasPlayed = aHasPlayed;
}

void LevelTrigger::Render()
{
	if (myLevelTriggerData.modelInstance->IsValid())
	{
		const Tga::Engine& engine = *Tga::Engine::GetInstance();
		Tga::GraphicsEngine& graphicsEngine = engine.GetGraphicsEngine();
		Tga::ModelDrawer& modelDrawer = graphicsEngine.GetModelDrawer();

		modelDrawer.Draw(*myLevelTriggerData.modelInstance);
	}
}


