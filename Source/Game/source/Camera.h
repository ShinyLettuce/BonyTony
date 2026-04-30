#pragma once

#include <tge/math/Vector.h>
#include <tge/graphics/Camera.h>

#include "InputMapper.h"
#include "Timer.h"

class Camera
{
	struct Plane
	{
		Tga::Vector3f point;
		Tga::Vector3f normal;
	};

	struct Frustum
	{
		Plane top;
		Plane bottom;
		Plane left;
		Plane right;
	};

	struct ShakeData
	{
		float time;
		float amplitude;
		float frequency;
		float duration;
		float remaining;
	};
public:
	void Init(Timer* aTimer);

	void Shake(const float aAmplitude, const float aFrequency, const float aDuration);

	void SetDepth(float aDepth);
	void SetHeight(float aHeight);
	void SetFov(float aFov);

	void MoveToPosition(Tga::Vector2f aPosition);
	void MoveTowardsPosition(Tga::Vector2f aPosition, float aFactor, float aDeltaTime);
	void MoveTowardsPosition(Tga::Vector2f aPosition, Tga::Vector2f aReferencePoint, float aFactor, float aDeltaTime);

	Tga::Vector3f GetScreenToWorldPoint(const Tga::Vector2f& aPoint);
	Tga::Vector2f GetWorldToScreenPoint(const Tga::Vector2f& aPoint);

	Tga::Camera& GetTgaCamera();

	void Update();
	void Prepare();

	bool IsPointWithinFrustum(const Tga::Vector3f& aPoint) const;

	void DrawScreenToWorldDebugGizmos(Tga::Vector2f aScreenPoint);
private:
	Tga::Vector2f GetShakeOffset();

	Frustum GetFrustum() const;
	float GetAspectRatio() const;

	bool IsPointWithinPlane(Plane& aPlane, const Tga::Vector3f& aPoint) const;
	bool IsPointWithinFrustum(Frustum& aFrustum, const Tga::Vector3f& aPoint) const;

	float HorizontalToVerticalFov(float aHorizontalFov) const;

	std::unique_ptr<Tga::Camera> myCamera;

	Timer* myTimer;

	Tga::Vector2f myResolution;

	Tga::Vector2f myPosition;

	float myDepth;
	float myHeight;

	float myVerticalFov;
	float myNearZ;
	float myFarZ;

	ShakeData myShakeData;
};