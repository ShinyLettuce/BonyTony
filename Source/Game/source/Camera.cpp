#include "Camera.h"

#include <tge/engine.h>
#include <tge/graphics/GraphicsEngine.h>
#include <tge/graphics/GraphicsStateStack.h>
#include <tge/graphics/DX11.h>
#include <tge/drawers/LineDrawer.h>
#include <tge/primitives/LinePrimitive.h>

#include "MathUtils.h"

#include <imgui/imgui.h>

void Camera::Init(Timer* aTimer)
{
	myTimer = aTimer;

	Tga::Engine& engine = *Tga::Engine::GetInstance();

	Tga::Vector2ui renderSize = engine.GetRenderSize();
	myResolution = Tga::Vector2f{ static_cast<float>(renderSize.x), static_cast<float>(renderSize.y) };

	myNearZ = 200.0f;
	myFarZ = 3500.0f;

	myCamera = std::make_unique<Tga::Camera>();
	myCamera->SetPerspectiveProjection(90.0f, myResolution, myNearZ, myFarZ);
	myCamera->GetTransform().Translate(Tga::Vector3f{ 0.0f, 0.0f, -1000.0f });
}

/// <summary>
/// Shakes the camrea with the given parameters 
/// Base values for the shotgun is: Shake(7.0f, 5.0f, 0.5f);
/// Base values for the revolver is: Shake(3.0f, 7.0f, 0.4f);
/// </summary>
void Camera::Shake(const float aAmplitude, const float aFrequency, const float aDuration)
{
	myShakeData.time = static_cast<float>(myTimer->GetTotalTime());
	myShakeData.amplitude = aAmplitude;
	myShakeData.frequency = aFrequency;
	myShakeData.duration = aDuration;
	myShakeData.remaining = aDuration;
}

void Camera::SetDepth(float aDepth)
{
	myDepth = aDepth;
	myCamera->GetTransform().SetPosition(Tga::Vector3f{ 0.0f, 0.0f, aDepth });
}

void Camera::SetHeight(float aHeight)
{
	myHeight = aHeight;
}

void Camera::SetFov(float aHorizontalFov)
{
	//aHorizontalFov *= 3.14159265f / 180.f;
	myVerticalFov = HorizontalToVerticalFov(aHorizontalFov);
	myCamera->SetPerspectiveProjection(aHorizontalFov, myResolution, myNearZ, myFarZ);
}

void Camera::MoveToPosition(Tga::Vector2f aPosition)
{
	myPosition = aPosition;
}

void Camera::MoveTowardsPosition(Tga::Vector2f aPosition, float aFactor, float aDeltaTime)
{
	myPosition = aPosition + (myPosition - aPosition) * std::exp(-aFactor * aDeltaTime);
}

void Camera::MoveTowardsPosition(Tga::Vector2f aPosition, Tga::Vector2f aReferencePoint, float aFactor, float aDeltaTime)
{
	constexpr float maxDistance = 400.0f;
	constexpr float maxMultiplier = 2.0f;

	float factor = aFactor;

	if (myPosition.y > aReferencePoint.y)
	{
		factor = MathUtils::LerpClamped(aFactor, aFactor * maxMultiplier, std::abs(myPosition.y - aReferencePoint.y) / maxDistance);
	}

	myPosition = aPosition + (myPosition - aPosition) * std::exp(-factor * aDeltaTime);
}

Tga::Vector3f Camera::GetScreenToWorldPoint(const Tga::Vector2f& aPoint)
{
	Tga::Engine& engine = *Tga::Engine::GetInstance();

	Tga::Vector2ui renderSize = engine.GetRenderSize();
	Tga::Vector2f resolution = Tga::Vector2f{ static_cast<float>(renderSize.x), static_cast<float>(renderSize.y) };

	Tga::Vector2f ndc
	{
		((2.0f * aPoint.x) / resolution.x) - 1.0f,
		1.0f - ((2.0f * aPoint.y) / resolution.y)
	};
	Tga::Matrix4x4 projection = myCamera->GetProjection();
	Tga::Vector2f view
	{
		ndc.x / projection(1, 1),
		ndc.y / projection(2, 2)
	};
	Tga::Vector4f world = Tga::Vector4f{ view, 1.0f, 1.0f } *myCamera->GetTransform();

	Tga::Vector3f rayOrigin = myCamera->GetTransform().GetPosition();
	Tga::Vector3f rayDirection = Tga::Vector3f{ world } - rayOrigin;

	rayDirection.Normalize();

	Tga::Vector3f planeOrigin{ Tga::Vector3f::Zero };
	Tga::Vector3f planeNormal{ Tga::Vector3f::Forward };

	const float denominator = planeNormal.Dot(rayDirection);
	if (std::abs(denominator) < 1e-6f)
	{
		return Tga::Vector3f::Zero;
	}

	const float t = (planeOrigin - rayOrigin).Dot(planeNormal) / denominator;
	if (t >= 1e-6f)
	{
		return rayOrigin + rayDirection * t;
	}
	else
	{
		return Tga::Vector3f::Zero;
	}
}

Tga::Vector2f Camera::GetWorldToScreenPoint(const Tga::Vector2f& aPoint)
{
	auto& engine = *Tga::Engine::GetInstance();
	Tga::Vector2ui renderSize = engine.GetRenderSize();
	Tga::Vector2f renderSizef = {static_cast<float>(renderSize.x), static_cast<float>(renderSize.y)};
	
	Tga::Vector4f pWorld = {aPoint.x, aPoint.y, 0.f, 1.f};
	Tga::Vector4f pView = pWorld * myCamera->GetTransform().GetInverse();
	Tga::Vector4f pClip = pView * myCamera->GetProjection();
	Tga::Vector3f pNDC = pClip / pClip.W;
	Tga::Vector2f pScreen = Tga::Vector2f{
		((pNDC.x + 1.f)/2) * renderSizef.x,
		((1.f - pNDC.y)/2) * renderSizef.y
	};

	return Tga::Vector2f{pScreen.x, renderSizef.y - pScreen.y};
}

Tga::Camera& Camera::GetTgaCamera()
{
	return *myCamera;
}

void Camera::Update()
{
	const Tga::Vector2f heightOffset = Tga::Vector2f{ 0.0f, myHeight };
	myCamera->GetTransform().SetPosition(Tga::Vector3f{ myPosition + GetShakeOffset() + heightOffset, -myDepth });
}

void Camera::Prepare()
{
	Tga::DX11::BackBuffer->SetAsActiveTarget(Tga::DX11::DepthBuffer);

	Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::GraphicsEngine& graphicsEngine = engine.GetGraphicsEngine();
	Tga::GraphicsStateStack& graphicsStateStack = graphicsEngine.GetGraphicsStateStack();

	graphicsStateStack.SetCamera(*myCamera);
}

bool Camera::IsPointWithinFrustum(const Tga::Vector3f& aPoint, float aRadius) const
{	
	const Tga::Matrix4x4f& projection = myCamera->GetProjection();

	//Get camera information
	const float aspectRatio = (float)Tga::Engine::GetInstance()->GetRenderSize().x / (float)Tga::Engine::GetInstance()->GetRenderSize().y;
	const float halfHorizontalFoVRad = atan(1.0f / projection(1, 1));
	const float halfVerticalFovRad = halfHorizontalFoVRad / aspectRatio;

	//Lambda to rotate a vector around an arbitrary axis
	auto rotateAroundArbitraryAxis = [](Tga::Vector3<float> aVectorToRotate, Tga::Vector3<float> aRotationAxis, float aAngle) {
		return aVectorToRotate * Tga::Matrix3x3<float>{
			cos(aAngle) + (aRotationAxis.x * aRotationAxis.x * (1.0f - cos(aAngle))), aRotationAxis.x* aRotationAxis.y* (1.0f - cos(aAngle)) - aRotationAxis.z * sin(aAngle), aRotationAxis.x* aRotationAxis.z* (1.0f - cos(aAngle)) + aRotationAxis.y * sin(aAngle),
			aRotationAxis.y* aRotationAxis.x* (1.0f - cos(aAngle)) + aRotationAxis.z * sin(aAngle), cos(aAngle) + aRotationAxis.y * aRotationAxis.y * (1.0f - cos(aAngle)), aRotationAxis.y* aRotationAxis.z* (1.0f - cos(aAngle)) - aRotationAxis.x * sin(aAngle),
			aRotationAxis.z* aRotationAxis.x* (1.0f - cos(aAngle)) - aRotationAxis.y * sin(aAngle), aRotationAxis.z* aRotationAxis.y* (1.0f - cos(aAngle)) + aRotationAxis.x * sin(aAngle), cos(aAngle) + aRotationAxis.z * aRotationAxis.z * (1.0f - cos(aAngle))
		};
	};

	const Tga::Matrix4x4f& transform = myCamera->GetTransform();
	const Tga::Vector3f position = transform.GetPosition();

	//Rotate camera direction vectors with FOV angles to get normals to all the planes
	Tga::Vector3f rightPlaneNormal = rotateAroundArbitraryAxis(transform.GetRight(), transform.GetUp(), -halfHorizontalFoVRad);
	Tga::Vector3f leftPlaneNormal = rotateAroundArbitraryAxis(-1.f * transform.GetRight(), transform.GetUp(), halfHorizontalFoVRad);
	Tga::Vector3f upPlaneNormal = rotateAroundArbitraryAxis(transform.GetUp(), transform.GetRight(), halfVerticalFovRad);
	Tga::Vector3f downPlaneNormal = rotateAroundArbitraryAxis(-1.f * transform.GetUp(), transform.GetRight(), -halfVerticalFovRad);

	const int frustumPlaneCount = 4;
	Tga::Vector3f frustumPlaneNormals[frustumPlaneCount] = { rightPlaneNormal, leftPlaneNormal, upPlaneNormal, downPlaneNormal };

	bool isWithinFrustum = true;

	for (int i = 0; i < frustumPlaneCount; ++i)
	{
		// Assume a default scaling of 5
		if (frustumPlaneNormals[i].Dot(aPoint - position) > 5.0f * aRadius)
		{
			isWithinFrustum = false;
			break;
		}
	}

	return isWithinFrustum;
}

void Camera::DrawScreenToWorldDebugGizmos(Tga::Vector2f aScreenPoint)
{
	Tga::Vector3f world = GetScreenToWorldPoint(aScreenPoint);
	world.z = 0.0f;
}

Tga::Vector2f Camera::GetShakeOffset()
{
	constexpr float pi = 3.1415927f;
	constexpr float tau = 2.0f * pi;

	if (myShakeData.remaining > 0.0f)
	{
		myShakeData.remaining -= myTimer->GetDeltaTime();

		const float scale = myShakeData.remaining / myShakeData.duration;

		const float frequency = myShakeData.frequency * tau * scale;
		const float amplitude = myShakeData.amplitude * scale * scale;

		const float cos = std::cos(myShakeData.time + myShakeData.remaining * frequency) * amplitude;
		const float sin = std::sin(myShakeData.time + myShakeData.remaining * frequency) * amplitude;

		Tga::Vector2f shakeOffset;

		shakeOffset.x = cos;
		shakeOffset.y = sin;

		return shakeOffset;
	}
	else
	{
		return Tga::Vector2f{ 0.0f, 0.0f };
	}
}

Camera::Frustum Camera::GetFrustum() const
{
	Frustum frustum;
	const float halfVSide = myFarZ * std::tanf(myVerticalFov * 0.5f);
	const float halfHSide = halfVSide * GetAspectRatio();
	const Tga::Matrix4x4 transform = myCamera->GetTransform();
	const Tga::Vector3f frontMultFar = myFarZ * transform.GetForward();

	frustum.right = Plane{ transform.GetPosition(), (frontMultFar - transform.GetRight() * halfHSide).Cross(transform.GetUp()) };
	frustum.left = Plane{ transform.GetPosition(), (transform.GetUp()).Cross(frontMultFar + transform.GetRight() * halfHSide) };
	frustum.top = Plane{ transform.GetPosition(), (transform.GetRight()).Cross(frontMultFar - transform.GetUp() * halfVSide) };
	frustum.bottom = Plane{ transform.GetPosition(), (frontMultFar + transform.GetUp() * halfVSide).Cross(transform.GetRight()) };

	return frustum;
}

float Camera::GetAspectRatio() const
{
	return myResolution.x / myResolution.y;
}

bool Camera::IsPointWithinPlane(Plane& aPlane, const Tga::Vector3f& aPoint) const
{
	return aPlane.normal.Dot(aPoint - aPlane.point) < 0.0f;
}

bool Camera::IsPointWithinFrustum(Frustum& aFrustum, const Tga::Vector3f& aPoint) const
{
	return bool
	{
		IsPointWithinPlane(aFrustum.top, aPoint) &&
		IsPointWithinPlane(aFrustum.bottom, aPoint) &&
		IsPointWithinPlane(aFrustum.left, aPoint) &&
		IsPointWithinPlane(aFrustum.right, aPoint)
	};
}

float Camera::HorizontalToVerticalFov(float aHorizontalFov) const
{
	Tga::Engine& engine = *Tga::Engine::GetInstance();

	Tga::Vector2ui renderSize = engine.GetRenderSize();
	Tga::Vector2f resolution = Tga::Vector2f{ static_cast<float>(renderSize.x), static_cast<float>(renderSize.y) };

	return 2.0f * std::atan(std::tan(aHorizontalFov / 2.0f) * (resolution.y / resolution.x));
}
