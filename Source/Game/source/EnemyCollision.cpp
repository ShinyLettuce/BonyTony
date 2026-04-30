#include "EnemyCollision.h"

#include <tge/math/Matrix2x2.h>

namespace GameStateUpdate
{
	void EnemyCollision(std::vector<Enemy>& aEnemies, Player& aPlayer, const std::vector<SceneLoader::TileConfig>& aTiles)
	{
		for (Enemy& enemy : aEnemies)
		{
			//physical collision with player
			const bool enemyAndPlayerCollide = Physics::AABBCollision(
				Physics::AABB
				{
					.position = aPlayer.GetPosition(),
					.size = aPlayer.GetSize()
				},
				Physics::AABB
				{
					.position = enemy.GetPosition(),
					.velocity = Tga::Vector2f{ 0.0f, 0.0f },
					.size = enemy.GetSize()
				}
			);
			
			enemy.SetCanRespawn(true);

			if (enemyAndPlayerCollide)
			{
				if (!enemy.GetIsAlive())
				{
					enemy.SetCanRespawn(false);
					continue;
				}
				if (!aPlayer.GetPlayerStunned())
				{
					aPlayer.StunPlayer(enemy.GetKnockbackVelocity());
					enemy.PerformMelee();
				}
			}
			
			if (!enemy.GetIsAlive())
			{
				continue;
			}

			if (!enemy.GetHasGun())
			{
				continue;
			}

			//sight checks
			Tga::Vector2f enemyToPlayer = aPlayer.GetPosition() - enemy.GetPosition();
			float enemyToPlayerLength = enemyToPlayer.Length();

			if (enemyToPlayerLength <= enemy.GetDetectionRange())
			{
				//detection angle check
				Tga::Matrix2x2f rotationUp = Tga::Matrix2x2f::CreateFromRotation(FMath::DegToRad * enemy.GetDetectionAngle() * 0.5f);
				Tga::Matrix2x2f rotationDown = Tga::Matrix2x2f::CreateFromRotation(FMath::DegToRad * -(enemy.GetDetectionAngle() * 0.5f));

				Physics::Ray peripheralUp
				{
					.origin = enemy.GetViewPosition(),
					.direction = enemy.GetFaceDirection() * rotationUp,
					.magnitude = 1.f,
				};

				Physics::Ray peripheralDown
				{
					.origin = enemy.GetViewPosition(),
					.direction = -1.f * (enemy.GetFaceDirection() * rotationDown),
					.magnitude = 1.f,
				};

				bool outSideOfDetectionAngle
				{
					Physics::GetRayNormal(peripheralUp).Dot(aPlayer.GetPosition() - enemy.GetViewPosition()) > 0.f ||
					Physics::GetRayNormal(peripheralDown).Dot(aPlayer.GetPosition() - enemy.GetViewPosition()) > 0.f
				};

				if (outSideOfDetectionAngle)
				{
					enemy.SetLineOfSight(false);
					continue;
				}

				//tile collision check

				const Physics::CollisionResult enemySightToTiles =
					Physics::RaycastAABBCollisionOverContainer<SceneLoader::TileConfig>(
						Physics::Ray
						{
							.origin = enemy.GetViewPosition(),
							.direction = enemyToPlayer.GetNormalized(),
							.magnitude = enemyToPlayerLength,
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
						}
					);

				enemy.SetLineOfSight(!enemySightToTiles.didCollide);
			}
			else
			{
				enemy.SetLineOfSight(false);
			}
		}
	}
}