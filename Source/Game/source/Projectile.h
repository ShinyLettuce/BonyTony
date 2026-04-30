#pragma once
#include <tge/math/Vector2.h>

#include "tge/model/ModelInstance.h"

class Player;

class Projectile
{
public:
    Projectile(Tga::ModelInstance& aModelInstance, const Tga::Vector2f aPosition, const Tga::Vector2f aVelocity, float aKnockBackStrength);
    void Update(float aDeltaTime);
    void Render() const;
    
    void Hit();
    
    Tga::Vector2f GetPosition() const;
	Tga::Vector2f GetVelocity() const;
	Tga::Vector2f GetSize() const;
	Tga::Vector2f GetKnockbackVelocity() const;
    bool GetDead() const { return myLifeTime >= myMaxLifeTime; }
private:
	Tga::ModelInstance myModelInstance;
    Tga::Vector2f myPosition{};
    Tga::Vector2f myVelocity{};
    Tga::Vector2f mySize = {50.f, 50.f};
	Tga::Vector2f myKnockbackVelocity{};
    float myLifeTime = 0.f;
    float myMaxLifeTime = 10.f;
};
