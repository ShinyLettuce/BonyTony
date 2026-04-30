#pragma once

#include "Timer.h"
#include "InputMapper.h"
#include <tge/drawers/SpriteDrawer.h>
#include <tge/drawers/ModelDrawer.h>

using StateHandle = unsigned int;

class StateUpdateResult
{
public:
	enum class Type
	{
		NONE,
		PUSH,
		POP,
		CLEAR,
		CLEAR_AND_PUSH
	};

	static StateUpdateResult CreateContinue();
	static StateUpdateResult CreatePush(StateHandle aStateHandle);
	static StateUpdateResult CreatePop();
	static StateUpdateResult CreateClear();
	static StateUpdateResult CreateClearAndPush(StateHandle aStateHandle);

	StateHandle GetStateHandle() const;

	Type GetType() const;
private:
	StateUpdateResult(StateHandle aStateHandle, Type aType);

	StateHandle myStateHandle;
	Type myType;
};

class State
{
public:
	virtual ~State() = default;

	virtual StateUpdateResult Update() = 0;
	virtual void Render() = 0;
	virtual void OnPush(){}
	virtual void OnPop(){}
	virtual void OnGainFocus(){}
	virtual void OnLoseFocus(){}
	virtual void OnResolutionChange(){}
};
