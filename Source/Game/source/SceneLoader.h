#pragma once

#include <vector>

#include <tge/math/Vector.h>
#include <tge/animation/Animation.h>
#include <tge/model/ModelInstance.h>
#include <tge/animation/AnimationPlayer.h>
#include <tge/engine.h>
#include <tge/graphics/Camera.h>
#include <tge/graphics/dx11.h>
#include <tge/graphics/GraphicsEngine.h>
#include <tge/drawers/ModelDrawer.h>
#include <tge/model/Modelfactory.h>
#include <tge/texture/TextureManager.h>
#include <tge/scene/Scene.h>
#include <tge/scene/SceneSerialize.h>
#include <tge/settings/settings.h>
#include <tge/scene/SceneObject.h>
#include <tge/scene/SceneObjectDefinition.h>
#include <tge/scene/SceneObjectDefinitionManager.h>
#include <tge/scene/ScenePropertyTypes.h>
#include <tge/model/Model.h>
#include <tge/script/BaseProperties.h>

#include "tge/stringRegistry/StringRegistry.h"

#include "AmbienceManager.h"

namespace SceneLoader
{
	struct CameraConfig
	{
		float depth;
		float height;
		float fov;
	};

	struct PlayerConfig
	{
		struct GunData
		{
			int maxClip;
			float maxDistance;
			float timeToMaxDistance;
			float fallTime;
			float gravityConstant;
			float range;
			float timeBetweenShots;
			//SoundHandle soundToPlayOnShoot;
			//SoundHandle soundToPlayOnReload;
			//SoundHandle soundToPlayOnReady;
			//SoundHandle soundToPlayOnEmpty;
		};

		int shotgunBulletAmount;
		float shotgunSpreadAngle;
		float shotgunHangtime;
		
		float bulletTimeTimeScale;
		float bulletTimeLerpToSpeed;
		float bulletTimeLerpFromSpeed;
		float timeInBulletTime;

		GunData shotgunData;
		GunData powerShotData;
		GunData revolverData;
		Tga::Vector2f position;
		Tga::Vector2f size;
		
		float stunDuration;
		float playerArmPivotHeight;
		float playerGroundedGravity;
		float playerGroundedFriction;
		float playerAirFriction;

		Tga::ModelInstance modelInstance;
		Tga::ModelInstance revolverModelInstance;
		Tga::ModelInstance shotgunModelInstance;
	};

	struct BossConfig
	{
		std::shared_ptr<Tga::AnimatedModelInstance> animatedModelInstance;
		Tga::AnimationClipReference idleClipReference;
	};

	struct LevelTriggerConfig
	{
		Tga::StringId sceneToLoad;
		Tga::Vector2f position;
		Tga::Vector2f size;
		std::shared_ptr <Tga::AnimatedModelInstance> modelInstance;
		Tga::AnimationClipReference closeAnimation;
		Tga::AnimationClipReference openAnimation;
		bool exists;
	};
	
	struct EnemySharedConfig
	{
		Tga::Vector2f knockBackForce;
		
		float detectionRange;
		float detectionAngle;
		float aimSpeed;
		float distanceToFire;
		float shotCooldown;
		float deathDuration;
		
		float projectileSpeed;
		float projectileKnockBackForce;
	};

	struct EnemyConfig
	{
		Tga::Vector2f position;
		std::shared_ptr <Tga::AnimatedModelInstance> modelInstance;
		std::shared_ptr <Tga::AnimatedModelInstance> modelInstanceNoHand;
		std::shared_ptr <Tga::AnimatedModelInstance> modelInstanceDeath;
		Tga::ModelInstance projectileModelInstance;
		Tga::ModelInstance weaponModelInstance;
		Tga::AnimationClipReference idleClipReference;
		Tga::AnimationClipReference meleeClipReference;
		Tga::AnimationClipReference deathClipReference;


		bool hasGun;
	};

	struct CrateConfig
	{
		CrateConfig()
			: 
			animatedModelInstance(std::make_shared<Tga::AnimatedModelInstance>())
		{
		}

		Tga::ModelInstance modelInstance;
		std::shared_ptr<Tga::AnimatedModelInstance> animatedModelInstance;
		Tga::AnimationClipReference breakClipReference;
		Tga::Vector2f position;
		Tga::Vector2f size;
		bool metal;
	};
	
	struct TileConfig
	{
		Tga::ModelInstance modelInstance;
		Tga::Vector2f position;
		Tga::Vector2f size;
	};

	struct ModelConfig
	{
		Tga::ModelInstance modelInstance;
	};

	enum class PickupType
	{
		None,
		Shotgun,
		Revolver,
		PowerShot
	};

	struct PickupConfig
	{
		PickupType type;
		Tga::ModelInstance modelInstance;
		Tga::Vector2f position;
		Tga::Vector2f size;
	};
	
	enum class SceneType
	{
		Unknown,
		Level1,
		Level2,
		BossScene,
	};

	struct SceneMetaConfig
	{
		SceneType type;
	};

	struct SceneConfig
	{
		SceneMetaConfig metaConfig;
		CameraConfig cameraConfig;
		PlayerConfig playerConfig;
		BossConfig bossConfig;
		EnemySharedConfig enemySharedConfig;
		LevelTriggerConfig levelTriggerConfig;
		std::vector<EnemyConfig> enemieyConfigs; // �ndra till 'enemyConfigs' ifall du �r sn�ll
		std::vector<TileConfig> tileConfigs;
		std::vector<ModelConfig> modelConfigs;
		std::vector<CrateConfig> crateConfigs;
		std::vector<PickupConfig> pickupConfigs;
		std::vector<Ambience> ambiences;
	};

	void Init();

	void StartPreloadProcess();
	void KillPreloadProcess();

	void PackScene(const char* aPath);
	bool LoadSceneByPack(const char* aPath, Tga::Scene& outScene);

	/// <summary>
	/// Loads a scene given a path
	/// </summary>
	SceneConfig& LoadSceneByPath(const char* aPath);

	/// <summary>
	/// Returns the active scene
	/// </summary>
	SceneConfig& GetActiveScene();
}
