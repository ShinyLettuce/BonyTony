#pragma once

#include <tge/model/ModelInstance.h>

#include "SceneLoader.h"
#include "Player.h"

#include <vector>

struct Pickup
{
	Tga::ModelInstance modelInstance;
	Tga::Vector2f position;
	Tga::Vector2f size;
	SceneLoader::PickupType type;
	bool isActive = true;
};

class PickupUpdater
{
public:
	void Init(std::vector<SceneLoader::PickupConfig> aPickupConfigs);
	SceneLoader::PickupType Update(Player& aPlayer);
	void Render();
private:
	std::vector<Pickup> myPickups;
};