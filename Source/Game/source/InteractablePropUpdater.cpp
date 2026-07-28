#include "InteractablePropUpdater.h"

void AnimatedPropUpdater::Init(std::vector<SceneLoader::AnimatedPropConfig> aProps)
{
	myInteractableProps.clear();

	for (SceneLoader::AnimatedPropConfig p : aProps)
	{
		Tga::ModelFactory& modelFactory = Tga::ModelFactory::GetInstance();

		std::shared_ptr<Tga::AnimationPlayer> interactAnimationPlayer = std::make_shared<Tga::AnimationPlayer>(
			modelFactory.GetAnimationPlayer(
				p.interactionClipReference.path.GetString(),
				p.animatedModelInstance->GetModel()
			)
		);

		AnimatedProp temp = AnimatedProp
		{
			.animationPlayer = interactAnimationPlayer,
			.animatedModelInstance = p.animatedModelInstance,
			.index = static_cast<unsigned int>(myInteractableProps.size()),
			.size = p.size,
			.oneTimeTriggerable = p.oneTimeTriggerable
		};

		// hmm how to separate these
		if (p.interactable)
		{
			myInteractableProps.push_back(temp);

			AnimatedProp& prop = myInteractableProps.back();

			prop.animationPlayer->Stop();
			prop.animationPlayer->SetTime(0.0f);
			prop.animatedModelInstance->GetTransform().SetPosition(prop.animatedModelInstance->GetTransform().GetPosition());
		}
		else if (p.looping)
		{
			myLoopingProps.push_back(temp);

			AnimatedProp& prop = myLoopingProps.back();

			prop.animationPlayer->Stop();
			prop.animationPlayer->SetTime(0.0f);
			prop.animatedModelInstance->GetTransform().SetPosition(prop.animatedModelInstance->GetTransform().GetPosition());
			prop.animationPlayer->SetIsLooping(true);
			prop.animationPlayer->Play();
		}

	}
}

void AnimatedPropUpdater::Update(float aDeltaTime)
{
	for (auto& prop : myInteractableProps)
	{
		if (prop.animationPlayer->IsValid())
		{
			prop.animationPlayer->Update(aDeltaTime);
		}
	}
	for (auto& prop : myLoopingProps)
	{
		if (prop.animationPlayer->IsValid())
		{
			prop.animationPlayer->Update(aDeltaTime);
		}
	}
}

void AnimatedPropUpdater::Render()
{
	Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::GraphicsEngine& graphicsEngine = engine.GetGraphicsEngine();
	Tga::ModelDrawer& modelDrawer = graphicsEngine.GetModelDrawer();

	for (auto& prop : myInteractableProps)
	{
		if (prop.animationPlayer->IsValid())
		{
			if (prop.animationPlayer->GetState() == Tga::AnimationState::Finished && !prop.oneTimeTriggerable)
			{
				prop.animationPlayer->SetTime(0.0f); // Might not be necessary later on
			}

			prop.animatedModelInstance->SetPose(*prop.animationPlayer);
			modelDrawer.Draw(*prop.animatedModelInstance);
		}
	}

	for (auto& prop : myLoopingProps)
	{
		if (prop.animationPlayer->IsValid())
		{
			prop.animatedModelInstance->SetPose(*prop.animationPlayer);
			modelDrawer.Draw(*prop.animatedModelInstance);
		}
	}
}

void AnimatedPropUpdater::PlayAnimation(unsigned int aIndex)
{
	if (myInteractableProps[aIndex].animationPlayer->GetState() == Tga::AnimationState::Playing)
	{
		return;
	}
	else if(myInteractableProps[aIndex].animationPlayer->GetState() == Tga::AnimationState::Finished &&
		myInteractableProps[aIndex].oneTimeTriggerable)
	{
		return;
	}
	else
	{
		myInteractableProps[aIndex].animationPlayer->Stop();
		myInteractableProps[aIndex].animationPlayer->Play();
	}
}