#pragma once

#include <vector>
#include <tge/math/Vector.h>

#include "AudioManager.h"
#include "FlipbookManager.h"
#include "tge/model/ModelInstance.h"
#include "tge/animation/AnimationPlayer.h"
#include "tge/model/AnimatedModelInstance.h"


namespace SceneLoader
{
	struct EnemyConfig;
	struct EnemySharedConfig;
}

class Projectile;
class Player;

class Enemy
{
	public:
		void Init(const SceneLoader::EnemyConfig& aEnemyConfig, const SceneLoader::EnemySharedConfig* aSharedConfig,
		          std::vector<Projectile>* aProjectileCollection,
		          FlipbookManager* aFlipbookManager, FlipbookManager::FlipbookHandle* aFireMeleeFlipbookHandle,
		          FlipbookManager::FlipbookHandle* aFireRevolverFlipbookHandle);
		void Update(const float aDeltaTime, const Tga::Vector2f aPlayerPosition);
		void Render();

		void AnimateWeapon(float aDeltaTime);

		void PerformMelee();
		void Kill();

		void SetLineOfSight(const bool aBool);
		float GetDetectionRange() const;
		float GetDetectionAngle() const;
		Tga::Vector2f GetFaceDirection() const;
		Tga::Vector2f GetViewPosition() const;

		bool GetIsAlive() const;
		void SetCanRespawn(bool aBool);
		bool GetHasGun() const;

		void UpdateImGui();

		Tga::Vector2f GetPosition() const;
		Tga::Vector2f GetSize() const;
		Tga::Vector2f GetKnockbackVelocity() const;

	private:

		struct EnemyFlipBookData
		{
			FlipbookManager* aFlipbookManager;
			FlipbookManager::FlipbookHandle* aFireMeleeHandle;
			FlipbookManager::FlipbookHandle* aFireRevolverHandle;
		};
		
		enum class EnemyState
		{
			Idle,
			MeleeAttack,
			Shoot,
			Death
		} myAnimationState;

		std::shared_ptr<Tga::AnimatedModelInstance> myModelInstance;
		std::shared_ptr<Tga::AnimatedModelInstance> myModelInstanceDeath;
		std::shared_ptr<Tga::AnimatedModelInstance> myModelInstanceNoHand;
		Tga::ModelInstance myProjectileModelInstance;
		Tga::ModelInstance myWeaponModelInstance;

		std::vector<Projectile>* myProjectileCollection = nullptr;

		Tga::Vector2f myAimPoint{};
		Tga::Vector2f myPosition{};
		Tga::Vector2f mySize = { 100.f, 100.f };
		Tga::Vector2f myShotSpawnPosition;
		Tga::Vector2f myKnockbackVelocity;
		Tga::Vector2f myNormalizedAim;

		const SceneLoader::EnemySharedConfig* mySharedConfig = nullptr;
		EnemyFlipBookData myFlipbookData;

		std::shared_ptr<Tga::AnimationPlayer> myIdleAnimation;
		std::shared_ptr<Tga::AnimationPlayer> myDeathAnimation;
		std::shared_ptr<Tga::AnimationPlayer> myReviveAnimation;
		std::shared_ptr<Tga::AnimationPlayer> myMeleeAnimation;

		float myTimer = 0.f;
		float myTimeSincePreviousAim = 0.f;
		float myShouldPlayReviveSfxTimer = 0.f;
		float myShouldPlayReviveSfxDelay = 1.f;
		bool myShouldPlayReviveSfx = false;

		bool myIsAlive = true;
		bool myHasGun = false;
		bool myLineOfSight = false;
		bool myCanRespawn = true;


		bool myIsBobbingUp = true;
		bool myIsAimingRight = true;

		float myAnimationTime = 0;
		float myRevolverAngle = 0;
		float myRevolverDistance = -10.0f;
		float myRevolverRightOffset = 50.0f;
		float myRevolverDecayFactor = 16.0f;
		float myWeaponForwardOffset = -34.0f;
		float myAnimationSpeed = 4.0f;
		float myRevolverRestingOffset = 25.0f;
		float myBobbingOffset = 0.0f;
		float myBobbingMaxOffset = 3.f;
		float myBobbingSpeed = 12.5f;
};
