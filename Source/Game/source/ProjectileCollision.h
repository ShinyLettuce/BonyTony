#pragma once
#include "Player.h"
#include "SceneLoader.h"
#include "CrateUpdater.h"
#include "EnemyUpdater.h"
#include <vector>
#include "FlipbookManager.h"


namespace GameStateUpdate
{
	void ProjectileCollision(Player& aPlayer, std::vector<Projectile>& aProjectiles,
	                         const std::vector<SceneLoader::TileConfig>& aTiles,
	                         const std::vector<CrateUpdater::Crate>& aCrates, const float aDeltaTime,
	                         const float aTickRate, FlipbookManager* aFlipbookManager,
	                         FlipbookManager::FlipbookHandle anEnvironmentHit, FlipbookManager::FlipbookHandle aCrateHit,
	                         FlipbookManager::FlipbookHandle aMetalCrateHit, FlipbookManager::FlipbookHandle anEnemyHit);
}
