#include "GroundCheck.h"

namespace GameStateUpdate
{
	void PlayerGroundCheck(const std::vector<CrateUpdater::Crate>& aCrates, Player& aPlayer, PlayerUpdateResult& aPlayerUpdateResult, const std::vector<SceneLoader::TileConfig>& aTileConfigs)
	{
		Tga::Vector2f downUnitVector{ 0.f, -1.f };
		float groundRayMagnitude = 1.f;

		if (aPlayerUpdateResult.velocity.y > 0.f)
		{
			aPlayer.SetGrounded(false); 
			return;
		}

		Physics::CollisionResult playerTileRayDownLeft = Physics::RaycastAABBCollisionOverContainer<SceneLoader::TileConfig>(
			Physics::Ray{
				.origin = {aPlayer.GetPosition().x - 25.f, aPlayer.GetPosition().y - 1.f},
				.direction = downUnitVector,
				.magnitude = groundRayMagnitude
			},
			aTileConfigs,
			[](const SceneLoader::TileConfig& tile) {
				return Physics::AABB{
					.position = tile.position,
					.size = tile.size
				};
			}
		);

		Physics::CollisionResult playerCrateRayDownLeft = Physics::RaycastAABBCollisionOverContainer<CrateUpdater::Crate>(
			Physics::Ray{
				.origin = {aPlayer.GetPosition().x - 25.f, aPlayer.GetPosition().y - 1.f},
				.direction = downUnitVector,
				.magnitude = groundRayMagnitude
			},
			aCrates,
			[](const CrateUpdater::Crate& crate) {
				return Physics::AABB{
					.position = crate.position,
					.size = crate.dead? Tga::Vector2f(): crate.size
				};
			}
		);

		Physics::CollisionResult playerTileRayDownRight = Physics::RaycastAABBCollisionOverContainer<SceneLoader::TileConfig>(
			Physics::Ray{
				.origin = {aPlayer.GetPosition().x + 25.f, aPlayer.GetPosition().y - 1.f},
				.direction = downUnitVector,
				.magnitude = groundRayMagnitude
			},
			aTileConfigs,
			[](const SceneLoader::TileConfig& tile) {
				return Physics::AABB{
					.position = tile.position,
					.size = tile.size
				};
			}
		);

		Physics::CollisionResult playerCrateRayDownRight = Physics::RaycastAABBCollisionOverContainer<CrateUpdater::Crate>(
			Physics::Ray{
				.origin = {aPlayer.GetPosition().x + 25.f, aPlayer.GetPosition().y - 1.f},
				.direction = downUnitVector,
				.magnitude = groundRayMagnitude
			},
			aCrates,
			[](const CrateUpdater::Crate& crate) {
				return Physics::AABB{
					.position = crate.position,
					.size = crate.dead ? Tga::Vector2f() : crate.size
				};
			}
		);

		Physics::CollisionResult groundRayLeftResult{};
		const bool isTileCloserLeft = playerTileRayDownLeft.pointOfCollisionAlongVelocity < playerCrateRayDownLeft.pointOfCollisionAlongVelocity;
		groundRayLeftResult = isTileCloserLeft ? playerTileRayDownLeft : playerCrateRayDownLeft;

		Physics::CollisionResult groundRayRightResult{};
		const bool isTileCloserRight = playerTileRayDownRight.pointOfCollisionAlongVelocity < playerCrateRayDownRight.pointOfCollisionAlongVelocity;
		groundRayRightResult = isTileCloserRight ? playerTileRayDownRight : playerCrateRayDownRight;

		aPlayer.SetGrounded(false);

		if (groundRayLeftResult.didCollide)
		{
			aPlayer.SetGrounded(true);
			aPlayerUpdateResult.position += downUnitVector * groundRayMagnitude * groundRayLeftResult.pointOfCollisionAlongVelocity * 0.99f;
		}
		else if (groundRayRightResult.didCollide)
		{
			aPlayer.SetGrounded(true);
			aPlayerUpdateResult.position += downUnitVector * groundRayMagnitude * groundRayRightResult.pointOfCollisionAlongVelocity * 0.99f;
		}
	}
}