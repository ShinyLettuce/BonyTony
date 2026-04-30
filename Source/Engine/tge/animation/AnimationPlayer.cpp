#include "stdafx.h"
#include "AnimationPlayer.h"
#include <tge/engine.h>

using namespace Tga;

void AnimationPlayer::Init(const std::shared_ptr<const Animation>& animation)
{
	myAnimation = animation;
	myFPS = animation->FramesPerSecond;
}

void AnimationPlayer::Update(float aDeltaTime)
{
	if (myState == AnimationState::Playing)
	{
		myTime += aDeltaTime;

		if (myTime >= GetAnimation()->Duration)
		{
			if (myIsLooping)
			{
				while (myTime >= GetAnimation()->Duration)
					myTime -= GetAnimation()->Duration;
			}
			else
			{
				myTime = GetAnimation()->Duration;
				myState = AnimationState::Finished;
			}
		}
	}
	UpdatePose();

}

void Tga::AnimationPlayer::UpdatePose()
{
	const float frameRate = 1.0f / myFPS;
	const float result = myTime / frameRate;
	const size_t frame = static_cast<size_t>(std::floor(result));// Which frame we're on
	const float delta = result - static_cast<float>(frame); // How far we have progressed to the next frame.

	size_t nextFrame = frame + 1;
	if (myState == AnimationState::Finished)
	{
		nextFrame = frame;
	}
	else if (nextFrame > GetAnimation()->Length)
		nextFrame = 0;

	int jointCount = (int)myAnimation->Frames[frame].Count;

	// Update all animations
	for (size_t i = 0; i < jointCount; i++)
	{
		const auto& currentFrameJointXform = myAnimation->Frames[frame].JointTransforms[i];
		ScaleRotationTranslationf jointXform = currentFrameJointXform;
		if (myIsInterpolating)
		{
			const ScaleRotationTranslationf& nextFrameJointXform = myAnimation->Frames[nextFrame].JointTransforms[i];

			jointXform = ScaleRotationTranslationf::Lerp(currentFrameJointXform, nextFrameJointXform, delta);
		}

		myLocalSpacePose.JointTransforms[i] = jointXform;
	}
	myLocalSpacePose.Count = jointCount;
}
