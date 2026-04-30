#include "PlayerSweep.h"
#include "AudioManager.h"
#include "MathUtils.h"

namespace GameStateUpdate
{
	void PlayerSweep(const std::vector<SceneLoader::TileConfig>& aTiles, std::vector<CrateUpdater::Crate>& aCrates, CrateUpdater& aCrateUpdater, Player& aPlayer, PlayerUpdateResult& aPlayerUpdateResult, FlipbookManager* aFlipbookManager, FlipbookManager::FlipbookHandle anMetalCrateHit, const float aDeltaTime, const float aTickRate)
	{
		Physics::CollisionResult tileCollisionResult = Physics::SweepAABBCollisionOverContainer<SceneLoader::TileConfig>(
			Physics::AABB{
				.position = aPlayerUpdateResult.position,
				.velocity = aPlayerUpdateResult.velocity,
				.size = {50.f, 98.f},
			},
			aTiles,
			[](const SceneLoader::TileConfig& tile) {
				return Physics::AABB
				{
					.position = tile.position,
					.size = tile.size
				};
			}, aDeltaTime * aTickRate);

		Physics::CollisionResult crateCollisionResult = Physics::SweepAABBCollisionOverContainer<CrateUpdater::Crate>(
			Physics::AABB{
				.position = aPlayerUpdateResult.position,
				.velocity = aPlayerUpdateResult.velocity,
				.size = {50.f, 98.f},
			},
			aCrates,
			[](const CrateUpdater::Crate& crate) {
				return Physics::AABB
				{
					.position = crate.position,
					.size = crate.dead ? Tga::Vector2f() : crate.size
				};
			}, aDeltaTime * aTickRate);

		Physics::CollisionResult collisionResult{};

		const bool isTileCloser = tileCollisionResult.pointOfCollisionAlongVelocity < crateCollisionResult.pointOfCollisionAlongVelocity;
		collisionResult = isTileCloser ? tileCollisionResult : crateCollisionResult;

		if (!isTileCloser && crateCollisionResult.didCollide && aPlayer.GetPowerBreakActive())
		{
			const Tga::Vector2f forward{ 1.f, 0.f };
			const Tga::Vector2f dir = crateCollisionResult.normal;
			const float angle = std::atan2f(forward.Cross(dir), forward.Dot(dir));
			const float randomizationAngle = 0.5f;
			aFlipbookManager->PlayAt(anMetalCrateHit, aPlayerUpdateResult.position + (aPlayerUpdateResult.position - aCrates[collisionResult.indexToEntityCollidedWith].position) * 0.1f , angle + randomizationAngle * MathUtils::RandFloat(-1, 1));
			aCrateUpdater.DeactivateCrate(collisionResult.indexToEntityCollidedWith);

			PlayerSweep(aTiles, aCrates, aCrateUpdater, aPlayer, aPlayerUpdateResult, aFlipbookManager, anMetalCrateHit, aDeltaTime, aTickRate);
			return;
		}
		else if (isTileCloser && tileCollisionResult.didCollide && aPlayer.GetPowerBreakActive())
		{
			aPlayer.DisablePowerBreak();
		}

		if (collisionResult.didCollide && (collisionResult.normal.Dot(aPlayerUpdateResult.velocity) < 0))
		{
			const float landingSoundVelocityCap = 450.f;
			const float ceilingSoundVelocityCap = 450.f;
			const float wallSoundVelocityCap = 450.f;
			
			if (collisionResult.normal.y > 0.f)
			{
				if (abs(aPlayerUpdateResult.velocity.y) > landingSoundVelocityCap)
				{
					AudioManager::GetAudioPoolByHandle(AudioHandles::playerLandingOnGround).Play();
				}
			}
			else if (collisionResult.normal.y < 0.f)
			{
				if (abs(aPlayerUpdateResult.velocity.y) > ceilingSoundVelocityCap)
				{
					AudioManager::GetAudioPoolByHandle(AudioHandles::playerCollideWithWallOrCeiling).Play();
				}
			}
			if (abs(aPlayerUpdateResult.velocity.x) > wallSoundVelocityCap)
			{
				AudioManager::GetAudioPoolByHandle(AudioHandles::playerCollideWithWallOrCeiling).Play();
			}

			if (collisionResult.normal.x != 0.f)
			{
				aPlayerUpdateResult.position = { aPlayerUpdateResult.position + (aPlayerUpdateResult.velocity * collisionResult.pointOfCollisionAlongVelocity * aDeltaTime * aTickRate) + collisionResult.normal * 5.f };
				aPlayerUpdateResult.velocity = { 0,aPlayerUpdateResult.velocity.y };
			}
			else if (collisionResult.normal.y < 0.f)
			{
				aPlayerUpdateResult.position = { aPlayerUpdateResult.position.x + aPlayerUpdateResult.velocity.x * collisionResult.pointOfCollisionAlongVelocity * aDeltaTime * aTickRate, aPlayerUpdateResult.position.y };
				aPlayerUpdateResult.velocity = { aPlayerUpdateResult.velocity.x,0 };
			}
			else if (collisionResult.normal.y > 0.f)
			{
				aPlayerUpdateResult.position = { aPlayerUpdateResult.position + aPlayerUpdateResult.velocity * collisionResult.pointOfCollisionAlongVelocity * aDeltaTime * aTickRate + collisionResult.normal };
				aPlayerUpdateResult.velocity = { aPlayerUpdateResult.velocity.x,0 };
			}
		}
		else
		{
			aPlayerUpdateResult.position = { aPlayerUpdateResult.position + aPlayerUpdateResult.velocity * aDeltaTime * aTickRate };
		}

		aPlayer.SetPosition(aPlayerUpdateResult.position);
		aPlayer.SetVelocity(aPlayerUpdateResult.velocity);
	}
}
