#include "UIPolygonTools.h"

#include <cmath>

#ifndef _RETAIL
#include <imgui/imgui.h>
#include <iostream>

#include "InputMapper.h"

#include <tge/Engine.h>
#include <tge/graphics/GraphicsEngine.h>
#include <tge/drawers/LineDrawer.h>
#include <tge/primitives/LinePrimitive.h>
#include <tge/math/vector3.h>
#endif

namespace UI
{
	float Cross2D(const Tga::Vector2f& a, const Tga::Vector2f& b)
	{
		return a.x * b.y - a.y * b.x;
	}

	bool PointInConvexPolygon(const Tga::Vector2f& p, const std::vector<Tga::Vector2f>& verts)
	{
		const size_t n = verts.size();
		if (n < 3)
			return false;

		int sign = 0;
		constexpr float eps = 1e-6f;

		for (size_t i = 0; i < n; ++i)
		{
			const Tga::Vector2f& a = verts[i];
			const Tga::Vector2f& b = verts[(i + 1) % n];

			const Tga::Vector2f ab{ b.x - a.x, b.y - a.y };
			const Tga::Vector2f ap{ p.x - a.x, p.y - a.y };

			const float c = Cross2D(ab, ap);

			if (std::fabs(c) < eps)
				continue;

			const int currentSign = (c > 0.0f) ? 1 : -1;

			if (sign == 0)
				sign = currentSign;
			else if (currentSign != sign)
				return false;
		}

		return true;
	}

	bool PolygonHitArea::ContainsPointConvex(const Tga::Vector2f& pWorld) const
	{
		return PointInConvexPolygon(pWorld, world);
	}

#ifndef _RETAIL
	void DebugDrawPolygon(const std::vector<Tga::Vector2f>& poly, const Tga::Color& color)
	{
		if (poly.size() < 2)
			return;

		Tga::LineDrawer& lineDrawer = Tga::Engine::GetInstance()->GetGraphicsEngine().GetLineDrawer();

		for (size_t i = 0; i < poly.size(); ++i)
		{
			const Tga::Vector2f& a = poly[i];
			const Tga::Vector2f& b = poly[(i + 1) % poly.size()];

			Tga::LinePrimitive line{};
			line.fromPosition = Tga::Vector3f(a, 0.0f);
			line.toPosition = Tga::Vector3f(b, 0.0f);
			line.color = color.AsVec4();

			lineDrawer.Draw(line);
		}
	}

	void PolygonRecorder::DrawWindowAndHandleRecording(
		const char* windowTitle,
		InputMapper* input,
		bool allowMouseUI,
		const Tga::Vector2f& originWorld,
		float uiScale,
		const std::function<void(const std::vector<Tga::Vector2f>& offsetsRef)>& onApplyOffsetsRef,
		const char* targetLabel
	)
	{
		if (ImGui::Begin(windowTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Checkbox("Record polygon points (LMB adds point)", &myRecordingEnabled);
			ImGui::Text("Recorded points: %d", static_cast<int>(myRecordedWorldPoints.size()));

			if (ImGui::Button("Clear recorded points"))
			{
				myRecordedWorldPoints.clear();
			}

			if (ImGui::Button("Apply recorded -> target hit poly"))
			{
				std::vector<Tga::Vector2f> offsetsRef;
				offsetsRef.reserve(myRecordedWorldPoints.size());

				for (const auto& p : myRecordedWorldPoints)
				{
					const Tga::Vector2f offsetPx{ p.x - originWorld.x, p.y - originWorld.y };
					offsetsRef.push_back({ offsetPx.x / uiScale, offsetPx.y / uiScale });
				}

				if (onApplyOffsetsRef)
				{
					onApplyOffsetsRef(offsetsRef);
				}
			}

			if (ImGui::Button("Print offsets (reference units) from origin"))
			{
				if (targetLabel && targetLabel[0] != '\0')
				{
					std::cout << "==== " << targetLabel << " ====\n";
				}
				else
				{
					std::cout << "==== (No target label) ====\n";
				}

				std::cout << "---- Reference-unit offsets from origin ----\n";
				for (const auto& p : myRecordedWorldPoints)
				{
					const Tga::Vector2f offsetPx{ p.x - originWorld.x, p.y - originWorld.y };
					const Tga::Vector2f offsetRef{ offsetPx.x / uiScale, offsetPx.y / uiScale };
					std::cout << "{ " << offsetRef.x << "f, " << offsetRef.y << "f },\n";
				}
				std::cout << "-------------------------------------------\n";
			}
		}
		ImGui::End();

		if (myRecordingEnabled && input != nullptr && allowMouseUI)
		{
			if (input->IsActionJustActivated(GameAction::UILeftClick))
			{
				const Tga::Vector2f p = input->GetMousePositionYUp();
				myRecordedWorldPoints.push_back(p);
				std::cout << "[UIPolygonTools] Added point: (" << p.x << ", " << p.y << ")\n";
			}
		}
	}
#endif
}