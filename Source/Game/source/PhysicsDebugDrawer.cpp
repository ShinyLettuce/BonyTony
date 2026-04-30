#include "PhysicsDebugDrawer.h"

#include <tge/engine.h>
#include <tge/graphics/GraphicsEngine.h>
#include <tge/drawers/LineDrawer.h>
#include <tge/primitives/LinePrimitive.h>
#include <imgui/imgui.h>

#include "SceneLoader.h"

namespace PhysicsDebugDrawer
{
	void DrawDebugAABB(Tga::Vector2f aPosition, Tga::Vector2f aSize)
	{
		UNREFERENCED_PARAMETER(aPosition);
		UNREFERENCED_PARAMETER(aSize);
#if defined(_DEBUG)
		Tga::Engine& engine = *Tga::Engine::GetInstance();
		Tga::GraphicsEngine& graphicsEngine = engine.GetGraphicsEngine();
		Tga::LineDrawer& lineDrawer = graphicsEngine.GetLineDrawer();

		const Tga::Vector2f position = aPosition;
		const Tga::Vector2f halfExtents = aSize / 2.0f;

		Tga::LinePrimitive top
		{
			.color = Tga::Vector4f{ 0.0f, 1.0f, 0.0f, 1.0f },
			.fromPosition = Tga::Vector3f{ position + Tga::Vector2f{ -halfExtents.x, halfExtents.y }, -51.0f },
			.toPosition = Tga::Vector3f{ position + Tga::Vector2f{ halfExtents.x, halfExtents.y }, -51.0f },
		};

		Tga::LinePrimitive bottom
		{
			.color = Tga::Vector4f{ 0.0f, 1.0f, 0.0f, 1.0f },
			.fromPosition = Tga::Vector3f{ position + Tga::Vector2f{ -halfExtents.x, -halfExtents.y }, -51.0f },
			.toPosition = Tga::Vector3f{ position + Tga::Vector2f{ halfExtents.x, -halfExtents.y }, -51.0f },
		};

		Tga::LinePrimitive left
		{
			.color = Tga::Vector4f{ 0.0f, 1.0f, 0.0f, 1.0f },
			.fromPosition = Tga::Vector3f{ position + Tga::Vector2f{ -halfExtents.x, -halfExtents.y }, -51.0f },
			.toPosition = Tga::Vector3f{ position + Tga::Vector2f{ -halfExtents.x, halfExtents.y }, -51.0f },
		};

		Tga::LinePrimitive right
		{
			.color = Tga::Vector4f{ 0.0f, 1.0f, 0.0f, 1.0f },
			.fromPosition = Tga::Vector3f{ position + Tga::Vector2f{ halfExtents.x, -halfExtents.y }, -51.0f },
			.toPosition = Tga::Vector3f{ position + Tga::Vector2f{ halfExtents.x, halfExtents.y }, -51.0f },
		};

		lineDrawer.Draw(top);
		lineDrawer.Draw(bottom);
		lineDrawer.Draw(left);
		lineDrawer.Draw(right);
#endif
	}

	void DrawDebugColliders(const Camera& aCamera)
	{
		UNREFERENCED_PARAMETER(aCamera);
#if !defined(_RETAIL)

		ImGui::Begin("Colliders");
		static bool drawColliders = false;
		ImGui::Checkbox("Draw Colliders", &drawColliders);
		if (drawColliders)
		{
			Tga::Engine& engine = *Tga::Engine::GetInstance();
			Tga::GraphicsEngine& graphicsEngine = engine.GetGraphicsEngine();
			Tga::LineDrawer& lineDrawer = graphicsEngine.GetLineDrawer();

			SceneLoader::SceneConfig& sceneConfig = SceneLoader::GetActiveScene();

			for (const auto& tile : sceneConfig.tileConfigs)
			{
				const Tga::Vector2f halfExtents = tile.size / 2.0f;
				const Tga::Vector2f position = tile.position + Tga::Vector2f{ 0.0f, halfExtents.y };

				if (aCamera.IsPointWithinFrustum(Tga::Vector3f{ position + Tga::Vector2f{ -halfExtents.x, halfExtents.y }, -51.0f }) || aCamera.IsPointWithinFrustum(Tga::Vector3f{ position + Tga::Vector2f{ halfExtents.x, halfExtents.y }, -51.0f }))
				{
					Tga::LinePrimitive top
					{
						.color = Tga::Vector4f{ 0.0f, 1.0f, 0.0f, 1.0f },
						.fromPosition = Tga::Vector3f{ position + Tga::Vector2f{ -halfExtents.x, halfExtents.y }, -51.0f },
						.toPosition = Tga::Vector3f{ position + Tga::Vector2f{ halfExtents.x, halfExtents.y }, -51.0f },
					};
					lineDrawer.Draw(top);
				}

				if (aCamera.IsPointWithinFrustum(Tga::Vector3f{ position + Tga::Vector2f{ -halfExtents.x, -halfExtents.y }, -51.0f }) || aCamera.IsPointWithinFrustum(Tga::Vector3f{ position + Tga::Vector2f{ halfExtents.x, -halfExtents.y }, -51.0f }))
				{
					Tga::LinePrimitive bottom
					{
						.color = Tga::Vector4f{ 0.0f, 1.0f, 0.0f, 1.0f },
						.fromPosition = Tga::Vector3f{ position + Tga::Vector2f{ -halfExtents.x, -halfExtents.y }, -51.0f },
						.toPosition = Tga::Vector3f{ position + Tga::Vector2f{ halfExtents.x, -halfExtents.y }, -51.0f },
					};
					lineDrawer.Draw(bottom);
				}

				if (aCamera.IsPointWithinFrustum(Tga::Vector3f{ position + Tga::Vector2f{ -halfExtents.x, -halfExtents.y }, -51.0f }) || aCamera.IsPointWithinFrustum(Tga::Vector3f{ position + Tga::Vector2f{ -halfExtents.x, halfExtents.y }, -51.0f }))
				{
					Tga::LinePrimitive left
					{
						.color = Tga::Vector4f{ 0.0f, 1.0f, 0.0f, 1.0f },
						.fromPosition = Tga::Vector3f{ position + Tga::Vector2f{ -halfExtents.x, -halfExtents.y }, -51.0f },
						.toPosition = Tga::Vector3f{ position + Tga::Vector2f{ -halfExtents.x, halfExtents.y }, -51.0f },
					};
					lineDrawer.Draw(left);
				}

				if (aCamera.IsPointWithinFrustum(Tga::Vector3f{ position + Tga::Vector2f{ halfExtents.x, -halfExtents.y }, -51.0f }) || aCamera.IsPointWithinFrustum(Tga::Vector3f{ position + Tga::Vector2f{ halfExtents.x, halfExtents.y }, -51.0f }))
				{
					Tga::LinePrimitive right
					{
						.color = Tga::Vector4f{ 0.0f, 1.0f, 0.0f, 1.0f },
						.fromPosition = Tga::Vector3f{ position + Tga::Vector2f{ halfExtents.x, -halfExtents.y }, -51.0f },
						.toPosition = Tga::Vector3f{ position + Tga::Vector2f{ halfExtents.x, halfExtents.y }, -51.0f },
					};
					lineDrawer.Draw(right);
				}
			}
		}
		ImGui::End();
#endif
	}


	void DrawDebugRay(Physics::Ray aRay, Tga::Vector4f aColor)
	{
		UNREFERENCED_PARAMETER(aRay);
		UNREFERENCED_PARAMETER(aColor);
#if !defined(_RETAIL)
		Tga::LinePrimitive line{
			.color = aColor,
			.fromPosition = Tga::Vector3f{ aRay.origin , 0.f},
			.toPosition = Tga::Vector3f{ aRay.origin , 0.f} + Tga::Vector3f{ aRay.direction , 0.f } *aRay.magnitude,
		};
		Tga::Engine& engine = *Tga::Engine::GetInstance();
		Tga::GraphicsEngine& graphicsEngine = engine.GetGraphicsEngine();
		Tga::LineDrawer& lineDrawer = graphicsEngine.GetLineDrawer();

		lineDrawer.Draw(line);
#endif
	}

	void DrawDebugRayCone(Physics::Ray aRay, int aRayAmount, float aSpreadAngle)
	{
		UNREFERENCED_PARAMETER(aRay);
		UNREFERENCED_PARAMETER(aRayAmount);
		UNREFERENCED_PARAMETER(aSpreadAngle);
#if !defined(_RETAIL)
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

		for (int j = 0; j < aRayAmount; ++j)
		{
			aRay.direction = {
				std::cos(startAngle + deltaAngle * j),
				std::sin(startAngle + deltaAngle * j)
			};
			DrawDebugRay(aRay);
		}
#endif
	}
}
