#pragma once

#include "State.h"
#include "ResolutionManager.h"

#include <vector>
#include <memory>

class StateStack
{
public:
	void Initialize(StateHandle aStateHandle);

	bool Empty() const;

	void RegisterState(StateHandle aStateHandle, State* aState);

	StateHandle MakeRegistryHandle();

	void Update();
	void Render();
private:
	std::vector<State*> myStates;
	std::vector<StateHandle> myStateHandles;

	Tga::Vector2f myLastResolution;
};