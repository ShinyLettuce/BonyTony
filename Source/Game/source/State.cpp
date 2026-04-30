#include "State.h"

StateUpdateResult StateUpdateResult::CreateContinue()
{
	StateUpdateResult stateUpdateResult{ 0u, Type::NONE };
	return stateUpdateResult;
}

StateUpdateResult StateUpdateResult::CreatePush(StateHandle aStateHandle)
{
	StateUpdateResult stateUpdateResult{ aStateHandle, Type::PUSH };
	return stateUpdateResult;
}

StateUpdateResult StateUpdateResult::CreatePop()
{
	StateUpdateResult stateUpdateResult{ 0u, Type::POP };
	return stateUpdateResult;
}

StateUpdateResult StateUpdateResult::CreateClear()
{
	StateUpdateResult stateUpdateResult{ 0u, Type::CLEAR };
	return stateUpdateResult;
}

StateUpdateResult StateUpdateResult::CreateClearAndPush(StateHandle aStateHandle)
{
	StateUpdateResult stateUpdateResult{ aStateHandle, Type::CLEAR_AND_PUSH };
	return stateUpdateResult;
}

StateHandle StateUpdateResult::GetStateHandle() const
{
	return myStateHandle;
}

StateUpdateResult::Type StateUpdateResult::GetType() const
{
	return myType;
}

StateUpdateResult::StateUpdateResult(StateHandle aStateHandle, Type aType)
	: myStateHandle{ aStateHandle }, myType{ aType }
{
}
