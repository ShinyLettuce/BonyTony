#include "RevolverRaycast.h"

#include "MathUtils.h"

namespace GameStateUpdate
{
	void RevolverRaycast(const std::vector<SceneLoader::TileConfig>& aTiles, std::vector<Enemy>& aEnemies,
	                     std::vector<CrateUpdater::Crate>& aCrates, CrateUpdater& aCrateUpdater, Player& aPlayer,
	                     FlipbookManager* aFlipbookManager, FlipbookManager::FlipbookHandle anEnvironmentHit,
	                     FlipbookManager::FlipbookHandle aCrateHit, FlipbookManager::FlipbookHandle aMetalCrateHit,
	                     FlipbookManager::FlipbookHandle anEnemyHit)
	{
		if (!aPlayer.GetIsRevolverEnabled())
		{
			return;
		}

		Physics::CollisionResult rayAndEnemy = Physics::RaycastAABBCollisionOverContainer<Enemy>(
			Physics::Ray{
				.origin = aPlayer.GetShotOrigin(),
				.direction = aPlayer.GetNormalizedRevolverAim(),
				.magnitude = aPlayer.GetRevolverRange(),
			},
			aEnemies,
			[](const Enemy& enemy)
			{
				if (enemy.GetIsAlive())
				{
					return Physics::AABB{
						.position = enemy.GetPosition(),
						.velocity = Tga::Vector2f{ 0.0f, 0.0f },
						.size = Tga::Vector2f{ 150.0f, 150.0f }

					};
				}
				else
				{
					return Physics::AABB{
						.position = enemy.GetPosition(),
						.velocity = Tga::Vector2f{ 0.0f, 0.0f },
						.size = Tga::Vector2f{ 0.f, 0.f }
					};
				}
			}
		);

		Physics::CollisionResult rayAndTile = Physics::RaycastAABBCollisionOverContainer<SceneLoader::TileConfig>(
			Physics::Ray{
				.origin = aPlayer.GetShotOrigin(),
				.direction = aPlayer.GetNormalizedRevolverAim(),
				.magnitude = aPlayer.GetRevolverRange(),
			},
			aTiles,
			[](const SceneLoader::TileConfig& tile)
			{
				return Physics::AABB{
					.position = tile.position,
					.velocity = Tga::Vector2f{ 0.0f, 0.0f },
					.size = tile.size
				};
			}
		);

		Physics::CollisionResult rayAndCrate = Physics::RaycastAABBCollisionOverContainer<CrateUpdater::Crate>(
			Physics::Ray{
				.origin = aPlayer.GetShotOrigin(),
				.direction = aPlayer.GetNormalizedRevolverAim(),
				.magnitude = aPlayer.GetRevolverRange(),
			},
			aCrates,
			[](const CrateUpdater::Crate& crate)
			{
				return Physics::AABB{
					.position = crate.position,
					.size = crate.dead ? Tga::Vector2f() : crate.size
				};
			}
		);
		Physics::CollisionResult* revolverCollisions[]{ &rayAndEnemy, &rayAndTile, &rayAndCrate };

		Physics::CollisionResult* revolverClosestCollisionResult{ &rayAndEnemy };
		for (Physics::CollisionResult* result : revolverCollisions)
		{
			if (result->pointOfCollisionAlongVelocity < revolverClosestCollisionResult->pointOfCollisionAlongVelocity)
			{
				revolverClosestCollisionResult = result;
			}
		}

		//For playing the flipbook
		const Tga::Vector2f forward{ 1.f, 0.f };
		const Tga::Vector2f aimDir = aPlayer.GetNormalizedRevolverAim();
		const float angle = std::atan2f(forward.Cross(aimDir), forward.Dot(aimDir));
		const float randomizationAngle = 0.5f;
		const float environmentHitAngleOffset = 0.25f;
		
		if (Physics::AreCollisionResultsEqual(revolverClosestCollisionResult, &rayAndEnemy) && rayAndEnemy.didCollide)
		{
			// We hit an enemy

			//aFlipbookManager->PlayAt(anEnemyHit, );
			
			Enemy& enemy = aEnemies[rayAndEnemy.indexToEntityCollidedWith];
			if (enemy.GetIsAlive())
			{
				enemy.Kill();
				aPlayer.Reload();
				//myTimer->BulletTime(0.2f, 1.f, 20.f, 30.f);
				aFlipbookManager->PlayAt(anEnemyHit, Tga::Vector2f{ enemy.GetPosition().x, enemy.GetPosition().y + 50.f }, angle + randomizationAngle * MathUtils::RandFloat(-1, 1));
			}
			
		}
		if (Physics::AreCollisionResultsEqual(revolverClosestCollisionResult, &rayAndTile) && rayAndTile.didCollide)
		{
			// We hit a tile
			aFlipbookManager->PlayAt(anEnvironmentHit, aPlayer.GetShotOrigin() + aimDir * aPlayer.GetRevolverRange() * revolverClosestCollisionResult->pointOfCollisionAlongVelocity, environmentHitAngleOffset + angle + randomizationAngle * MathUtils::RandFloat(-1, 1));
		}
		if (Physics::AreCollisionResultsEqual(revolverClosestCollisionResult, &rayAndCrate) && rayAndCrate.didCollide)
		{
			// We hit a crate
			if (!aCrateUpdater.GetCrates()[rayAndCrate.indexToEntityCollidedWith].metal)
			{
				aCrateUpdater.DeactivateCrate(rayAndCrate.indexToEntityCollidedWith);
				aFlipbookManager->PlayAt(aCrateHit, aPlayer.GetShotOrigin() + aimDir * aPlayer.GetRevolverRange() * revolverClosestCollisionResult->pointOfCollisionAlongVelocity, angle + randomizationAngle * MathUtils::RandFloat(-1, 1));
			}
			else
			{
				AudioManager::GetAudioPoolByHandle(AudioHandles::bulletHittingIronCrate).Play();
				aFlipbookManager->PlayAt(aMetalCrateHit, aPlayer.GetShotOrigin() + aimDir * aPlayer.GetRevolverRange() * revolverClosestCollisionResult->pointOfCollisionAlongVelocity, angle + randomizationAngle * MathUtils::RandFloat(-1, 1));
			}
		}
		//std::erase_if(aCrates, [](const CrateUpdater::Crate& aCrate) { return aCrate.dead; });
	}
}
