#include "ShotgunRaycast.h"

#include "MathUtils.h"

namespace GameStateUpdate
{
	void ShotgunRaycast(Player& aPlayer, const std::vector<SceneLoader::TileConfig>& aTiles,
	                    std::vector<Enemy>& aEnemies, std::vector<CrateUpdater::Crate>& aCrates,
	                    CrateUpdater& aCrateUpdater, FlipbookManager* aFlipbookManager,
	                    FlipbookManager::FlipbookHandle anEnvironmentHit, FlipbookManager::FlipbookHandle aCrateHit,
	                    FlipbookManager::FlipbookHandle aMetalCrateHit, FlipbookManager::FlipbookHandle anEnemyHit)
	{
		const int numberOfShots = aPlayer.GetShotgunBulletAmount();

		std::vector<Physics::CollisionResult> rayAndEnemy = Physics::RaycastConeAABBCollisionOverContainer<Enemy>(
			Physics::Ray{
				.origin = {aPlayer.GetPosition().x, aPlayer.GetPosition().y + 50.f},
				.direction = aPlayer.GetNormalizedShotgunAim(),
				.magnitude = aPlayer.GetShotgunRange()
			},
			numberOfShots,
			aPlayer.GetShotgunSpreadAngle(),
			aEnemies,
			[](const Enemy& enemy)
			{
				if (enemy.GetIsAlive())
				{
					return Physics::AABB{
						.position = enemy.GetPosition(),
						.velocity = Tga::Vector2f{ 0.0f, 0.0f },
						.size = Tga::Vector2f{ 100.0f, 100.0f }

					};
				}
				else
				{
					return Physics::AABB{
						.position = enemy.GetPosition(),
						.velocity = Tga::Vector2f{ 0.0f, 0.0f },
						.size = Tga::Vector2f{ 0.f, 0.0f }
					};
				}
			}
		);

		std::vector<Physics::CollisionResult> rayAndCrate = Physics::RaycastConeAABBCollisionOverContainer<CrateUpdater::Crate>(
			Physics::Ray{
				.origin = {aPlayer.GetPosition().x, aPlayer.GetPosition().y + 50.f},
				.direction = aPlayer.GetNormalizedShotgunAim(),
				.magnitude = aPlayer.GetShotgunRange()
			},
			numberOfShots,
			aPlayer.GetShotgunSpreadAngle(),
			aCrates,
			[](const CrateUpdater::Crate& crate)
			{
				return Physics::AABB{
					.position = crate.position,
					.size = crate.dead ? Tga::Vector2f() : crate.size
				};
			}
		);

		std::vector<Physics::CollisionResult> rayAndTile = Physics::RaycastConeAABBCollisionOverContainer<SceneLoader::TileConfig>(
			Physics::Ray{
				.origin = {aPlayer.GetPosition().x, aPlayer.GetPosition().y + 50.f},
				.direction = aPlayer.GetNormalizedShotgunAim(),
				.magnitude = aPlayer.GetShotgunRange()
			},
			numberOfShots,
			aPlayer.GetShotgunSpreadAngle(),
			aTiles,
			[](const SceneLoader::TileConfig& tile)
			{
				return Physics::AABB{
					.position = tile.position,
					.size = tile.size
				};
			}
		);

		const Tga::Vector2f forward{1.f, 0.f};
		Tga::Vector2f aimDir = aPlayer.GetNormalizedShotgunAim();
		const float originalAngle = std::atan2f(forward.Cross(aimDir), forward.Dot(aimDir));
		float spreadAngle = aPlayer.GetShotgunSpreadAngle();
		float deltaAngle = 0;
		float startAngle;
		const float environmentHitAngleOffset = 0.25f;
		const float randomizationAngle = 0.5f;
		
		if (numberOfShots != 1)
		{
			deltaAngle = spreadAngle / static_cast<float>(numberOfShots - 1);
			startAngle = originalAngle - spreadAngle / 2;
		}
		else
		{
			startAngle = originalAngle;
		}
		
		for (int i = 0; i < numberOfShots; ++i)
		{
			
			Physics::CollisionResult* revolverCollisions[]{ &rayAndEnemy[i], &rayAndTile[i], &rayAndCrate[i] };
			Physics::CollisionResult* revolverClosestCollisionResult{ &rayAndEnemy[i] };

			for (Physics::CollisionResult* result : revolverCollisions)
			{
				if (result->pointOfCollisionAlongVelocity < revolverClosestCollisionResult->pointOfCollisionAlongVelocity)
				{
					revolverClosestCollisionResult = result;
				}
			}
			
			aimDir =
			{
				std::cos(startAngle + deltaAngle * i),
				std::sin(startAngle + deltaAngle * i)
			};
			
			if (Physics::AreCollisionResultsEqual(revolverClosestCollisionResult, &rayAndEnemy[i]) && rayAndEnemy[i].didCollide)
			{
				// We hit an enemy
				Enemy& enemy = aEnemies[rayAndEnemy[i].indexToEntityCollidedWith];
				if (enemy.GetIsAlive())
				{
					enemy.Kill();
					aPlayer.Reload();
					aFlipbookManager->PlayAt(anEnemyHit, Tga::Vector2f{ enemy.GetPosition().x, enemy.GetPosition().y + 50.f }, randomizationAngle * MathUtils::RandFloat(-1, 1) + startAngle + deltaAngle * i);
				}
			}
			
			if (Physics::AreCollisionResultsEqual(revolverClosestCollisionResult, &rayAndTile[i]) && rayAndTile[i].didCollide)
			{
				// We hit a tile
				aFlipbookManager->PlayAt(anEnvironmentHit, aPlayer.GetShotOrigin() + aimDir * aPlayer.GetShotgunRange() * revolverClosestCollisionResult->pointOfCollisionAlongVelocity, environmentHitAngleOffset + randomizationAngle * MathUtils::RandFloat(-1, 1) + startAngle + deltaAngle * i);
			}
			
			if (Physics::AreCollisionResultsEqual(revolverClosestCollisionResult, &rayAndCrate[i]) && rayAndCrate[i].didCollide)
			{
				// We hit a crate
				if (!aCrateUpdater.GetCrates()[rayAndCrate[i].indexToEntityCollidedWith].metal)
				{
					aCrateUpdater.DeactivateCrate(rayAndCrate[i].indexToEntityCollidedWith);
					aFlipbookManager->PlayAt(aCrateHit, aPlayer.GetShotOrigin() + aimDir * aPlayer.GetShotgunRange() * revolverClosestCollisionResult->pointOfCollisionAlongVelocity, randomizationAngle * MathUtils::RandFloat(-1, 1) + startAngle + deltaAngle * i);
				}
				else
				{
					if (aCrateUpdater.GetTimeSinceLastShotgunBulletHit() >= aCrateUpdater.GetTimeToAllowShotgunBulletHitSfx())
					{
						AudioManager::GetAudioPoolByHandle(AudioHandles::bulletHittingIronCrate).Play();
						aCrateUpdater.ShotgunBulletHit();
					}
					aFlipbookManager->PlayAt(aMetalCrateHit, aPlayer.GetShotOrigin() + aimDir * aPlayer.GetShotgunRange() * revolverClosestCollisionResult->pointOfCollisionAlongVelocity, randomizationAngle * MathUtils::RandFloat(-1, 1) + startAngle + deltaAngle * i);
				}
			}
		}
		//std::erase_if(aCrates, [](const CrateUpdater::Crate& aCrate) { return aCrate.dead; });
	}
}
