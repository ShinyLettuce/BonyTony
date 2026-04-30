#include "Cutscene.h"

namespace Cutscene
{
	static PreparedConfig globalPreparedConfig;

	void Set(PreparedConfig aPreparedConfig)
	{
		globalPreparedConfig = aPreparedConfig;
	}

	PreparedConfig Get()
	{
		return globalPreparedConfig;
	}
}
