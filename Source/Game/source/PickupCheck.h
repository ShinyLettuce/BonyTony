#pragma once

#include "Physics.h"
#include "Player.h"
#include "SceneLoader.h"

namespace GameStateUpdate
{
	void PlayerPickupCheck(Player& aPlayer, std::vector<SceneLoader::PickupConfig>& aPickupConfigs);
}