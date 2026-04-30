#pragma once
#include <vector>

#include "AudioManager.h"
#include "Physics.h"
#include "SceneLoader.h"
#include "tge/model/ModelInstance.h"


class CrateUpdater
{
public:
	struct Crate
	{
		Tga::ModelInstance instance;
		std::shared_ptr<Tga::AnimationPlayer> animationPlayer;
		std::shared_ptr<Tga::AnimatedModelInstance> animatedModelInstance;
		Tga::Vector2f position;
		Tga::Vector2f size;
		bool metal;
		unsigned int id;
		bool dead;
	};

	void Init(const std::vector<SceneLoader::CrateConfig>& someCrates);
	void DeactivateCrate(int aCrateId);
	std::vector<Crate>& GetCrates();
	void Update(float aDeltaTime);
	void Render() const;
	
	float GetTimeToAllowShotgunBulletHitSfx() const;
	float GetTimeSinceLastShotgunBulletHit() const;
	void ShotgunBulletHit();
	bool HasShotgunBulletHit() const;

private:
	std::vector<Crate> myCrates;
	float myTimeSinceLastDeath = 0.f;
	static constexpr float myTimeToAllowBreakSfx = 0.5f;
	float myTimeSinceLastShotgunBulletHit = 0.f;
	static constexpr float myTimeToAllowShotgunBulletHitSfx = 0.1f;
	bool myHasShotgunBulletHit = false;
	//AnimationInstance
};
