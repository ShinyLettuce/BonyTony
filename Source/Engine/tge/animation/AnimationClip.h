#pragma once

#include <tge/script/CopyOnWriteWrapper.h>
#include <tge/animation/Pose.h>
#include <tge/animation/Skeleton.h>
#include <tge/stringRegistry/StringRegistry.h>

namespace Tga
{
	struct AnimationClip
	{
		StringId animationSourcePath;
		StringId selectedAnimationName;

		StringId previewModelPath;
		
		float startTime;
		float endTime;

		bool isLooping;

		bool isSyncronized;
		float cycleOffsetPercentage;
		float cycleCount = 1.f;
	};

	AnimationClip* GetAnimationClip(StringId path);
	AnimationClip* GetOrCreateAnimationClip(StringId path);
	void SaveAnimationClip(StringId path);
}