#include <stdafx.h>
#include "AdjustAnimationSpeedNode.h"

#include <tge/animation/Animation.h>
#include <tge/animation/AnimationClip.h>
#include <tge/animation/AnimationPlayer.h>
#include <tge/animation/PoseGenerator.h>
#include <tge/model/ModelFactory.h>

#include <tge/scene/ScenePropertyTypes.h>
#include <tge/script/BaseProperties.h>

using namespace Tga;

struct AdjustAnimationSpeedGenerator : public PoseGenerator
{
	uint32_t lastUpdatedFrame = (uint32_t)-1;
	PoseGenerator* generator;
	float speedScale;
	
	void GeneratePose(PoseGenerationContext& context, LocalSpacePose& outputPose) override
	{
		if (generator == nullptr)
		{
			PoseGenerator::GeneratePose(context, outputPose);
			return;
		}

		PoseGenerationContext contextCopy = context;
		contextCopy.deltaTime *= speedScale;

		generator->GeneratePose(contextCopy, outputPose);

	}

	void GenerateRootMotionDelta(PoseGenerationContext& context, Vector3f& outRootMotionPositionDelta, Quatf& outRootMotionRotationDelta) override
	{
		if (generator == nullptr)
		{
			PoseGenerator::GenerateRootMotionDelta(context, outRootMotionPositionDelta, outRootMotionRotationDelta);
			return;
		}

		PoseGenerationContext contextCopy = context;
		contextCopy.deltaTime /= speedScale;

		generator->GenerateRootMotionDelta(contextCopy, outRootMotionPositionDelta, outRootMotionRotationDelta);
	}
};

struct AdjustAnimationSpeedRuntimeInstance : public ScriptNodeRuntimeInstanceBase
{
	AdjustAnimationSpeedGenerator generator;
};


void AdjustAnimationSpeedNode::Init(const ScriptCreationContext& context)
{
	{
		ScriptPin valuePin = {};
		valuePin.type = ScriptLinkType::Property;
		valuePin.dataType = GetPropertyType<PoseAndMotion>();
		valuePin.defaultValue = Property::Create<PoseAndMotion>();
		valuePin.name = "Pose"_tgaid;
		valuePin.node = context.GetNodeId();
		valuePin.role = ScriptPinRole::Input;

		myPoseInPin = context.FindOrCreatePin(valuePin);
	}

	{
		ScriptPin valuePin = {};
		valuePin.type = ScriptLinkType::Property;
		valuePin.dataType = GetPropertyType<float>();
		valuePin.defaultValue = Property::Create<float>(1.f);
		valuePin.name = "Speed Scale"_tgaid;
		valuePin.node = context.GetNodeId();
		valuePin.role = ScriptPinRole::Input;

		mySpeedScalePin = context.FindOrCreatePin(valuePin);
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


std::unique_ptr<ScriptNodeRuntimeInstanceBase> AdjustAnimationSpeedNode::CreateRuntimeInstanceData() const
{
	return std::make_unique<AdjustAnimationSpeedRuntimeInstance>();
}

Property AdjustAnimationSpeedNode::ReadPin(ScriptExecutionContext& context, ScriptPinId) const
{
	AdjustAnimationSpeedGenerator& generator = static_cast<AdjustAnimationSpeedRuntimeInstance*>(context.GetRuntimeInstanceData())->generator;

	PoseAndMotion input = *context.ReadInputPin(myPoseInPin).Get<PoseAndMotion>();

	float speedScale = *context.ReadInputPin(mySpeedScalePin).Get<float>();

	generator.generator = input.poseGenerator;
	generator.speedScale = speedScale;
	
	PoseAndMotion result = {};

	result.poseGenerator = &generator;

	result.desiredSyncedPlaybackRate = input.desiredSyncedPlaybackRate * speedScale;
	result.desiredSyncedPlaybackRateWeight = input.desiredSyncedPlaybackRateWeight;
	
	return Property::Create<PoseAndMotion>(result);
}