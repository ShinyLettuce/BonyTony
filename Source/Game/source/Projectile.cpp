#include "Projectile.h"

#include "tge/Engine.h"
#include "tge/drawers/ModelDrawer.h"
#include "tge/graphics/GraphicsEngine.h"
#include "tge/model/ModelFactory.h"

namespace Tga
{
    class ModelDrawer;
    class GraphicsEngine;
    class Engine;
}

Projectile::Projectile(Tga::ModelInstance& aModelInstance, Tga::Vector2f aPosition, Tga::Vector2f aVelocity, float aKnockBackStrength)
{
    myPosition = aPosition;
    myVelocity = aVelocity;
    
    myKnockbackVelocity = aVelocity.Normalize() * aKnockBackStrength;
    
    myModelInstance = aModelInstance;
    myModelInstance.GetTransform().SetPosition({myPosition.x, myPosition.y, 0.f});
}

void Projectile::Update(float aDeltaTime)
{
    myPosition += myVelocity * aDeltaTime;
    myModelInstance.GetTransform().SetPosition({myPosition.x, myPosition.y, 0.f});
    
    myLifeTime += aDeltaTime;
}

void Projectile::Render() const
{
    const Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::GraphicsEngine& graphicsEngine = engine.GetGraphicsEngine();
	Tga::ModelDrawer& modelDrawer = graphicsEngine.GetModelDrawer();

	modelDrawer.Draw(myModelInstance);
}

void Projectile::Hit()
{
    myVelocity = 0.f;
    myLifeTime = myMaxLifeTime;
    
    //TODO: play hit effect
}

Tga::Vector2f Projectile::GetPosition() const
{
    return myPosition;
}

Tga::Vector2f Projectile::GetVelocity() const
{
    return myVelocity;
}

Tga::Vector2f Projectile::GetSize() const
{
    return mySize;
}

Tga::Vector2f Projectile::GetKnockbackVelocity() const
{
    return myKnockbackVelocity;
}
