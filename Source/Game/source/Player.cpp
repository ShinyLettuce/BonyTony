#include "Player.h"
#include "Camera.h"
#include "imgui/imgui.h"
#include "tge/Engine.h"
#include "tge/drawers/ModelDrawer.h"
#include "tge/graphics/GraphicsEngine.h"
#include "tge/drawers/LineDrawer.h"

#include <math.h>
#include "MathUtils.h"
#include "Options.h"
#include "imguizmo/GraphEditor.h"

#pragma region Init

void Player::SetInput(InputMapper* anInputMapper)
{
	myInputMapper = anInputMapper;
}

void Player::Init(const SceneLoader::PlayerConfig& aPlayerConfig)
{
	myIsFrozen = false;
	myPlayerArmPivotHeight = aPlayerConfig.playerArmPivotHeight;
	myStunDuration = aPlayerConfig.stunDuration;
	myModelInstance = aPlayerConfig.modelInstance;
	myRevolverModelInstance = aPlayerConfig.revolverModelInstance;
	myShotgunModelInstance = aPlayerConfig.shotgunModelInstance;
	myPosition = aPlayerConfig.position;
	myGroundedGravity = aPlayerConfig.playerGroundedGravity; 
	mySize = aPlayerConfig.size; 
	myGroundedFriction = aPlayerConfig.playerGroundedFriction;
	myAirFriction = aPlayerConfig.playerAirFriction;
	
	myShotgun.enabled = true;
	myShotgun.clip = aPlayerConfig.shotgunData.maxClip;
	myShotgun.maxClip = aPlayerConfig.shotgunData.maxClip;
	myShotgun.range = aPlayerConfig.shotgunData.range;
	myShotgun.timeBetweenShots = aPlayerConfig.shotgunData.timeBetweenShots;
	myShotgun.maxDistance = aPlayerConfig.shotgunData.maxDistance;
	myShotgun.fallTime = aPlayerConfig.shotgunData.fallTime;
	myShotgun.timeToMaxDistance = aPlayerConfig.shotgunData.timeToMaxDistance;
	myShotgun.gravityConstant = aPlayerConfig.shotgunData.gravityConstant;
	myShotgun.ready = true; 
	myShotgunBulletAmount = aPlayerConfig.shotgunBulletAmount;
	myShotgunBulletSpreadAngle = aPlayerConfig.shotgunSpreadAngle;
	myShotgunHangtime = aPlayerConfig.shotgunHangtime;

	myRevolver.enabled = false;
	myRevolver.clip = aPlayerConfig.revolverData.maxClip;
	myRevolver.maxClip = aPlayerConfig.revolverData.maxClip;
	myRevolver.range = aPlayerConfig.revolverData.range;
	myRevolver.timeBetweenShots = aPlayerConfig.revolverData.timeBetweenShots;
	myRevolver.maxDistance = aPlayerConfig.revolverData.maxDistance;
	myRevolver.fallTime = aPlayerConfig.revolverData.fallTime;
	myRevolver.timeToMaxDistance = aPlayerConfig.revolverData.timeToMaxDistance;
	myRevolver.gravityConstant = aPlayerConfig.revolverData.gravityConstant;
	myRevolver.ready = true;
	myRevolverModelInstance.GetTransform().Scale(Tga::Vector3f{ 1.0f, 1.0f, -1.0f });

	myBulletTimeTimeScale = aPlayerConfig.bulletTimeTimeScale;
	myBulletTimeLerpToSpeed = aPlayerConfig.bulletTimeLerpToSpeed;
	myBulletTimeLerpFromSpeed = aPlayerConfig.bulletTimeLerpFromSpeed;
	myTimeInBulletTime= aPlayerConfig.timeInBulletTime;
	
	myPowerShot.enabled = false;
	myPowerShot.clip = aPlayerConfig.powerShotData.maxClip;
	myPowerShot.maxClip = aPlayerConfig.powerShotData.maxClip;
	myPowerShot.range = aPlayerConfig.powerShotData.range;
	myPowerShot.timeBetweenShots = aPlayerConfig.powerShotData.timeBetweenShots;
	myPowerShot.maxDistance = aPlayerConfig.powerShotData.maxDistance;
	myPowerShot.fallTime = aPlayerConfig.powerShotData.fallTime;
	myPowerShot.timeToMaxDistance = aPlayerConfig.powerShotData.timeToMaxDistance;
	myPowerShot.gravityConstant = aPlayerConfig.powerShotData.gravityConstant;
	myPowerShot.ready = true; 
	
	Tga::Engine* engine = Tga::Engine::GetInstance();
	Tga::Vector2ui intResolution = engine->GetRenderSize();
	myScreenResolution = { static_cast<float>(intResolution.x), static_cast<float>(intResolution.y) };

	if (!myModelInstance.IsValid())
	{
		std::cout << "\n[Player.cpp] WARNING: Player has Initialized without Model and will NOT render.\n";
	}

	CalculateVelocity(myShotgun);
}

#pragma endregion

#pragma region Update, Gravity, Friction

PlayerUpdateResult Player::Update(const float aDeltaTime, Camera& aCamera)
{
	PlayerUpdateResult::Action action = PlayerUpdateResult::Action::None;

	myRevolverWasFiredThisFrame = false;
	myShotgunWasFiredThisFrame = false;
	myPowershotWasFiredThisFrame = false;
	myWasHitByEnemyThisFrame = false;

	// Quick and dirty fix so that the player acts differently in the boss room
	if (myFreezeShotgunSoThatItOnlyPointsRightAndCantBeMovedWithMouseOrController)
	{
		if (myInputMapper->IsActionJustActivated(GameAction::PlayerShootShotgun) && myShotgun.ready && myShotgun.clip > 0)
		{
			action = PlayerUpdateResult::Action::Shotgun;
		}

		return PlayerUpdateResult
		{
			.action = action,
			.position = myPosition,
			.velocity = myVelocity
		};
	}

	UpdateNormalizedAim(aCamera);

	#if !defined(_RETAIL)
	ImGuiUpdate(aDeltaTime); //TODO !!! DO NOT SHIP
	#endif
	
	if (myIsFrozen)
	{
		action = PlayerUpdateResult::Action::Stunned;
	}
	else if (myPlayerStunned)
	{
		if (myStunTimer < myStunDuration && !myIsGrounded)
		{
			myStunTimer += aDeltaTime;
		}
		else
		{
			myPlayerStunned = false;
		}

		action = PlayerUpdateResult::Action::Stunned;
	}
	else if (myInputMapper->IsActionJustActivated(GameAction::PlayerShootShotgun) && myShotgun.ready && myShotgun.clip > 0)
	{
		if (myInputMapper->IsActionActive(GameAction::PlayerPowerShotOverride))
		{
			if (myShotgun.clip > 1 && myPowerShot.enabled)
			{
				ShootPowerShot(aCamera);
				action = PlayerUpdateResult::Action::PowerShot;
				myPowershotWasFiredThisFrame = true;
			}
			else
			{
				ShootShotgun(aCamera);
				action = PlayerUpdateResult::Action::Shotgun;
				myShotgunWasFiredThisFrame = true;
			}	
		}
		else 
		{
			ShootShotgun(aCamera);
			action = PlayerUpdateResult::Action::Shotgun;
			myShotgunWasFiredThisFrame = true;
		}
	}
	else if ((myInputMapper->IsActionJustActivated(GameAction::PlayerShootShotgun) && myShotgun.clip < 1))
	{
		AudioManager::GetAudioPoolByHandle(AudioHandles::shotgunShotNoAmmo).Play();
	}
	else if ((myInputMapper->IsActionJustActivated(GameAction::PlayerShootRevolver) || myRevolverTimeSinceInput > 0.f) && myRevolver.clip > 0)
	{
		if (myRevolver.ready)
		{
			ShootRevolver(aCamera);
			action = PlayerUpdateResult::Action::Revolver;
			myRevolverWasFiredThisFrame = true;
			myRevolverTimeSinceInput = 0.f;
		}
		else
		{
			myRevolverTimeSinceInput = REVOLVER_INPUT_BUFFER_TIME;
		}
	}
	else if ((myInputMapper->IsActionJustActivated(GameAction::PlayerShootRevolver) && myRevolver.clip < 1))
	{
		AudioManager::GetAudioPoolByHandle(AudioHandles::revolverShotNoAmmo).Play();
	}
	
	myVelocity.y += GetGravity(aDeltaTime) * aDeltaTime * 0.5f;
	
	//TODO !!! TEMP DO NOT SHIP
	if (myInputMapper->IsActionActive(GameAction::DebugReload))
	{
		Reload(); 
	}

	if (myIsGrounded && myShotgun.ready && myRevolver.ready &&(myShotgun.clip < myShotgun.maxClip || myRevolver.clip < myRevolver.maxClip))
	{
		Reload();
	}
	
	if (!myShotgun.ready)
	{
		if ((myShotgun.timeBetweenShots < myShotgun.currentLockTime))
		{
			myShotgun.ready = true;
		}
		else
		{
			myShotgun.currentLockTime += aDeltaTime;
		}
	}
	
	if (myRevolverTimeSinceInput >= 0.f)
	{
		myRevolverTimeSinceInput -= aDeltaTime;
	}
	myRevolverTimeSincePreviousShot += aDeltaTime;
	if (!myRevolver.ready)
	{
		if ((myRevolver.timeBetweenShots < myRevolver.currentLockTime))
		{
			myRevolver.ready = true;
		}
		else
		{
			myRevolver.currentLockTime += aDeltaTime;
		}
	}

	//This is only used for Breaking metal crates, The powershot is only reliant on the Shotgun to fire
	if (!myPowerShot.ready) 
	{
		if ((myPowerShot.timeBetweenShots < myPowerShot.currentLockTime))
		{
			myPowerShot.ready = true;
		}
		else
		{
			myPowerShot.currentLockTime += aDeltaTime;
		}
	}
	
	DecayXVelocity(aDeltaTime);

	return PlayerUpdateResult { 
		.action = action,
		.position = myPosition,
		.velocity = myVelocity };
}

void Player::LateUpdate(const float aDeltaTime)
{
	AnimateWeapons(aDeltaTime);
	AnimateWobble(aDeltaTime);

	myVelocity.y += GetGravity(aDeltaTime) * aDeltaTime * 0.5f;
}

void Player::CalculateVelocity(const Gun& aGun)
{
	myForceVelocity = (aGun.gravityConstant * aGun.maxDistance) / aGun.timeToMaxDistance;
	myAirResistance = (-aGun.gravityConstant * aGun.maxDistance) / (aGun.timeToMaxDistance * aGun.timeToMaxDistance);
	myFallGravity = (-aGun.gravityConstant * aGun.maxDistance) / (aGun.fallTime * aGun.fallTime);
}

void Player::DecayXVelocity(float aDeltaTime)
{
	myVelocity.x *= pow(myIsGrounded && myPowerShot.ready ? myGroundedFriction : myAirFriction, aDeltaTime * (-(myIsGrounded ? myGroundedFriction : myAirFriction)));
}

void Player::AnimateWeapons(float aDeltaTime)
{
	constexpr float shotgunDistance = 40.0f;
	constexpr float shotgunDecayFactor = 24.0f;
	constexpr float revolverDistance = 80.0f;
	constexpr float revolverRightOffset = 50.0f;
	constexpr float revolverDecayFactor = 16.0f;
	constexpr float animationTimeScale = 3.0f;
	constexpr float animationStartPercent = 0.5f; // Starts the animation after a given percent of time has passed
	constexpr float weaponForwardOffset = -40.0f;

	const Tga::Vector2f pivot = Tga::Vector2f{ 0.0f, 50.0f };

	const Tga::Vector2f worldRight = Tga::Vector2f{ 1.0f, 0.0f };
	const Tga::Vector2f worldUp = Tga::Vector2f{ 0.0f, 1.0f };
	
	Tga::Matrix4x4f& revolverTransform = myRevolverModelInstance.GetTransform();
	if (myRevolver.enabled)
	{
		const float animationTime = ((myRevolverTimeSincePreviousShot * myRevolverTimeSincePreviousShot) - animationStartPercent) * animationTimeScale;
		const float animationPercent = MathUtils::EaseInOutBack(MathUtils::Clamp01(animationTime));

		const bool isAimingRight = myNormalizedRevolverAim.Dot(worldRight) < 0.0f;
		const bool isRevolverAimingRight = myRevolverPreviousShotDirection.Dot(worldRight) < 0.0f;

		const float targetAngle = animationPercent < 1.0f ? (isRevolverAimingRight ? 0.0f : 180.0f) : (isAimingRight ? 180.0f : 0.0f);
		static float currentZAngle = 0.0f;

		currentZAngle = MathUtils::Decay(currentZAngle, targetAngle, revolverDecayFactor, aDeltaTime);

		const Tga::Vector2f direction = Tga::Vector2f::Lerp(myRevolverPreviousShotDirection, worldUp, animationPercent);

		const Tga::Vector3f right = Tga::Vector3f{ direction, 0.0f };
		Tga::Vector3f up = right.Cross(Tga::Vector3f::Forward);

		const Tga::Quaternionf rotation = Tga::Quaternionf{ right, currentZAngle };
		up = Tga::Quaternionf::RotateVectorByQuaternion(rotation, up);

		const Tga::Vector3f forward = up.Cross(right);

		const Tga::Vector2f revolverOffset = worldRight * (isAimingRight ? revolverRightOffset : -revolverRightOffset);

		const Tga::Vector2f startPosition = myPosition + direction * revolverDistance + pivot;
		const Tga::Vector2f endPosition = myPosition + pivot + revolverOffset;

		const Tga::Vector2f position = Tga::Vector2f::Lerp(startPosition, endPosition, animationPercent);

		revolverTransform.SetPosition(Tga::Vector3f{ position, weaponForwardOffset });
		revolverTransform.SetRight(right);
		revolverTransform.SetForward(forward);
		revolverTransform.SetUp(up);
		revolverTransform.Scale(Tga::Vector3f{ 1.0f, 1.0f, -1.0f });
	}

	Tga::Matrix4x4f& shotgunTransform = myShotgunModelInstance.GetTransform();
	if (myShotgun.enabled)
	{
		const bool isAimingRight = myNormalizedShotgunAim.Dot(worldRight) < 0.0f;

		const float targetAngle = isAimingRight ? 0.0f : 180.0f;
		static float currentZAngle = 0.0f;

		currentZAngle = MathUtils::Decay(currentZAngle, targetAngle, shotgunDecayFactor, aDeltaTime);

		const Tga::Vector3f right = Tga::Vector3f{ myNormalizedShotgunAim, 0.0f };
		Tga::Vector3f up = right.Cross(Tga::Vector3f::Forward);

		const Tga::Quaternionf rotation = Tga::Quaternionf{ right, currentZAngle };
		up = Tga::Quaternionf::RotateVectorByQuaternion(rotation, up);

		const Tga::Vector3f forward = up.Cross(right);

		shotgunTransform.SetPosition(Tga::Vector3f{ (myPosition + pivot + (myNormalizedShotgunAim * shotgunDistance * Tga::Vector2f{ 1.0f, 0.7f })), weaponForwardOffset});
		shotgunTransform.SetRight(right);
		shotgunTransform.SetForward(forward);
		shotgunTransform.SetUp(up);
		shotgunTransform.Scale(Tga::Vector3f{ 1.0f, 1.0f, -1.0f });
	}
}

void Player::AnimateWobble(float aDeltaTime)
{
	static float currentZAngle = 0.0f;
	static float currentZVelocity = 0.0f;
	static bool isMovingHorizontally = false;

	constexpr float shotgunZTilt = 22.0f;
	constexpr float revolverYTilt = 17.0f;
	constexpr float powershotYTilt = 30.0f;

	constexpr float hitYTilt = 60.0f;

	constexpr float springForce = 12.0f;
	constexpr float springDamping = 0.2f;

	constexpr float maxVelocity = 1800.0f;

	MathUtils::Spring(currentZAngle, currentZVelocity, aDeltaTime, springForce, springDamping);

	if (myShotgunWasFiredThisFrame)
	{
		const float direction = myVelocity.x < 0.0f ? 1.0f : -1.0f;
		currentZAngle = direction * shotgunZTilt;
	}

	if (myRevolverWasFiredThisFrame)
	{
		const float direction = myVelocity.x < 0.0f ? 1.0f : -1.0f;
		currentZAngle = direction * revolverYTilt;
	}

	if (myPowershotWasFiredThisFrame)
	{
		const float direction = myVelocity.x < 0.0f ? 1.0f : -1.0f;
		currentZAngle = direction * powershotYTilt;
	}

	if (myWasHitByEnemyThisFrame)
	{
		const float direction = myVelocity.x < 0.0f ? 1.0f : -1.0f;
		currentZAngle = direction * hitYTilt;
	}


	constexpr float yTiltDecayFactor = 18.0f;

	static float yTilt = 12.0f;
	static float currentYAngle = 0.0f;

	const Tga::Vector2f worldRight = Tga::Vector2f{ 1.0f, 0.0f };
	const bool isAimingRight = myNormalizedRevolverAim.Dot(worldRight) < 0.0f;

	const float targetYAngle = isAimingRight ? yTilt : -yTilt;

	currentYAngle = MathUtils::Decay(currentYAngle, targetYAngle, yTiltDecayFactor, aDeltaTime);

	myModelInstance.GetTransform().SetRotation(Tga::Vector3f{ 0.0f, currentYAngle, currentZAngle * MathUtils::LerpClamped(0.0f, 1.0f, std::abs(myVelocity.x) / maxVelocity) });
}

float Player::GetGravity([[maybe_unused]]float aDeltaTime)
{
	if (myIsGrounded)
	{
		return myGroundedGravity;
	}

	//Hangtime
	if (myVelocity.y < 0)
	{
		if (myShotgunHangtimeTimer < myShotgunHangtime)
		{
			//TODO: Make lag proofing
			myShotgunHangtimeTimer += aDeltaTime;
			return myAirResistance * 0.5f;
		}
		
		return myFallGravity;
	}
	
	return myAirResistance;
}

#pragma endregion

#pragma region Shoot

void Player::ShootRevolver(Camera& aCamera)
{
	if (myRevolver.enabled)
	{
		myRevolverTimeSincePreviousShot = 0.0f;
		myRevolverPreviousShotDirection = myNormalizedRevolverAim;

		CalculateVelocity(myRevolver);

		aCamera.Shake(3.f, 7.f, 0.4f);

		myInputMapper->AddRumble(0.4f, 0.1f, 0.1f);

		AudioManager::GetAudioPoolByHandle(AudioHandles::revolverShot).Play();

		myRevolver.currentLockTime = 0.f;
		myRevolver.ready = false;
		myRevolver.clip--;

		//Empty Clip sound

		//Update UI
		
		constexpr float minYVelocity = 800.0f;

		if (myVelocity.y < minYVelocity)
		{
			myVelocity.y = minYVelocity;
		}

		constexpr float minXVelocity = 300.0f;

		if (std::abs(myVelocity.x) < minXVelocity)
		{
			const Tga::Vector2f right = Tga::Vector2f{ 1.0f, 0.0f };
			if (myNormalizedRevolverAim.Dot(right) > 0.0f)
			{
				myVelocity.x = -minXVelocity;
			}
			else
			{
				myVelocity.x = minXVelocity;
			}
		}
		
		if (myVelocity.y > 0.f)
		{
			myIsGrounded = false;
		}
		
		//myVelocity = myIsGrounded ? -(1.0f) * (myForceVelocity * myNormalizedAim) : myVelocity + -(1.0f) * (myForceVelocity * myNormalizedAim);
	}
}

void Player::ShootShotgun(Camera& aCamera)
{
	if (myShotgun.enabled)
	{
		CalculateVelocity(myShotgun);

		aCamera.Shake(7.f, 5.f, 0.5f);

		myInputMapper->AddRumble(0.2f, 0.6f, 0.2f);

		AudioManager::GetAudioPoolByHandle(AudioHandles::shotgunShot).Play();

		myShotgun.currentLockTime = 0.f;
		myShotgun.ready = false;
		myShotgun.clip--;
		myShotgunHangtimeTimer = 0;

		//Empty Clip sound

		//Update UI

		myVelocity = (-1.f) * (myForceVelocity * myNormalizedShotgunAim);

		if (myVelocity.y > 0.f)
		{
			myIsGrounded = false;
		}
	}
}

void Player::ShootPowerShot(Camera& aCamera)
{
	if (myPowerShot.enabled)
	{
		CalculateVelocity(myPowerShot);

		aCamera.Shake(10.f, 10.f, 0.4f);

		myInputMapper->AddRumble(1.f, 0.2f, 0.2f);

		AudioManager::GetAudioPoolByHandle(AudioHandles::shotGunPowerShot).Play();

		myShotgun.currentLockTime = 0.f;
		myShotgun.ready = false;
		myPowerShot.currentLockTime = 0.f;
		myPowerShot.ready = false;
		myShotgun.clip = 0; 

		//Empty Clip sound

		//Update UI

		myVelocity = (-1.f) * (myForceVelocity * myNormalizedShotgunAim);

		if (myVelocity.y > 0.f)
		{
			myIsGrounded = false;
		}
	}
}

// Tga::Vector2f Player::GetNormalizedAim() const
// {
// 	return myNormalizedAim;
// }

Tga::Vector2f Player::GetNormalizedRevolverAim() const
{
	return myNormalizedRevolverAim;
}

Tga::Vector2f Player::GetNormalizedShotgunAim() const
{
	return myNormalizedShotgunAim;
}

void Player::UpdateNormalizedAim(Camera& aCamera)
{
	if (myInputMapper->GetIsUsingMouse())
	{
		Tga::Vector3f mouseToWorld = aCamera.GetScreenToWorldPoint(myInputMapper->GetMousePositionYDown());
		myNormalizedShotgunAim = (Tga::Vector2f (mouseToWorld.x, mouseToWorld.y) - GetShotOrigin()).GetNormalized();
		myNormalizedRevolverAim = myNormalizedShotgunAim;
	}
	else
	{
		if (Options::shotgunOnRS)
		{
			myNormalizedShotgunAim = myInputMapper->GetRightStickPosition().GetNormalized();
			myNormalizedRevolverAim = Options::enableDualStick ? myInputMapper->GetLeftStickPosition().GetNormalized() : myNormalizedShotgunAim;
		}
		else
		{
			myNormalizedShotgunAim = myInputMapper->GetLeftStickPosition().GetNormalized();
			myNormalizedRevolverAim = Options::enableDualStick ? myInputMapper->GetRightStickPosition().GetNormalized() : myNormalizedShotgunAim;
		}
	}
}

Tga::Vector2f Player::GetShotOrigin() const
{
	return Tga::Vector2f { myPosition.x, myPosition.y + myPlayerArmPivotHeight };
}

Tga::Vector2f Player::GetShotgunBarrelEndPoint() const
{
	return GetShotOrigin();
}

#pragma endregion

#pragma region Setters

void Player::OnCollision(const Physics::CollisionResult& aCollisionResult)
{
	//TODO: check to play collision audio here according to the type the object collided with has and velocity
	if (aCollisionResult.didCollide)
	{
		myVelocity *= aCollisionResult.pointOfCollisionAlongVelocity;
	}
}

void Player::StunPlayer(Tga::Vector2f aKnockbackVelocity)
{
	myVelocity = aKnockbackVelocity;
	myStunTimer = 0.0f;
	myPlayerStunned = true;
	AudioManager::GetAudioPoolByHandle(AudioHandles::playerHurt).Play();
	myWasHitByEnemyThisFrame = true;
}

void Player::Reload()
{
	myShotgun.clip = myShotgun.maxClip;
	myRevolver.clip = myRevolver.maxClip;

	AudioManager::GetAudioPoolByHandle(AudioHandles::reload).Play();
	//Update UI
}

void Player::StopVelocityX()
{
	myVelocity.x = 0;
}

void Player::StopVelocityY()
{
	myVelocity.y = 0;
}

void Player::SetGrounded(bool aIsGroundedBool)
{
	myIsGrounded = aIsGroundedBool;
}

void Player::SetPosition(const Tga::Vector2f& aPosition)
{
	myPosition = aPosition;
	Tga::Matrix4x4f& instanceTransform = myModelInstance.GetTransform();
	instanceTransform(4,1) = myPosition.x;
	instanceTransform(4,2) = myPosition.y;
}

void Player::SetVelocity(const Tga::Vector2f& aVelocity)
{
	myVelocity = aVelocity;
}

void Player::SetFrozen(bool aIsFrozen)
{
	myIsFrozen = aIsFrozen;
}

#pragma endregion

#pragma region Getters

float Player::GetShotgunSpreadAngle() const
{
	return myShotgunBulletSpreadAngle;
}

int Player::GetShotgunBulletAmount() const
{
	return myShotgunBulletAmount;
}

float Player::GetShotgunRange() const
{
	return myShotgun.range;
}

float Player::GetRevolverRange() const
{
	return myRevolver.range;
}

Tga::Vector2f Player::GetRevolverPosition() const
{
	Tga::Vector2f worldRight = { 1.0f, 0.0f };
	return GetShotOrigin() + worldRight * (myNormalizedRevolverAim.Dot(worldRight) < 0.0 ? myPlayerArmPivotHeight : -myPlayerArmPivotHeight);;
}

Tga::Vector2f Player::GetShotgunPosition() const
{
	Tga::Vector3f shotgunPosition = myShotgunModelInstance.GetTransform().GetPosition();
	return {shotgunPosition.x, shotgunPosition.y};
}

Tga::Vector2f Player::GetPosition() const
{
	return myPosition;
}

Tga::Vector2f Player::GetSize() const
{
	return mySize;
}

Tga::Vector2f Player::GetVelocity() const
{
	return myVelocity;
}

bool Player::GetPlayerStunned() const
{
	return myPlayerStunned;
}

bool Player::GetGrounded() const
{
	return myIsGrounded;
}

int Player::GetShotgunClip() const
{
	return myShotgun.clip;
}

int Player::GetShotgunMaxClip() const
{
	return myShotgun.maxClip;
}

int Player::GetRevolverClip() const
{
	return myRevolver.clip;
}

int Player::GetRevolverMaxClip() const
{
	return myRevolver.maxClip;
}

void Player::EnableShotgun()
{
	myShotgun.enabled = true;
}

void Player::EnableRevolver()
{
	myRevolver.enabled = true;
}

void Player::EnablePowershot()
{
	myPowerShot.enabled = true;
}

void Player::DisableShotgun()
{
	myShotgun.enabled = false;
}

void Player::DisableRevolver()
{
	myRevolver.enabled = false;
}

void Player::DisablePowershot()
{
	myPowerShot.enabled = false;
}

bool Player::GetIsShotgunEnabled() const
{
	return myShotgun.enabled;
}

bool Player::GetIsRevolverEnabled() const
{
	return myRevolver.enabled;
}

bool Player::GetIsPowershotEnabled() const
{
	return myPowerShot.enabled;
}

bool Player::GetPowerBreakActive() const
{
	return !myPowerShot.ready;
}

void Player::DisablePowerBreak()
{
	myPowerShot.currentLockTime = myPowerShot.timeBetweenShots;
}

void Player::FreezeTheShotgunSoThatItCanOnlyPointRightAndCantBeMovedWithMouseOrControllerOrAnyOtherInputDeviceForThatMatter()
{
	myFreezeShotgunSoThatItOnlyPointsRightAndCantBeMovedWithMouseOrController = true;

	myNormalizedShotgunAim = Tga::Vector2f{ std::cos(Tga::DegToRad(14.0f)), std::sin(Tga::DegToRad(14.0f)) };
}

bool Player::GetIsRevolverReady() const
{
	return myRevolver.ready;
}

float Player::GetShotgunTimeBetweenShots() const
{
	return myShotgun.timeBetweenShots;
}

#pragma endregion

#pragma region BulletTime

float Player::GetTimeInBulletTime() const
{
	return myTimeInBulletTime;
}

float Player::GetBulletTimeTimeScale() const
{
	return myBulletTimeTimeScale;
}

float Player::GetBulletTimeLerpToSpeed() const
{
	return myBulletTimeLerpToSpeed;
}

float Player::GetBulletTimeLerpFromSpeed() const
{
	return myBulletTimeLerpFromSpeed;
}

#pragma endregion

void Player::ImGuiUpdate([[maybe_unused]]float aDeltaTime)
{
	ImGui::Begin("Player");
	//all code here
	
	if (ImGui::Button("Teleport to end"))
	{
		SceneLoader::SceneConfig sceneConfig = SceneLoader::GetActiveScene();
		myPosition = sceneConfig.levelTriggerConfig.position;
		myPosition.x += 200.0f;
	}

	if (ImGui::Button("Toggle revolver"))
	{
		myRevolver.enabled = !myRevolver.enabled;
	}

	if (ImGui::Button("Toggle powershot"))
	{
		myPowerShot.enabled = !myPowerShot.enabled;
	}

	ImGui::Text("\nPlayer");
	ImGui::DragFloat("Grounded Gravity", &myGroundedGravity);
	ImGui::DragFloat("Grounded Friction in %0/second", &myGroundedFriction);
	ImGui::DragFloat("Air Friction in %0/second", &myAirFriction);

	ImGui::Text("\nBulletTime");
	ImGui::DragFloat("Bullet Time TimeScale (Must be: > 0)", &myBulletTimeTimeScale);
	ImGui::DragFloat("Bullet Time Lerp To Speed", &myBulletTimeLerpToSpeed);
	ImGui::DragFloat("Bullet Time Lerp From Speed", &myBulletTimeLerpFromSpeed);
	ImGui::DragFloat("Bullet Time In Bullet time", &myTimeInBulletTime);
	
	ImGui::Text("\nShotgun");
	ImGui::DragFloat("Shotgun Gravity", &myShotgun.gravityConstant);
	ImGui::DragFloat("Shotgun FallTime", &myShotgun.fallTime);
	ImGui::DragFloat("Shotgun Knockback Distance", &myShotgun.maxDistance);
	ImGui::DragFloat("Shotgun Knockback Duration", &myShotgun.timeToMaxDistance);
	ImGui::DragFloat("Shotgun Hangtime", &myShotgunHangtime);
	
	ImGui::Text("\nRevolver");
	ImGui::DragFloat("Revolver Gravity", &myRevolver.gravityConstant);
	ImGui::DragFloat("Revolver FallTime", &myRevolver.fallTime);
	ImGui::DragFloat("Revolver Knockback Distance", &myRevolver.maxDistance);
	ImGui::DragFloat("Revolver Knockback Duration", &myRevolver.timeToMaxDistance);
	
	ImGui::Text("\nPowerShot");
	ImGui::DragFloat("Power Shot Gravity", &myPowerShot.gravityConstant);
	ImGui::DragFloat("Power Shot FallTime", &myPowerShot.fallTime);
	ImGui::DragFloat("Power Shot Knockback Distance", &myPowerShot.maxDistance);
	ImGui::DragFloat("Power Shot Knockback Duration", &myPowerShot.timeToMaxDistance);
	ImGui::DragFloat("Power Shot BreakPower Duration", &myPowerShot.timeBetweenShots);
	
	ImGui::Text("\nRealtime Calculated Variables");
	ImGui::Text("Air Resistance: %f", myAirResistance);
	ImGui::Text("Fall Gravity: %f", myFallGravity);
	ImGui::Text("Force: %f", myForceVelocity);
	ImGui::Text("X Velocity: %.3f", myVelocity.x / 100.0f);
	ImGui::Text("Y Velocity: %.3f", myVelocity.y / 100.0f);
	ImGui::Text("Friction Calc: %f", (1 - 1/(myIsGrounded ? myGroundedFriction : myAirFriction)));
	ImGui::Text("Grounded: %s", (myIsGrounded ? "true" : "false"));
	ImGui::Text("Fall Gravity: %s", (myShotgunHangtimeTimer > myShotgunHangtime ? "true" : "false"));
	ImGui::Text("ShotGunHangtime Timer: %f", myShotgunHangtimeTimer);
	
	ImGui::End();
	
}

void Player::Render()
{
	Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::GraphicsEngine& graphicsEngine = engine.GetGraphicsEngine();
	Tga::ModelDrawer& modelDrawer = graphicsEngine.GetModelDrawer();
	
	if (myModelInstance.IsValid())
	{
		modelDrawer.Draw(myModelInstance);
	}

	if (myRevolver.enabled && myRevolverModelInstance.IsValid())
	{
		modelDrawer.Draw(myRevolverModelInstance);
	}

	if (myShotgun.enabled && myShotgunModelInstance.IsValid())
	{
		modelDrawer.Draw(myShotgunModelInstance);
	}
}
