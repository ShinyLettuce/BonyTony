#include "CrateUpdater.h"
#include "tge/Engine.h"
#include "tge/drawers/ModelDrawer.h"
#include "tge/graphics/GraphicsEngine.h"

void CrateUpdater::Init(const std::vector<SceneLoader::CrateConfig>& someCratesConfigs)
{
	myCrates.clear();

	for (auto& crateConfig : someCratesConfigs)
	{
		Tga::ModelFactory& modelFactory = Tga::ModelFactory::GetInstance();

		std::shared_ptr<Tga::AnimationPlayer> breakAnimationPlayer = std::make_shared<Tga::AnimationPlayer>(
			modelFactory.GetAnimationPlayer(crateConfig.breakClipReference.path.GetString(),
			crateConfig.modelInstance.GetModel()));

		myCrates.push_back
		(
			Crate
			{
				.instance = crateConfig.modelInstance,
				.animationPlayer = breakAnimationPlayer,
				.animatedModelInstance = crateConfig.animatedModelInstance,
				.position = crateConfig.position,
				.size = crateConfig.size,
				.metal = crateConfig.metal,
				.id = static_cast<unsigned int>(myCrates.size()),
				.dead = false
			}
		);

		Crate& crate = myCrates.back();

		crate.animationPlayer->Stop();
		crate.animationPlayer->SetTime(0.0f);
		crate.animatedModelInstance->GetTransform().SetPosition(Tga::Vector3f{ crate.position, 0.0f });
		crate.animatedModelInstance->GetTransform().ResetScaleAndRotation();
		crate.animatedModelInstance->GetTransform().Scale(Tga::Vector3f{ 100.0f, 100.0f, 100.0f });
	}
}

void CrateUpdater::DeactivateCrate(const int aCrateId)
{
	myCrates[aCrateId].dead = true;
	
	if (myTimeSinceLastDeath >= myTimeToAllowBreakSfx)
	{
		myTimeSinceLastDeath = 0;
		
		if (myCrates[aCrateId].metal)
		{
			AudioManager::GetAudioPoolByHandle(AudioHandles::bulletproofGlassDestroyed).Play();
		}
		else
		{
			AudioManager::GetAudioPoolByHandle(AudioHandles::woodenCrateDestroyed).Play();
		}
	}
	
	myCrates[aCrateId].animationPlayer->Play();
}

std::vector<CrateUpdater::Crate>& CrateUpdater::GetCrates()
{
	return myCrates;
}

void CrateUpdater::Update(float aDeltaTime)
{
	myTimeSinceLastDeath += aDeltaTime;
	myTimeSinceLastShotgunBulletHit += aDeltaTime;
	
	for (auto& crate : myCrates)
	{
		if (crate.animationPlayer->IsValid())
		{
			crate.animationPlayer->Update(aDeltaTime);
		}
	}
}

void CrateUpdater::Render() const
{
	Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::GraphicsEngine& graphicsEngine = engine.GetGraphicsEngine();
	Tga::ModelDrawer& modelDrawer = graphicsEngine.GetModelDrawer();
	
	for (auto& crate : myCrates)
	{
		if (crate.dead)
		{
			if (crate.animationPlayer->IsValid() && crate.animationPlayer->GetState() == Tga::AnimationState::Playing)
			{
				crate.animatedModelInstance->SetPose(*crate.animationPlayer);
				modelDrawer.Draw(*crate.animatedModelInstance);
			}
		}
		else
		{
			modelDrawer.Draw(crate.instance);
		}
	}
}

float CrateUpdater::GetTimeToAllowShotgunBulletHitSfx() const
{
	return myTimeToAllowShotgunBulletHitSfx;
}

float CrateUpdater::GetTimeSinceLastShotgunBulletHit() const
{
	return myTimeSinceLastShotgunBulletHit;
}

void CrateUpdater::ShotgunBulletHit()
{
	myTimeSinceLastShotgunBulletHit = 0.f;
	myHasShotgunBulletHit = true;
}

bool CrateUpdater::HasShotgunBulletHit() const
{
	return myHasShotgunBulletHit;
}
