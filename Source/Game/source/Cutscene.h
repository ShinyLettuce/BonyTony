#pragma once

#include "AudioManager.h"
#include "State.h"

namespace Cutscene
{
	struct PreparedConfig
	{
		const char* filePath;
		AudioManager::AudioPoolHandle audioHandle;
		StateHandle nextStateHandle;
	};

	void Set(PreparedConfig aPreparedConfig);
	PreparedConfig Get();
}