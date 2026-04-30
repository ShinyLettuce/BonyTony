#include "Enemy.h"

#include "AudioManager.h"
#include "MathUtils.h"
#include "Physics.h"
#include "Projectile.h"
#include "SceneLoader.h"
#include "imgui/imgui.h"
#include "tge/Engine.h"
#include "tge/drawers/DebugDrawer.h"
#include "tge/drawers/ModelDrawer.h"
#include "tge/graphics/GraphicsEngine.h"

namespace Tga
{
	class ModelDrawer;
}

void Enemy::Init(const SceneLoader::EnemyConfig& aEnemyConfig, const SceneLoader::EnemySharedConfig* aSharedConfig, std::vector<Projectile>* aProjectileCollection, 
	FlipbookManager* aFlipbookManager, FlipbookManager::FlipbookHandle* aFireMeleeFlipbookHandle, FlipbookManager::FlipbookHandle* aFireRevolverFlipbookHandle)
{
	myPosition = aEnemyConfig.position;
	mySize = { 100.f, 100.f };
	myModelInstance = aEnemyConfig.modelInstance;
	myModelInstanceNoHand = aEnemyConfig.modelInstanceNoHand;
	myModelInstanceDeath = aEnemyConfig.modelInstanceDeath;
	myProjectileModelInstance = aEnemyConfig.projectileModelInstance;
	myWeaponModelInstance = aEnemyConfig.weaponModelInstance;
	myHasGun = aEnemyConfig.hasGun;
	myProjectileCollection = aProjectileCollection;
	mySharedConfig = aSharedConfig;

	myIsAlive = true;
	myLineOfSight = false;
	myShotSpawnPosition = { myPosition.x, myPosition.y + 50.f };
	myAimPoint = { myPosition.x, myPosition.y + 50.f };

	myKnockbackVelocity.x = mySharedConfig->knockBackForce.x * myModelInstance->GetTransform().GetRight().x;
	myKnockbackVelocity.y = mySharedConfig->knockBackForce.y;

	Tga::ModelFactory& modelFactory = Tga::ModelFactory::GetInstance();

	if (myHasGun)
	{
		myIdleAnimation = std::make_shared<Tga::AnimationPlayer>(
			modelFactory.GetAnimationPlayer(aEnemyConfig.idleClipReference.path.GetString(), aEnemyConfig.modelInstanceNoHand->GetModel()));
	}
	else
	{
		myIdleAnimation = std::make_shared<Tga::AnimationPlayer>(
			modelFactory.GetAnimationPlayer(aEnemyConfig.idleClipReference.path.GetString(), aEnemyConfig.modelInstance->GetModel()));
	}

	myDeathAnimation = std::make_shared<Tga::AnimationPlayer>(
		modelFactory.GetAnimationPlayer(aEnemyConfig.deathClipReference.path.GetString(), aEnemyConfig.modelInstanceDeath->GetModel()));

	myMeleeAnimation = std::make_shared<Tga::AnimationPlayer>(
		modelFactory.GetAnimationPlayer(aEnemyConfig.meleeClipReference.path.GetString(), aEnemyConfig.modelInstance->GetModel()));

	myIdleAnimation->SetIsLooping(true);
	myIdleAnimation->Stop();
	myIdleAnimation->Play();

	myModelInstance->GetTransform().Rotate(Tga::Vector3f{ 0.0f,0.0f, 0.0f});
	
	myIsAimingRight = myModelInstance->GetTransform().GetRight().x > 0.f;

	myFlipbookData.aFlipbookManager = aFlipbookManager;
	myFlipbookData.aFireMeleeHandle = aFireMeleeFlipbookHandle;
	myFlipbookData.aFireRevolverHandle = aFireRevolverFlipbookHandle;
}

void Enemy::Update(const float aDeltaTime, const Tga::Vector2f aPlayerPosition)
{
	myTimeSincePreviousAim += aDeltaTime;
	myTimer += aDeltaTime;
	
	if (myShouldPlayReviveSfx)
	{
		myShouldPlayReviveSfxTimer += aDeltaTime;
		if (myShouldPlayReviveSfxTimer >= myShouldPlayReviveSfxDelay)
		{
			AudioManager::GetAudioPoolByHandle(AudioHandles::enemyReassemble).Play();
			myShouldPlayReviveSfx = false;
			myShouldPlayReviveSfxTimer = 0.f;
		}
	}
	
	switch (myAnimationState)
	{
		case EnemyState::Idle:
		{
			myIdleAnimation->Update(aDeltaTime);
			myModelInstance->SetPose(myIdleAnimation->GetLocalSpacePose());
			if (myHasGun)
			{
				myModelInstanceNoHand->SetPose(myIdleAnimation->GetLocalSpacePose());
			}
			break;
		}
		case EnemyState::MeleeAttack:
		{
			myMeleeAnimation->Update(aDeltaTime);
			if (myMeleeAnimation->GetState() == Tga::AnimationState::Finished)
			{
				myAnimationState = EnemyState::Idle;
			}

			myModelInstance->SetPose(myMeleeAnimation->GetLocalSpacePose());
			break;
		}
		case EnemyState::Death:
		{
			myDeathAnimation->Update(aDeltaTime);
			myModelInstanceDeath->SetPose(myDeathAnimation->GetLocalSpacePose());
		}
	}

	if (!myIsAlive)
	{
		if (myTimer < mySharedConfig->deathDuration || !myCanRespawn)
		{
			return;
		}
		myTimer = 0.f;
		myIsAlive = true;
		myAnimationState = EnemyState::Idle;
	}

	if (myHasGun)
	{
		// ImGui::Begin("EnemyAnimationVariables");
		//
		// ImGui::DragFloat("myRevolverDistance", &myRevolverDistance);
		// ImGui::DragFloat("myRevolverRightOffset", &myRevolverRightOffset);
		// ImGui::DragFloat("myRevolverDecayFactor", &myRevolverDecayFactor);
		// ImGui::DragFloat("myWeaponForwardOffset", &myWeaponForwardOffset);
		// ImGui::DragFloat("myAnimationSpeed", &myAnimationSpeed);
		// ImGui::DragFloat("myRevolverRestingOffset", &myRevolverRestingOffset);
		// ImGui::DragFloat("myBobbingMaxOffset", &myBobbingMaxOffset);
		// ImGui::DragFloat("myBobbingSpeed", &myBobbingSpeed);
		//
		// ImGui::End();
		
		AnimateWeapon(aDeltaTime);
		
		if (!myLineOfSight)
		{
			return;
		}
		
		myShotSpawnPosition = {myWeaponModelInstance.GetTransform().GetPosition().x , myWeaponModelInstance.GetTransform().GetPosition().y};

		myAimPoint = FMath::Lerp(myAimPoint, aPlayerPosition, mySharedConfig->aimSpeed * aDeltaTime); // This is not framerate independant 
		myNormalizedAim = (myAimPoint - myPosition).GetNormalized();
		myTimeSincePreviousAim = 0.f;

		if (Tga::Vector2f::Distance(myAimPoint, aPlayerPosition) < mySharedConfig->distanceToFire)
		{
			if (myTimer < mySharedConfig->shotCooldown)
			{
				return;
			}

			myTimer = 0.f;
			myProjectileCollection->emplace_back(
				myProjectileModelInstance,
				myShotSpawnPosition,
				myNormalizedAim * mySharedConfig->projectileSpeed,
				mySharedConfig->projectileKnockBackForce
			);
			
			const Tga::Vector2f forward{ 1.f, 0.f };
			const float angle = std::atan2f(forward.Cross(myNormalizedAim), forward.Dot(myNormalizedAim));
			myFlipbookData.aFlipbookManager->PlayAt(*myFlipbookData.aFireRevolverHandle, myShotSpawnPosition + myNormalizedAim * 100.f, Tga::Vector2f{0.4f, 0.4f}, angle);
		}
	}
}

void Enemy::Render()
{
	const Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::GraphicsEngine& graphicsEngine = engine.GetGraphicsEngine();
	Tga::ModelDrawer& modelDrawer = graphicsEngine.GetModelDrawer();
		
	if (myHasGun)
	{
#if defined(_DEBUG)
		engine.GetDebugDrawer().DrawCircle(myAimPoint, 10.f);
#endif
		if (myAnimationState == EnemyState::Idle || myAnimationState == EnemyState::Shoot)
		{
			modelDrawer.Draw(*myModelInstanceNoHand);
			modelDrawer.Draw(myWeaponModelInstance);
		}
		else if(myAnimationState == EnemyState::Death)
		{
			modelDrawer.Draw(*myModelInstanceDeath);
		}
		else 
		{
			modelDrawer.Draw(*myModelInstance.get());
		}
	}
	else
	{
		if (myAnimationState == EnemyState::Death)
		{
			modelDrawer.Draw(*myModelInstanceDeath);
		}
		else
		{
			modelDrawer.Draw(*myModelInstance.get());
		}
	}

	
}

void Enemy::AnimateWeapon(float aDeltaTime)
{	
	float revolverDistance = myRevolverDistance;
	float revolverRightOffset = myRevolverRightOffset;
	float revolverDecayFactor = myRevolverDecayFactor;
	float weaponForwardOffset = myIsAimingRight ? myWeaponForwardOffset : -myWeaponForwardOffset;
	
	Tga::Matrix4x4f& revolverTransform = myWeaponModelInstance.GetTransform();
	
	const Tga::Vector2f pivot = { 0.0f, 50.0f };

	const Tga::Vector2f worldRight = { 1.0f, 0.0f };
	const Tga::Vector2f worldUp = { 0.0f, 1.0f };

	if (myLineOfSight)
	{
		myAnimationTime += aDeltaTime;	
	}
	else
	{
		myAnimationTime -= aDeltaTime;
	}
	
	myAnimationTime = MathUtils::Clamp01(myAnimationTime);
	
	const float animationPercent = MathUtils::EaseInOutBack(MathUtils::Clamp01(myAnimationTime * myAnimationSpeed));

	const float targetAngle = static_cast<float>(myIsAimingRight) * 180.0f;

	myRevolverAngle = MathUtils::Decay(myRevolverAngle, targetAngle, revolverDecayFactor, aDeltaTime);

	const Tga::Vector2f direction = Tga::Vector2f::Lerp(worldUp, myNormalizedAim.GetNormalized(), animationPercent);

	const Tga::Vector3f right = Tga::Vector3f{ direction, 0.0f };
	Tga::Vector3f up = right.Cross(Tga::Vector3f::Forward);

	const Tga::Quaternionf rotation = Tga::Quaternionf{ right, myRevolverAngle };
	up = Tga::Quaternionf::RotateVectorByQuaternion(rotation, up);

	const Tga::Vector3f forward = up.Cross(right);

	const Tga::Vector2f revolverOffset = worldRight * (myIsAimingRight ? revolverRightOffset : -revolverRightOffset);

	Tga::Vector2f startPosition = myPosition + direction * revolverDistance + pivot;
	startPosition.x += myIsAimingRight ? myRevolverRestingOffset : -myRevolverRestingOffset;
	const Tga::Vector2f endPosition = myPosition + pivot + revolverOffset;

	const Tga::Vector2f animationPosition = Tga::Vector2f::Lerp(startPosition, endPosition, animationPercent);
	
	
	if (myBobbingMaxOffset < myBobbingOffset)
	{
		myIsBobbingUp = false;
	}
	if (-myBobbingMaxOffset > myBobbingOffset)
	{
		myIsBobbingUp = true;
	}
	
	myBobbingOffset += (myIsBobbingUp ? myBobbingSpeed : -myBobbingSpeed) * aDeltaTime;
	
	revolverTransform.SetPosition(Tga::Vector3f{ animationPosition.x, animationPosition.y + myBobbingOffset, weaponForwardOffset });
	revolverTransform.SetRight(right);
	revolverTransform.SetForward(forward);
	revolverTransform.SetUp(up);
	revolverTransform.Scale(Tga::Vector3f{ 0.8f, 0.8f, -0.9f });
}

void Enemy::PerformMelee()
{
	std::cout << "[Enemy.cpp] Enemy melee!" << std::endl;

	//TODO: play melee animation
	if (!myHasGun)
	{
		AudioManager::GetAudioPoolByHandle(AudioHandles::batSwingAndHit).Play();
	}
	else
	{
		AudioManager::GetAudioPoolByHandle(AudioHandles::pistolWhipAndHit).Play();
	}

	myAnimationState = EnemyState::MeleeAttack;
	myMeleeAnimation->SetTime(0.0f);
	myMeleeAnimation->Play();
	myFlipbookData.aFlipbookManager->PlayAt(*myFlipbookData.aFireMeleeHandle, myPosition + Tga::Vector2f{0.f, 100.f}, Tga::Vector2f{0.8f, 0.8f}, 0.f, !myIsAimingRight);
}

void Enemy::Kill()
{
	myIsAlive = false;
	myTimer = 0.f;
	AudioManager::GetAudioPoolByHandle(AudioHandles::enemyDeathPoofCloud).Play();
	AudioManager::GetAudioPoolByHandle(AudioHandles::enemyHurt).Play();
	myShouldPlayReviveSfx = true;

	myAnimationState = EnemyState::Death;
	myDeathAnimation->SetTime(0.0f);
	myDeathAnimation->Play();
}

void Enemy::SetLineOfSight(const bool aBool)
{
	if (aBool && !myLineOfSight)
	{
		std::cout << "[Enemy.cpp] Enemy sees player!" << std::endl;
	}
	else if (!aBool && myLineOfSight)
	{
		std::cout << "[Enemy.cpp] Enemy does not see player!" << std::endl;
	}

	myLineOfSight = aBool;
}

float Enemy::GetDetectionRange() const
{
	return mySharedConfig->detectionRange;
}

float Enemy::GetDetectionAngle() const
{
	return mySharedConfig->detectionAngle;
}

Tga::Vector2f Enemy::GetFaceDirection() const
{
	return Tga::Vector2f{ myModelInstance->GetTransform().GetRight().x, myModelInstance->GetTransform().GetRight().y };
}

Tga::Vector2f Enemy::GetViewPosition() const
{
	return Tga::Vector2f{ myPosition.x, myPosition.y + 50.f };
}

bool Enemy::GetIsAlive() const
{
	return myIsAlive;
}

void Enemy::SetCanRespawn(const bool aBool)
{
	myCanRespawn = aBool;
}

bool Enemy::GetHasGun() const
{
	return myHasGun;
}

void Enemy::UpdateImGui()
{
	myKnockbackVelocity.x = mySharedConfig->knockBackForce.x * myModelInstance->GetTransform().GetRight().x;
	myKnockbackVelocity.y = mySharedConfig->knockBackForce.y;
}

Tga::Vector2f Enemy::GetPosition() const
{
	return myPosition;
}

Tga::Vector2f Enemy::GetSize() const
{
	return mySize;
}

Tga::Vector2f Enemy::GetKnockbackVelocity() const
{
	return myKnockbackVelocity;
}
