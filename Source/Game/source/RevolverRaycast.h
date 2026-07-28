#pragma once
#include "Player.h"
#include "Physics.h"
#include "CrateUpdater.h"
#include "Enemy.h"
#include <vector>

#include "FlipbookManager.h"
#include "InteractablePropUpdater.h"


namespace GameStateUpdate
{
	void RevolverRaycast(const std::vector<SceneLoader::TileConfig>& aTiles, std::vector<Enemy>& aEnemies,
	                     std::vector<CrateUpdater::Crate>& aCrates, CrateUpdater& aCrateUpdater, Player& aPlayer,
	                     FlipbookManager* aFlipbookManager, AnimatedPropUpdater& aPropUpdater); // why does it take crate updater and crates if one contains the other?? - Jacob
}
