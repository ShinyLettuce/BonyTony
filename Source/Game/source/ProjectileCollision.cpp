#include "ProjectileCollision.h"

#include "MathUtils.h"

namespace GameStateUpdate
{
	void ProjectileCollision(Player& aPlayer, std::vector<Projectile>& aProjectiles,
		const std::vector<SceneLoader::TileConfig>& aTiles,
		const std::vector<CrateUpdater::Crate>& aCrates, const float aDeltaTime,
		const float aTickRate, FlipbookManager* aFlipbookManager,
		FlipbookManager::FlipbookHandle anEnvironmentHit, FlipbookManager::FlipbookHandle aCrateHit,
		FlipbookManager::FlipbookHandle aMetalCrateHit, FlipbookManager::FlipbookHandle anEnemyHit)
	{
		const float randomizationAngle = 0.5f;
		const Tga::Vector2f forward{ 1.f, 0.f };

		for (Projectile& projectile : aProjectiles)
		{
			Physics::CollisionResult projectileToTiles =
				Physics::SweepAABBCollisionOverContainer<SceneLoader::TileConfig>(
					Physics::AABB{
						.position = projectile.GetPosition(),
						.velocity = projectile.GetVelocity(),
						.size = projectile.GetSize(),
					},
					aTiles,
					[](const SceneLoader::TileConfig& tile)
					{
						return Physics::AABB
						{
							.position = tile.position,
							.velocity = Tga::Vector2f{ 0.0f, 0.0f },
							.size = tile.size
						};
					},
					aDeltaTime * aTickRate
				);


			if (projectileToTiles.didCollide)
			{
				AudioManager::GetAudioPoolByHandle(AudioHandles::bulletHittingWallOrFloor).Play();
				projectile.Hit();
				const Tga::Vector2f dir = projectileToTiles.normal * -1.f;
				const float angle = std::atan2f(forward.Cross(dir), forward.Dot(dir));
				aFlipbookManager->PlayAt(anEnvironmentHit, projectile.GetPosition() + dir * projectile.GetVelocity().Length() * projectileToTiles.pointOfCollisionAlongVelocity, angle + randomizationAngle * MathUtils::RandFloat(-1, 1));
			}
		}

		for (Projectile& projectile : aProjectiles)
		{
			Physics::CollisionResult projectileToCrates =
				Physics::SweepAABBCollisionOverContainer<CrateUpdater::Crate>(
					Physics::AABB{
						.position = projectile.GetPosition(),
						.velocity = projectile.GetVelocity(),
						.size = projectile.GetSize(),
					},
					aCrates,
					[](const CrateUpdater::Crate& crate)
					{
						return Physics::AABB
						{
							.position = crate.position,
							.velocity = Tga::Vector2f{ 0.0f, 0.0f },
							.size = crate.size
						};
					},
					aDeltaTime * aTickRate
				);

			if (projectileToCrates.didCollide)
			{
				const Tga::Vector2f dir = projectileToCrates.normal * -1.f;
				const float angle = std::atan2f(forward.Cross(dir), forward.Dot(dir));
				if (aCrates.at(projectileToCrates.indexToEntityCollidedWith).metal)
				{
					AudioManager::GetAudioPoolByHandle(AudioHandles::bulletHittingIronCrate).Play();
					aFlipbookManager->PlayAt(aMetalCrateHit, projectile.GetPosition() + dir * projectile.GetVelocity().Length() * projectileToCrates.pointOfCollisionAlongVelocity, angle + randomizationAngle * MathUtils::RandFloat(-1, 1));
				}
				else
				{
					AudioManager::GetAudioPoolByHandle(AudioHandles::bulletHittingWallOrFloor).Play();
					aFlipbookManager->PlayAt(aCrateHit, projectile.GetPosition() + dir * projectile.GetVelocity().Length() * projectileToCrates.pointOfCollisionAlongVelocity, angle + randomizationAngle * MathUtils::RandFloat(-1, 1));
				}

				projectile.Hit();

			}
		}

		const Physics::CollisionResult playerToProjectilesCollision =
			Physics::SweepAABBCollisionOverContainer<Projectile>(
				Physics::AABB
				{
					.position = aPlayer.GetPosition(),
					.velocity = aPlayer.GetVelocity(),
					.size = {50.f, 100.f}
				},
				aProjectiles,
				[](const Projectile& projectile)
				{
					return Physics::AABB
					{
						.position = projectile.GetPosition(),
						.velocity = projectile.GetVelocity(),
						.size = projectile.GetSize()
					};
				},
				aDeltaTime
			);

		if (playerToProjectilesCollision.didCollide)
		{
			Projectile& projectile = aProjectiles.at(playerToProjectilesCollision.indexToEntityCollidedWith);

			if (!aPlayer.GetPlayerStunned())
			{
				aPlayer.StunPlayer(projectile.GetKnockbackVelocity());
				projectile.Hit();
				const Tga::Vector2f dir = playerToProjectilesCollision.normal * -1.f;
				const float angle = std::atan2f(forward.Cross(dir), forward.Dot(dir));
				aFlipbookManager->PlayAt(anEnemyHit, projectile.GetPosition() + dir * projectile.GetVelocity().Length() * playerToProjectilesCollision.pointOfCollisionAlongVelocity, angle + randomizationAngle * MathUtils::RandFloat(-1, 1));
			}
		}
	}
}
