#include <stdafx.h>
#include "PlayClipNode.h"

#include <tge/animation/Animation.h>
#include <tge/animation/AnimationClip.h>
#include <tge/animation/AnimationPlayer.h>
#include <tge/animation/PoseGenerator.h>
#include <tge/model/ModelFactory.h>

#include <tge/scene/ScenePropertyTypes.h>

using namespace Tga;

struct PlayClipGenerator : public PoseGenerator
{
	uint32_t lastUpdatedFrame = (uint32_t)-1;
	AnimationClip* clip = nullptr;
	AnimationPlayer animationPlayer;

	float lastSyncLocation = 0.f;

	bool EnsureLoadedAndUpdated(PoseGenerationContext& context)
	{
		if (!clip)
			return false;

		if (lastUpdatedFrame == (uint32_t)-1)
		{
			animationPlayer = ModelFactory::GetInstance().GetAnimationPlayer(clip->animationSourcePath.GetString(), context.model);
		}

		if (!animationPlayer.GetAnimation())
			return false;

		if (clip->isSyncronized)
		{
			float factor = context.syncedPlaybackTime + clip->cycleOffsetPercentage;
			factor = factor - floor(factor);
			
			// closest new value for lastSyncLocation, larger than previous value, but with correct factor
			float count = ceil(lastSyncLocation - factor);
			lastSyncLocation = count + factor;

			if (clip->isLooping && clip->cycleCount > 0.f)
			{
				while (lastSyncLocation > clip->cycleCount)
				{
					lastSyncLocation -= clip->cycleCount;
				}
			}
			else
			{
				lastSyncLocation = std::min(lastSyncLocation, clip->cycleCount);
			}

			animationPlayer.SetTime(clip->startTime + (clip->endTime - clip->startTime) * lastSyncLocation / clip->cycleCount );
		}
		else
		{
			if (lastUpdatedFrame == (uint32_t)-1)
			{
				animationPlayer.SetTime(clip->startTime);
			}
			else
			{
				float newTime = animationPlayer.GetTime();
				newTime += context.deltaTime;

				if (clip->isLooping && clip->endTime > clip->startTime)
				{
					while (newTime > clip->endTime)
					{
						newTime -= clip->endTime - clip->startTime;
					}
				}
				else
				{
					newTime = std::min(newTime, clip->endTime);
				}

				animationPlayer.SetTime(newTime);
			}
		}

		lastUpdatedFrame = context.frameNumber;

		animationPlayer.UpdatePose();

		return true;
	}
	void GeneratePose(PoseGenerationContext& context, LocalSpacePose& outputPose) override
	{
		if (!EnsureLoadedAndUpdated(context))
			return; 

		outputPose = animationPlayer.GetLocalSpacePose();
	}

	void GenerateRootMotionDelta(PoseGenerationContext& context, Vector3f& outRootMotionPositionDelta, Quatf& outRootMotionRotationDelta) override
	{
		if (!EnsureLoadedAndUpdated(context))
			return;

		outRootMotionPositionDelta = Vector3f{};
		outRootMotionRotationDelta = Quatf{};
	}
};

struct PlayClipRuntimeInstance : public ScriptNodeRuntimeInstanceBase
{
	PlayClipGenerator generator;
};

void PlayClipNode::Init(const ScriptCreationContext& context)
{
	{
		ScriptPin namePin = {};
		namePin.type = ScriptLinkType::Property;
		namePin.dataType = GetPropertyType<CopyOnWriteWrapper<AnimationClipReference>>();
		namePin.defaultValue = Property::Create<CopyOnWriteWrapper<AnimationClipReference>>();
		namePin.name = "Clip"_tgaid;
		namePin.node = context.GetNodeId();
		namePin.role = ScriptPinRole::Input;
		myAnimationClipInPin = context.FindOrCreatePin(namePin);
	}

	{
		ScriptPin outputPin = {};
		outputPin.type = ScriptLinkType::Property;
		outputPin.dataType = GetPropertyType<PoseAndMotion>();
		outputPin.name = "Pose"_tgaid;
		outputPin.node = context.GetNodeId();
		outputPin.role = ScriptPinRole::Output;

		myPoseOutPin = context.FindOrCreatePin(outputPin);
	}
}

std::unique_ptr<ScriptNodeRuntimeInstanceBase> PlayClipNode::CreateRuntimeInstanceData() const
{
	return std::make_unique<PlayClipRuntimeInstance>();
}

Property PlayClipNode::ReadPin(ScriptExecutionContext& context, ScriptPinId) const
{
	PlayClipGenerator& generator = static_cast<PlayClipRuntimeInstance*>(context.GetRuntimeInstanceData())->generator;

	if (generator.clip == nullptr)
	{
		StringId path = context.ReadInputPin(myAnimationClipInPin).Get<CopyOnWriteWrapper<AnimationClipReference>>()->Get().path;
		generator.clip = GetAnimationClip(path);
	}

	PoseAndMotion result = {};
	
	if (generator.clip && generator.clip->isSyncronized)
	{
		result.desiredSyncedPlaybackRateWeight = 1.f;
		result.desiredSyncedPlaybackRate = 1.f / (generator.clip->endTime - generator.clip->startTime);
	}

	result.poseGenerator = &generator;
	

	return Property::Create<PoseAndMotion>(result);
}