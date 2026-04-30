#include "StateStack.h"

void StateStack::Initialize(StateHandle aStateHandle)
{
	while (!myStateHandles.empty())
	{
		StateHandle stateHandle = myStateHandles.back();
		myStateHandles.pop_back();
		State* state = myStates.at(stateHandle);
		state->OnLoseFocus();
		state->OnPop();
	}

	myStateHandles.push_back(aStateHandle);

	ResolutionManager::Update();
	myLastResolution = ResolutionManager::GetCurrentResolution();
}

bool StateStack::Empty() const
{
	return myStateHandles.empty();
}

void StateStack::RegisterState(StateHandle aStateHandle, State* aState)
{
	myStates.at(aStateHandle) = aState;
}

StateHandle StateStack::MakeRegistryHandle()
{
	StateHandle stateHandle = static_cast<StateHandle>(myStates.size());
	myStates.emplace_back(nullptr);
	return stateHandle;
}

void StateStack::Update()
{
	if (myStateHandles.empty())
	{
		return;
	}

	ResolutionManager::Update();
	Tga::Vector2f currentResolution = ResolutionManager::GetCurrentResolution();

	if (currentResolution.x != myLastResolution.x || currentResolution.y != myLastResolution.y)
	{
		for (auto handle : myStateHandles)
		{
			State* state = myStates.at(handle);
			state->OnResolutionChange();
		}
		myLastResolution = currentResolution;
	}

	StateHandle currentStateHandle = myStateHandles.back();
	State* currentState = myStates.at(currentStateHandle);
	StateUpdateResult stateUpdateResult = currentState->Update();

	switch (stateUpdateResult.GetType())
	{
		case StateUpdateResult::Type::PUSH:
		{
			StateHandle newStateHandle = stateUpdateResult.GetStateHandle();
			myStateHandles.push_back(newStateHandle);
			State* newState = myStates.at(newStateHandle);
			currentState->OnLoseFocus();
			newState->OnPush();
			newState->OnGainFocus();
			newState->OnResolutionChange();
			break;
		}
		case StateUpdateResult::Type::POP:
		{
			myStateHandles.pop_back();
			StateHandle newStateHandle = myStateHandles.back();
			State* newState = myStates.at(newStateHandle);
			currentState->OnLoseFocus();
			currentState->OnPop();
			newState->OnGainFocus();
			break;
		}
		case StateUpdateResult::Type::CLEAR:
		{
			while (!myStateHandles.empty())
			{
				StateHandle stateHandle = myStateHandles.back();
				myStateHandles.pop_back();
				State* state = myStates.at(stateHandle);
				state->OnLoseFocus();
				state->OnPop();
			}
			break;
		}
		case StateUpdateResult::Type::CLEAR_AND_PUSH:
		{
			while (!myStateHandles.empty())
			{
				StateHandle stateHandle = myStateHandles.back();
				myStateHandles.pop_back();
				State* state = myStates.at(stateHandle);
				state->OnLoseFocus();
				state->OnPop();
			}
			StateHandle newStateHandle = stateUpdateResult.GetStateHandle();
			myStateHandles.push_back(newStateHandle);
			State* newState = myStates.at(newStateHandle);
			newState->OnPush();
			newState->OnGainFocus();
			newState->OnResolutionChange();
			break;
		}
	}
}

void StateStack::Render()
{
	if (myStateHandles.empty())
	{
		return;
	}

	for (auto handle : myStateHandles)
	{
		State* state = myStates.at(handle);
		state->Render();
	}
}
