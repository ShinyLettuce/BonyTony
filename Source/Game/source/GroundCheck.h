#pragma once
#include "Player.h"
#include "SceneLoader.h"
#include "CrateUpdater.h"
#include "InteractablePropUpdater.h"

namespace GameStateUpdate
{
	void PlayerGroundCheck(
		const std::vector<CrateUpdater::Crate>& aCrates,
		Player& aPlayer, PlayerUpdateResult& aPlayerUpdateResult,
		const std::vector<SceneLoader::TileConfig>& aTileConfigs,
		AnimatedPropUpdater& aPropUpdater
	);
}

