#pragma once
#include <tge/animation/AnimationPlayer.h>
#include <tge/model/AnimatedModelInstance.h>
#include "SceneLoader.h"

class AnimatedPropUpdater
{
public:
	struct AnimatedProp
	{
		std::shared_ptr<Tga::AnimationPlayer> animationPlayer;
		std::shared_ptr<Tga::AnimatedModelInstance> animatedModelInstance;
		unsigned int index;
		Tga::Vector2f size = {0,0};
		bool oneTimeTriggerable = false;
	};

	void Init(std::vector<SceneLoader::AnimatedPropConfig> aConfigs);
	std::vector<AnimatedProp>& GetInteractableProps() { return myInteractableProps; };
	void Update(float aDeltaTime);
	void Render();
	void PlayAnimation(unsigned int aPropIndex);

private:
	std::vector<AnimatedProp> myInteractableProps;
	std::vector<AnimatedProp> myLoopingProps;
};