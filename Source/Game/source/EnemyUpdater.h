#pragma once

#include <vector>

#include "AudioManager.h"
#include "Enemy.h"
#include "FlipbookManager.h"
#include "Projectile.h"
#include "SceneLoader.h"


class EnemyUpdater
{
	public:
		struct EnemyFlipBookHandles
		{
			FlipbookManager::FlipbookHandle fireMeleeHandle;
			FlipbookManager::FlipbookHandle fireRevolverHandle;
		};

		void Init(const std::vector<SceneLoader::EnemyConfig>& aEnemyConfigs,
		          SceneLoader::EnemySharedConfig* aEnemySharedConfig);
		void Update(const float aDeltaTime, const Tga::Vector2f aPlayerPosition);
		void Render();

		std::vector<Enemy>* GetEnemies();
		std::vector<Projectile>* GetProjectiles();

	private:
		std::vector<Projectile> myProjectiles;
		std::vector<Enemy> myEnemies;
		SceneLoader::EnemySharedConfig* myEnemySharedConfig = nullptr;
		EnemyFlipBookHandles myEnemyFlipBookHandles;
		FlipbookManager myEnemyFlipbookManager;
};
