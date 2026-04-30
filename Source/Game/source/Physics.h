#pragma once

#include <algorithm>

#include <tge/math/Vector.h>
#include <tge/primitives/LinePrimitive.h>

#include <vector>
#include <functional>

#include <tge/math/CommonMath.h>

#undef min
#undef max

namespace Physics
{

	struct AABB
	{
		Tga::Vector2f position;
		Tga::Vector2f velocity;
		Tga::Vector2f size;
	};

	struct Ray
	{
		Tga::Vector2f origin;
		Tga::Vector2f direction;
		float magnitude;
	};

	inline Tga::Vector2f GetRayNormal(Ray aRay)
	{
		return { -aRay.direction.y, aRay.direction.x };
	}

	// initialized with standard values corresponding to no collision
	struct CollisionResult
	{
		bool didCollide = false;
		int indexToEntityCollidedWith;
		float pointOfCollisionAlongVelocity = FLT_MAX;
		Tga::Vector2f normal = {};
	};

	inline bool AreCollisionResultsEqual(const CollisionResult* aCollisionResultA, const CollisionResult* aCollisionResultB)
	{
		return aCollisionResultA == aCollisionResultB;
	}

	inline Tga::Vector2f BottomToCenterPivot(const AABB& aAABB)
	{
		return { aAABB.position.x, aAABB.position.y + aAABB.size.y * 0.5f };
	}

	inline bool AABBCollision(const AABB& aAABB, const AABB& aOtherAABB)
	{
		const float sizeAHalfHeight = aAABB.size.y * 0.5f;
		const float sizeAHalfWidth = aAABB.size.x * 0.5f;
		const float sizeBHalfHeight = aOtherAABB.size.y * 0.5f;
		const float sizeBHalfWidth = aOtherAABB.size.x * 0.5f;

		const bool hasCollided = (aAABB.position.x > aOtherAABB.position.x - (sizeBHalfWidth + sizeAHalfWidth) &&
			aAABB.position.x < aOtherAABB.position.x + (sizeBHalfWidth + sizeAHalfWidth) &&
			aAABB.position.y > aOtherAABB.position.y - (sizeBHalfHeight + sizeAHalfHeight) &&
			aAABB.position.y < aOtherAABB.position.y + sizeBHalfHeight + sizeAHalfHeight);

		return hasCollided;
	}

	/// <summary>
	/// Returns a number between 0 and 1 representing where along the line a collision occurred. Returns -1 on no collision.
	/// </summary>
	inline float AABBLineCollision(const AABB& aAABB, const Ray& aRay)
	{
		float tMin = 0.f;
		float tMax = 1.f;

		const float topCornerPositionX = aAABB.position.x - aAABB.size.x * 0.5f;
		const float topCornerPositionY = aAABB.position.y + aAABB.size.y * 0.5f;
		Tga::Vector2f lineStart = aRay.origin;
		Tga::Vector2f lineDirection = aRay.direction.GetNormalized() * aRay.magnitude;

		if (abs(lineDirection.x) < FLT_EPSILON)
		{
			if (lineStart.x < topCornerPositionX || lineStart.x >  topCornerPositionX + aAABB.size.x)
			{
				return FLT_MAX;
			}
		}
		else
		{
			float t1 = (topCornerPositionX - lineStart.x) / lineDirection.x;
			float t2 = (topCornerPositionX + aAABB.size.x - lineStart.x) / lineDirection.x;

			if (t1 > t2)
			{
				std::swap(t1, t2);
			}

			tMin = std::max(tMin, t1);
			tMax = std::min(tMax, t2);

			if (tMin > tMax)
			{
				return FLT_MAX;
			}
		}
		if (abs(lineDirection.y) < FLT_EPSILON)
		{
			if (lineStart.y > topCornerPositionY || lineStart.y < topCornerPositionY - aAABB.size.y)
			{
				return FLT_MAX;
			}
		}
		else
		{
			float t1 = (topCornerPositionY - lineStart.y) / lineDirection.y;
			float t2 = (topCornerPositionY - aAABB.size.y - lineStart.y) / lineDirection.y;

			if (t1 > t2)
			{
				std::swap(t1, t2);
			}

			tMin = std::max(tMin, t1);
			tMax = std::min(tMax, t2);

			if (tMin > tMax)
			{
				return FLT_MAX;
			}
		}
		return tMin;
	}

	template <typename T>
	CollisionResult AABBCollisionOverContainer(const AABB& aAABB, const std::vector<T> aObjects, std::function<AABB(const T&)> aObjectToAABBFunc)
	{
		for (int i = 0; i < aObjects.size(); ++i)
		{
			AABB otherAABB = aObjectToAABBFunc(aObjects[i]);
			
			if (otherAABB.size.x == 0.0f || otherAABB.size.y == 0.0f)
			{
				continue;
			}

			if (AABBCollision(aAABB, otherAABB))
			{
				return CollisionResult
				{
					.didCollide = true,
					.indexToEntityCollidedWith = i
				};
			}
		}
		return CollisionResult{ };
	}


	/// <summary>
	/// Check the closest collider in a list that is hit
	/// </summary>
	template<typename T>
	CollisionResult SweepAABBCollisionOverContainer( AABB aAABB, const std::vector<T>& aObjects, std::function<AABB(const T&)> aObjectToAABBFunc, float aDeltaTime)
	{
		aAABB.position = BottomToCenterPivot(aAABB);
		float closestCollisionAlongVelocity = FLT_MAX;
		int closestIndex = 9999999;
		Tga::Vector2f closestNormal = { 0,0 };
		for (int i = 0; i < aObjects.size(); ++i)
		{
			AABB otherAABB = aObjectToAABBFunc(aObjects[i]);
			if (otherAABB.size.x < FLT_EPSILON || otherAABB.size.y < FLT_EPSILON)
			{
				continue;
			}
			otherAABB.position = BottomToCenterPivot(otherAABB);
			otherAABB.size += aAABB.size;
			float collisionAlongVelocity = AABBLineCollision(otherAABB, Ray(aAABB.position, aAABB.velocity, aAABB.velocity.Length() * aDeltaTime));

			if (collisionAlongVelocity == FLT_MAX)
			{
				continue;
			}
			else if (collisionAlongVelocity < closestCollisionAlongVelocity)
			{
				closestCollisionAlongVelocity = collisionAlongVelocity;
				closestIndex = i;


				Tga::Vector2f hitPosition = aAABB.position + aAABB.velocity * aDeltaTime * closestCollisionAlongVelocity;

				float dx = hitPosition.x - otherAABB.position.x;
				float dy = hitPosition.y - otherAABB.position.y;
				float px = otherAABB.size.x * 0.5f - std::fabsf(dx);
				float py = otherAABB.size.y * 0.5f - std::fabsf(dy);

				if (px < py)
				{
					closestNormal.x = (static_cast<float>(dx > 0.f)) - (static_cast<float>(dx < 0.f));
				}
				else
				{
					closestNormal.y = (static_cast<float>(dy > 0.f)) - (static_cast<float>(dy < 0.f));
				}
			}
			else if (collisionAlongVelocity == closestCollisionAlongVelocity)
			{
				Tga::Vector2f hitPosition = aAABB.position + aAABB.velocity * aDeltaTime * closestCollisionAlongVelocity;

				float dx = hitPosition.x - otherAABB.position.x;
				float dy = hitPosition.y - otherAABB.position.y;
				float px = otherAABB.size.x * 0.5f - std::fabsf(dx);
				float py = otherAABB.size.y * 0.5f - std::fabsf(dy);

				if (px < py)
				{
					closestNormal.x = (static_cast<float>(dx > 0.f)) - (static_cast<float>(dx < 0.f));
				}
				else
				{
					closestNormal.y = (static_cast<float>(dy > 0.f)) - (static_cast<float>(dy < 0.f));
				}

				if (fabsf(aAABB.velocity.x) > fabsf(aAABB.velocity.y) && closestNormal.x != 0)
				{
					closestCollisionAlongVelocity = collisionAlongVelocity;
					closestIndex = i;
				}
				else if (fabsf(aAABB.velocity.y) > fabsf(aAABB.velocity.x) && closestNormal.y != 0)
				{
					closestCollisionAlongVelocity = collisionAlongVelocity;

				}
			}
		}

		if (closestCollisionAlongVelocity == FLT_MAX)
		{
			return CollisionResult{ };
		}

		return CollisionResult{
			.didCollide = true,
			.indexToEntityCollidedWith = closestIndex,
			.pointOfCollisionAlongVelocity = closestCollisionAlongVelocity,
			.normal = closestNormal
		};
	}

	template<typename T>
	CollisionResult RaycastAABBCollisionOverContainer(Ray aRay, const std::vector<T>& aObjects, std::function<AABB(const T&)> aObjectToAABBFunc)
	{
		float closestT = FLT_MAX;
		int closestIndex = 99999999;
		for (int i = 0; i < aObjects.size(); ++i)
		{
			AABB otherAABB = aObjectToAABBFunc(aObjects[i]);
			if (otherAABB.size.x < FLT_EPSILON || otherAABB.size.y < FLT_EPSILON)
			{
				continue;
			}
			otherAABB.position = BottomToCenterPivot(otherAABB);
			const float t = AABBLineCollision(otherAABB, aRay);

			if (t == FLT_MAX)
			{
				continue;
			}
			else if (t < closestT)
			{
				closestT = t;
				closestIndex = i;
			}
		}

		if (closestT == FLT_MAX)
		{
			return CollisionResult{};
		}

		return CollisionResult{
			.didCollide = true,
			.indexToEntityCollidedWith = closestIndex,
			.pointOfCollisionAlongVelocity = closestT
		};
	}

	/// <summary>
	/// Send out a cone of rays and return a list containing the closest collision result for each ray.
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="aRay"> The center, origin of the cone as well as the length of each ray</param>
	/// <param name="aRayAmount"> Amount of rays that are sent out</param>
	/// <param name="aSpreadAngle"> The total angle of the spread</param>
	/// <param name="aObjects"> List of objects to loop over</param>
	/// <param name="aObjectToAABBFunc"> Lambda returning an AABB of an object in the list</param>
	/// <returns> List of Collision Results, </returns>
	template <typename T>
	std::vector<CollisionResult> RaycastConeAABBCollisionOverContainer(Ray aRay, int aRayAmount, float aSpreadAngle, const std::vector<T>& aObjects, std::function<AABB(const T&)> aObjectToAABBFunc)
	{
		std::vector<CollisionResult> results{ static_cast<size_t>(aRayAmount), CollisionResult{.pointOfCollisionAlongVelocity = FLT_MAX} };
		float originalAngle = std::atan2(aRay.direction.y, aRay.direction.x);
		float deltaAngle = 0;
		float startAngle;
		if (aRayAmount != 1)
		{
			deltaAngle = aSpreadAngle / static_cast<float>(aRayAmount - 1);
			startAngle = originalAngle - aSpreadAngle / 2;
		}
		else
		{
			startAngle = originalAngle;
		}

		for (int i = 0; i < aRayAmount; ++i)
		{
			aRay.direction = {
				std::cos(startAngle + deltaAngle * i),
				std::sin(startAngle + deltaAngle * i)
			};

			for (int j = 0; j < aObjects.size(); ++j)
			{
				AABB otherAABB = aObjectToAABBFunc(aObjects[j]);
				if (otherAABB.size.x < FLT_EPSILON || otherAABB.size.y < FLT_EPSILON)
				{
					continue;
				}
				otherAABB.position = BottomToCenterPivot(otherAABB);
				float collisionTime = AABBLineCollision(otherAABB, aRay);
				if (collisionTime == FLT_MAX)
				{
					continue;
				}
				if (collisionTime < results[i].pointOfCollisionAlongVelocity)
				{
					results[i].didCollide = true;
					results[i].pointOfCollisionAlongVelocity = collisionTime;
					results[i].indexToEntityCollidedWith = j;
				}
			}
		}
		return results;
	}
}
