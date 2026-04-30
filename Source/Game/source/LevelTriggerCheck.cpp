#include "LevelTriggerCheck.h"

bool GameStateUpdate::PlayerLevelTriggerCheck(const Player& aPlayer, LevelTrigger& aLevelTrigger)
{
	if (!aLevelTrigger.GetExists())
	{
		return false;
	}

	bool collided = Physics::AABBCollision
	(
		Physics::AABB
		{
			.position = aPlayer.GetPosition(),
			.velocity = aPlayer.GetVelocity(),
			.size = {100.f, 100.f}
		},
		Physics::AABB
		{
			.position = aLevelTrigger.GetPosition(),
			.velocity = {0.f, 0.f},
			.size = aLevelTrigger.GetSize()
		}
	);
	
	if (collided && aLevelTrigger.GetActive())
	{
		std::cout << "[LevelTriggerCheck.cpp] Collision, SceneChange Triggered]\n";
		return true;                                     
	}
	
	return false;
}
