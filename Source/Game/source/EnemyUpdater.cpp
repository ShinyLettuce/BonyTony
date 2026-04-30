#include "EnemyUpdater.h"

#include "SceneLoader.h"
#include "imgui/imgui.h"

void EnemyUpdater::Init(const std::vector<SceneLoader::EnemyConfig>& aEnemyConfigs, SceneLoader::EnemySharedConfig* aEnemySharedConfig)
{
    myEnemySharedConfig = aEnemySharedConfig;

    myProjectiles.clear();
    myEnemies.clear();

    myEnemyFlipBookHandles.fireMeleeHandle = myEnemyFlipbookManager.MakeNewFlipbookHandle();
    myEnemyFlipBookHandles.fireRevolverHandle = myEnemyFlipbookManager.MakeNewFlipbookHandle();

    myEnemyFlipbookManager.RegisterFlipBook(FlipBookPresets::ENEMY_BASEBALL_FIRE, myEnemyFlipBookHandles.fireMeleeHandle);
    myEnemyFlipbookManager.RegisterFlipBook(FlipBookPresets::ENEMY_REVOLVER_FIRE, myEnemyFlipBookHandles.fireRevolverHandle);
    
    for (auto& enemyConfig : aEnemyConfigs)
    {
        myEnemies.emplace_back();
        myEnemies.back().Init(enemyConfig, aEnemySharedConfig, &myProjectiles, &myEnemyFlipbookManager,
                    &myEnemyFlipBookHandles.fireMeleeHandle, &myEnemyFlipBookHandles.fireRevolverHandle);
    }
}

void EnemyUpdater::Update(const float aDeltaTime, const Tga::Vector2f aPlayerPosition)
{
    myEnemyFlipbookManager.Update(aDeltaTime);
    
    for (size_t i = 0; i < myProjectiles.size(); i++)
    {
        myProjectiles[i].Update(aDeltaTime);
    }
    std::erase_if(myProjectiles, [](Projectile p) { return p.GetDead(); });

    for (auto& enemy : myEnemies)
    {
        enemy.Update(aDeltaTime, aPlayerPosition);
    }
    
    #if !defined(_RETAIL)
        
    ImGui::Begin("Shared Enemy Config");
    
    ImGui::DragFloat2("Melee Knockback", myEnemySharedConfig->knockBackForce.myValues);
    
    ImGui::DragFloat("Detection Range", &myEnemySharedConfig->detectionRange);
    ImGui::DragFloat("Detection Angle", &myEnemySharedConfig->detectionAngle);
    
    ImGui::DragFloat("Aim Speed", &myEnemySharedConfig->aimSpeed);
    ImGui::DragFloat("Distance to Fire", &myEnemySharedConfig->distanceToFire);
    
    ImGui::DragFloat("Shot Cooldown", &myEnemySharedConfig->shotCooldown);
    
    ImGui::DragFloat("Death Duration", &myEnemySharedConfig->deathDuration);
    
    ImGui::DragFloat("Projectile Speed", &myEnemySharedConfig->projectileSpeed);
    ImGui::DragFloat("Projectile Knockback", &myEnemySharedConfig->projectileKnockBackForce);
    
    for (auto& enemy : myEnemies)
    {
        enemy.UpdateImGui();
    }
    
    ImGui::End();
    
    #endif
}

void EnemyUpdater::Render()
{
    myEnemyFlipbookManager.Render();
    
    for (auto& projectile : myProjectiles)
    {
        projectile.Render();
    }
    for (auto& enemy : myEnemies)
    {
        enemy.Render();
    }
}

std::vector<Enemy>* EnemyUpdater::GetEnemies()
{
    return &myEnemies;
}

std::vector<Projectile>* EnemyUpdater::GetProjectiles()
{
    return &myProjectiles;
}
