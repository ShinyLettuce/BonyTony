#pragma once
#include "Player.h"
#include "Physics.h"
#include "CrateUpdater.h"
#include "SceneLoader.h"
#include "Enemy.h"

#include <vector>

#include "FlipbookManager.h"


namespace GameStateUpdate
{
	void ShotgunRaycast(Player& aPlayer, const std::vector<SceneLoader::TileConfig>& aTiles,
	                    std::vector<Enemy>& aEnemies,
	                    std::vector<CrateUpdater::Crate>& aCrates, CrateUpdater& aCrateUpdater,
	                    FlipbookManager* aFlipbookManager, FlipbookManager::FlipbookHandle anEnvironmentHit,
	                    FlipbookManager::FlipbookHandle aCrateHit, FlipbookManager::FlipbookHandle aMetalCrateHit,
	                    FlipbookManager::FlipbookHandle anEnemyHit);
}
