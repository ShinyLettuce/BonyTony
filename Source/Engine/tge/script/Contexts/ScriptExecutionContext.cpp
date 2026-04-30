#include <stdafx.h>

#include <tge/script/Contexts/ScriptExecutionContext.h>
#include <tge/script/Script.h>
#include <tge/script/ScriptRuntimeInstance.h>

using namespace Tga;

const ScriptUpdateContext& ScriptExecutionContext::GetUpdateContext()
{
	return myUpdateContext;
}

ScriptExecutionContext::ScriptExecutionContext(ScriptRuntimeInstance& scriptRuntimeInstance, const ScriptUpdateContext& updateContext, ScriptNodeId nodeId, ScriptNodeRuntimeInstanceBase* nodeRuntimeInstance)
	: myScriptRuntimeInstance(scriptRuntimeInstance)
	, myUpdateContext(updateContext)
	, myNodeId(nodeId)
	, myNodeRuntimeInstance(nodeRuntimeInstance)
{}

ScriptExecutionContext::~ScriptExecutionContext()
{
	const Script& script = myScriptRuntimeInstance.GetScript();

	for (int i = 0; i < myTriggeredOutputCount; i++)
	{
		ScriptPinId pinId = myTriggeredOutputQueue[i];

		size_t count;
		const ScriptLinkId* linkIds = script.GetConnectedLinks(pinId, count);

		assert("Trying to trigger an output pin that isn't of type flow" && script.GetPin(pinId).type == ScriptLinkType::Flow);
		assert("Only one link allowed on Flow out pins" && count <= 1);

		if (count == 0)
			return;

		const ScriptLink& link = script.GetLink(linkIds[0]);
		ScriptPinId targetPinId = link.targetPinId;
		const ScriptPin& targetPin = script.GetPin(targetPinId);

		ScriptNodeId nodeId = targetPin.node;

		ScriptExecutionContext executionContext(myScriptRuntimeInstance, myUpdateContext, nodeId, myScriptRuntimeInstance.GetRuntimeInstance(nodeId));

		const ScriptNodeBase& node = script.GetNode(nodeId);

		ScriptNodeResult result = node.Execute(executionContext, targetPinId);
		if (result == ScriptNodeResult::KeepRunning)
		{
			myScriptRuntimeInstance.ActivateNode(nodeId);
		}
		else
		{
			myScriptRuntimeInstance.DeactivateNode(nodeId);
		}
	}
}

void ScriptExecutionContext::TriggerOutputPin(ScriptPinId pinId)
{
	assert(myTriggeredOutputCount < MAX_TRIGGERED_OUTPUTS);
	
	if (myTriggeredOutputCount < MAX_TRIGGERED_OUTPUTS)
	{
		myTriggeredOutputQueue[myTriggeredOutputCount] = pinId;
		myTriggeredOutputCount++;
	}
}

Property ScriptExecutionContext::ReadInputPin(ScriptPinId pinId)
{
	const Script& script = myScriptRuntimeInstance.GetScript();

	size_t count;
	const ScriptLinkId* linkIds = script.GetConnectedLinks(pinId, count);

	assert("Trying to read from a flow pin" && script.GetPin(pinId).type != ScriptLinkType::Flow);
	assert("Trying to read from a pin with unknown type" && script.GetPin(pinId).type != ScriptLinkType::Unknown);
	assert("Only one link allowed on value input pins" && count <= 1);

	if (count == 0)
	{
		const ScriptPin& pin = script.GetPin(pinId);
		if (pin.overridenValue.GetType() != nullptr)
			return pin.overridenValue;

		return pin.defaultValue;
	}

	const ScriptLink& link = script.GetLink(linkIds[0]);
	ScriptPinId sourcePinId = link.sourcePinId;
	const ScriptPin& sourcePin = script.GetPin(sourcePinId);

	ScriptNodeId nodeId = sourcePin.node;

	ScriptExecutionContext executionContext(*this);
	executionContext.myNodeId = nodeId;
	executionContext.myNodeRuntimeInstance = myScriptRuntimeInstance.GetRuntimeInstance(nodeId);

	const ScriptNodeBase& node = script.GetNode(nodeId);

	return node.ReadPin(executionContext, sourcePinId);
}


ScriptNodeRuntimeInstanceBase* ScriptExecutionContext::GetRuntimeInstanceData()
{
	return myNodeRuntimeInstance;
}
