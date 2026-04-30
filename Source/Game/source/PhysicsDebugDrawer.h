#pragma once

#include <tge/math/Vector.h>

#include "Physics.h"
#include "Camera.h"

namespace PhysicsDebugDrawer
{
	void DrawDebugAABB(Tga::Vector2f aPosition, Tga::Vector2f aSize);
	void DrawDebugColliders(const Camera& aCamera);
	void DrawDebugRay(Physics::Ray aRay, Tga::Vector4f aColor = { 0.0f, 1.0f, 0.0f, 1.0f });
	void DrawDebugRayCone(Physics::Ray aRay, int aRayAmount, float aSpreadAngle);
}