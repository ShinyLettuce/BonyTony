#pragma once
#include "AudioManager.h"
#include "Camera.h"
#include "InputMapper.h"
#include "PlayerUpdateResult.h"
#include "Physics.h"
#include "SceneLoader.h"

class Player
{
	struct Gun
	{
		bool enabled;
		bool ready;
		int clip;
		int maxClip;
		float maxDistance;
		float range;
		float timeBetweenShots;
		float currentLockTime;
		float timeToMaxDistance;
		float fallTime;
		float gravityConstant;
	};
	
	public:
	
		void SetInput(InputMapper* anInputMapper);
		
		void Init(const SceneLoader::PlayerConfig& aPlayerConfig);
		
		PlayerUpdateResult Update(float aDeltaTime, Camera& aCamera);
		void LateUpdate(float aDeltaTime);
		void Render();
		
		void OnCollision(const Physics::CollisionResult& aCollisionResult);
		void StunPlayer(Tga::Vector2f aKnockbackVelocity);
		void Reload();
		void StopVelocityX();
		void StopVelocityY();
		void SetGrounded(bool aIsGroundedBool);
		void SetPosition(const Tga::Vector2f& aPosition);
		void SetVelocity(const Tga::Vector2f& aVelocity);
		void SetFrozen(bool aIsFrozen);
		void UpdateNormalizedAim(Camera& aCamera);

		float GetShotgunSpreadAngle() const;
		int GetShotgunBulletAmount() const;
		float GetShotgunRange() const;
		float GetRevolverRange() const;
		Tga::Vector2f GetRevolverPosition() const;
		Tga::Vector2f GetShotgunPosition() const;
		Tga::Vector2f GetShotOrigin() const;
		Tga::Vector2f GetShotgunBarrelEndPoint() const;

		int GetShotgunClip() const;
		int GetShotgunMaxClip() const;
		int GetRevolverClip() const;
		int GetRevolverMaxClip() const;
	
		bool GetIsRevolverReady() const;
	
		float GetShotgunTimeBetweenShots() const;
		
		float GetTimeInBulletTime() const;
		float GetBulletTimeTimeScale() const;
		float GetBulletTimeLerpToSpeed() const;
		float GetBulletTimeLerpFromSpeed() const;

		void EnableShotgun();
		void EnableRevolver();
		void EnablePowershot();

		void DisableShotgun();
		void DisableRevolver();
		void DisablePowershot();

		bool GetIsShotgunEnabled() const;
		bool GetIsRevolverEnabled() const;
		bool GetIsPowershotEnabled() const;

		float GetGravity(float aDeltaTime);
		Tga::Vector2f GetPosition() const;
		/// <summary>
		///	Returns width, height
		/// </summary>
		Tga::Vector2f GetSize() const;
		Tga::Vector2f GetVelocity() const;
		bool GetPlayerStunned() const;
		//Tga::Vector2f GetNormalizedAim() const;
		Tga::Vector2f GetNormalizedRevolverAim() const;
		Tga::Vector2f GetNormalizedShotgunAim() const;
		bool GetGrounded() const;
		/// <summary>
		///	Returns if the Powershot has been fired
		/// </summary>
		bool GetPowerBreakActive() const;
		void DisablePowerBreak();
	
		void FreezeTheShotgunSoThatItCanOnlyPointRightAndCantBeMovedWithMouseOrControllerOrAnyOtherInputDeviceForThatMatter();
	private:

		void ImGuiUpdate(float aDeltaTime);
		void ShootRevolver(Camera& aCamera);
		void ShootShotgun(Camera& aCamera);
		void ShootPowerShot(Camera& aCamera);
		void DecayXVelocity(float aDeltaTime);
		void AnimateWeapons(float aDeltaTime);
		void AnimateWobble(float aDeltaTime);

		/// <summary>
		///	Calculates Air resistance, air-resistance and gravity from the specifics of the gun.
		///	This will change the feel of control as the player fires the weapons
		/// </summary>
		void CalculateVelocity(const Gun& aGun);
		
		bool myPlayerStunned = false;
		bool myIsGrounded = false;
		bool myIsFrozen = false;
		
		float myStunDuration = 0;
		float myPlayerArmPivotHeight = 0;
		float myStunTimer = 0;
		float myGroundedGravity = 0;
		float myGroundedFriction = 0;
		float myAirFriction = 0;
		
		float myForceVelocity = 0;
		float myAirResistance = 0;
		float myFallGravity = 0;

		float myShotgunBulletSpreadAngle = 0;
		float myShotgunHangtime = 0;
		float myShotgunHangtimeTimer = 0;
		int myShotgunBulletAmount = 0;

		inline static const float REVOLVER_INPUT_BUFFER_TIME{ 0.3f };
		float myRevolverTimeSinceInput = 0.0f;
		float myRevolverTimeSincePreviousShot = 0.0f;
		Tga::Vector2f myRevolverPreviousShotDirection;

		float myBulletTimeTimeScale = 0.2f;
		float myBulletTimeLerpToSpeed = 10.f;
		float myBulletTimeLerpFromSpeed = 10.f;
		float myTimeInBulletTime = 0.6f;

		bool myShotgunWasFiredThisFrame = false;
		bool myRevolverWasFiredThisFrame = false;
		bool myPowershotWasFiredThisFrame = false;
		bool myWasHitByEnemyThisFrame = false;

		bool myFreezeShotgunSoThatItOnlyPointsRightAndCantBeMovedWithMouseOrController = false;
		
		Tga::Vector2f myPosition;
		Tga::Vector2f myVelocity;
		Tga::Vector2f myNormalizedShotgunAim;;
		Tga::Vector2f myNormalizedRevolverAim;
		Tga::Vector2f myScreenResolution;
		Tga::Vector2f mySize;
		
		InputMapper* myInputMapper = nullptr;
		Tga::ModelInstance myModelInstance;
		Tga::ModelInstance myRevolverModelInstance;
		Tga::ModelInstance myShotgunModelInstance;
		
		Gun myShotgun {};
		Gun myRevolver {};
		Gun myPowerShot {}; // Based on ShotGun other than physics variables, will drain Shotgun bullets
};
