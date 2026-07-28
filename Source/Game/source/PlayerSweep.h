#pragma once
#include "Physics.h"
#include "SceneLoader.h"
#include "Player.h"
#include "CrateUpdater.h"

#include <vector>

#include "FlipbookManager.h"
#include "InteractablePropUpdater.h"

namespace GameStateUpdate
{
	void PlayerSweep(
		const std::vector<SceneLoader::TileConfig>& aTiles,
		std::vector<CrateUpdater::Crate>& aCrates,
		CrateUpdater& aCrateUpdater, Player& aPlayer,
		PlayerUpdateResult& aPlayerUpdateResult,
		FlipbookManager* aFlipbookManager,
		AnimatedPropUpdater& aPropUpdater,
		const float aDeltaTime,
		const float aTickRate
	);
}
