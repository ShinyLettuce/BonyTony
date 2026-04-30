#pragma once

#include <vector>
#include <functional>

#include <tge/math/vector2.h>
#include <tge/sprite/sprite.h>
#include <tge/graphics/GraphicsEngine.h>
#include <tge/drawers/SpriteDrawer.h>
#include <tge/texture/TextureManager.h>
#include <tge/engine.h>

class InputMapper;

namespace UI
{
	struct PolygonHitArea
	{
		std::vector<Tga::Vector2f> offsetsRef;
		std::vector<Tga::Vector2f> world;

		void Clear()
		{
			offsetsRef.clear();
			world.clear();
		}

		bool HasValidPolygon() const
		{
			return world.size() >= 3;
		}

		void RebuildWorld(const Tga::Vector2f& originWorld, float uiScale)
		{
			world.clear();
			world.reserve(offsetsRef.size());

			for (const auto& o : offsetsRef)
			{
				world.push_back(originWorld + Tga::Vector2f{ o.x * uiScale, o.y * uiScale });
			}
		}

		bool ContainsPointConvex(const Tga::Vector2f& pWorld) const;
	};

	float Cross2D(const Tga::Vector2f& a, const Tga::Vector2f& b);
	bool PointInConvexPolygon(const Tga::Vector2f& p, const std::vector<Tga::Vector2f>& verts);

#ifndef _RETAIL
	void DebugDrawPolygon(const std::vector<Tga::Vector2f>& poly, const Tga::Color& color);

	class PolygonRecorder
	{
	public:
		void DrawWindowAndHandleRecording(
			const char* windowTitle,
			InputMapper* input,
			bool allowMouseUI,
			const Tga::Vector2f& originWorld,
			float uiScale,
			const std::function<void(const std::vector<Tga::Vector2f>& offsetsRef)>& onApplyOffsetsRef,
			const char* targetLabel = nullptr
		);

		const std::vector<Tga::Vector2f>& GetRecordedWorldPoints() const { return myRecordedWorldPoints; }
		void ClearRecorded() { myRecordedWorldPoints.clear(); }

		bool IsRecording() const { return myRecordingEnabled; }

	private:
		bool myRecordingEnabled = false;
		std::vector<Tga::Vector2f> myRecordedWorldPoints;
	};
#endif
}

