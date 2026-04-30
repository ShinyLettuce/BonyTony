#pragma once
#include "Physics.h"
#include "Enemy.h"
#include "Player.h"
#include "SceneLoader.h"

#include <vector>

namespace GameStateUpdate
{
	void EnemyCollision(std::vector<Enemy>& aEnemies, Player& aPlayer, const std::vector<SceneLoader::TileConfig>& aTiles);
}