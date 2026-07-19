#pragma once
#include <vector>

#include "tge/engine.h"
#include "tge/math/Vector2.h"
#include "tge/sprite/sprite.h"
#include "tge/shaders/SpriteShader.h"

namespace FlipBookPresets
{
	struct FlipbookPreset
	{
		const char* flipbookAssetPath;
		unsigned int frameAmount;
		float timeStep;
		float size;
	};

	inline constexpr FlipbookPreset TONY_SHOTGUN_FIRE
	{
		.flipbookAssetPath = "textures/Flipbooks/ShotgunMuzzle_Tony.dds",
		.frameAmount = 3
	};
	inline constexpr FlipbookPreset TONY_SHOTGUN_FIRE_TRAIL
	{
		.flipbookAssetPath = "textures/Flipbooks/ShotgunBullets_Tony.dds",
		.frameAmount = 3
	};
	inline constexpr FlipbookPreset TONY_POWERSHOT_FIRE
	{
		.flipbookAssetPath = "textures/Flipbooks/ShotgunMuzzlePower_Tony.dds",
		.frameAmount = 3
	};
	inline constexpr FlipbookPreset TONY_POWERSHOT_FIRE_TRAIL
	{
		.flipbookAssetPath = "textures/Flipbooks/ShotgunBulletsPower_Tony.dds",
		.frameAmount = 3
	};
	inline constexpr FlipbookPreset TONY_REVOLVER_FIRE
	{
		.flipbookAssetPath = "textures/Flipbooks/T_MuzzleFlashTony_flip.dds",
		.frameAmount = 3
	};
	inline constexpr FlipbookPreset ENEMY_BASEBALL_FIRE
	{
		.flipbookAssetPath = "textures/Flipbooks/Batswing_Enemy.dds",
		.frameAmount = 3
	};
	inline constexpr FlipbookPreset ENEMY_REVOLVER_FIRE
	{
		.flipbookAssetPath = "textures/Flipbooks/T_MuzzleFlashEnemy_flip.dds",
		.frameAmount = 3
	};
	inline constexpr FlipbookPreset ENEMY_HIT
	{
		.flipbookAssetPath = "textures/Flipbooks/T_EnemyHit_flip.DDS",
		.frameAmount = 4,
		.timeStep = 0.038f,
		.size = 600.f
	};
	inline constexpr FlipbookPreset ENVIRONMENT_HIT
	{
		.flipbookAssetPath = "textures/Flipbooks/T_EnvironmentHit_flip.DDS",
		.frameAmount = 3,
		.timeStep = 0.022f
	};
	inline constexpr FlipbookPreset CRATE_HIT
	{
		.flipbookAssetPath = "textures/Flipbooks/T_WoodDestruction_Flip.DDS",
		.frameAmount = 4
	};
	inline constexpr FlipbookPreset METAL_CRATE_HIT
	{
		.flipbookAssetPath = "textures/Flipbooks/T_MetallicDestruction_Flip.DDS",
		.frameAmount = 4
	};
	inline constexpr FlipbookPreset STEAM_ENVIRONMENT
	{
		.flipbookAssetPath = "textures/Flipbooks/T_Steam_Flip.DDS",
		.frameAmount = 4
	};
	inline constexpr FlipbookPreset SHELL_EJECT
	{
		.flipbookAssetPath = "textures/Flipbooks/T_ShellEject_flip.dds",
		.frameAmount = 4
	};
	inline constexpr FlipbookPreset BULLET_EJECT
	{
		.flipbookAssetPath = "textures/Flipbooks/T_BulletEject_flip.dds",
		.frameAmount = 4
	};
}

enum class FlipbookHandle
{
	ShotgunFire,
	ShotgunFireTrail,
	PowershotFire,
	PowershotFireTrail,
	RevolverFire,
	EnemyBaseballFire,
	EnemyRevolverFire,
	EnemyHit,
	EnvironmentHit,
	CrateHit,
	MetalCrateHit,
	SteamEnvironment,
	BossSpeechBubble,
	ShellEject,
	BulletEject,
	
	Count
};

enum class PersistentInstanceHandle
{
	ShotgunFire,
	PowerShotFire,
	RevolverFire
};

class FlipbookManager
{
	public:

		//Registers a Flipbook Based on a preset
		void RegisterFlipBook(FlipBookPresets::FlipbookPreset aPreset, FlipbookHandle aHandle, bool aLooping = false);
		void InitPersistentFlipbooks();
		void RemoveAllLoopingInstances();
		void SetPersistentInstanceFlipbook(PersistentInstanceHandle anInstanceHandle, FlipbookHandle aFlipbookHandle);
		bool GetPersistentInstanceActive(PersistentInstanceHandle anInstanceHandle) const;
		void PlayPersistent(PersistentInstanceHandle anInstanceHandle, float aFrameUpdateRate);
		void MovePersistent(PersistentInstanceHandle anInstanceHandle, Tga::Matrix4x4f aTransform);

		void PlayAt(FlipbookHandle aHandle, Tga::Vector2f aPosition, float aRotation = 0.f, Tga::Vector2f aSize = { 1.f, 1.f }, float aTimeStep = 0.01f, bool aFlipped = false);
		void PlayAt(FlipbookHandle aHandle, Tga::Matrix4x4f aTransform, float aTimeStep = 0.01f);

		//Updates state of all following flipbooks, and removes flipbook instances that have finished
		void Update(float aDeltaTime);

		void Render() const;

	private:
		struct UV
		{
			UV(Tga::Vector2f aStart, Tga::Vector2f aEnd)
			{
				start = aStart;
				end = aEnd;
			}

			Tga::Vector2f start;
			Tga::Vector2f end;
		};

		struct Flipbook
		{
			bool looping = false;
			unsigned int frameAmount;
			std::vector<UV> uvMap;
			Tga::SpriteSharedData spriteData;
		};

		struct FlipbookInstance
		{
			unsigned int frameIndex = 0;

			bool flipped = false;
			bool ignoreDepth = false;
			
			FlipbookHandle flipbookHandle;

			Tga::Vector2f position;
			float rotation;

			float frameUpdateRate = 0.01f;
			float timer = 0.f;

			Tga::Vector2f size;
			Tga::Vector2f pivot{ 0.5f, 0.5f };
		};

		struct Flipbook3DInstance
		{
			unsigned int frameIndex = 0;

			bool active = true;
			bool flipped = false;
			
			FlipbookHandle flipbookHandle;

			Tga::Matrix4x4f transform;

			float frameUpdateRate = 0.01f;
			float timer = 0.f;
		};
		
		std::array<Flipbook, (int)FlipbookHandle::Count> myFlipbooks;

		//Created upon playing a flipbook, removed when done
		std::vector<FlipbookInstance> myFlipbookInstances;
		std::vector<Flipbook3DInstance> myFlipbook3DInstances;
		std::vector<Flipbook3DInstance> myPersistentFlipbook3DInstances;
};
