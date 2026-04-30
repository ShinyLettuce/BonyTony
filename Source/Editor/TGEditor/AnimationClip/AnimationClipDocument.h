#pragma once

#include <imgui.h>

#include <Document/Document.h>
#include <Tools/Viewport/Viewport.h>
#include <tge/animation/AnimationClip.h>
#include <scene/SceneUtil.h>

namespace Tga
{
	class AnimationClipDocument : public Document, public ViewportInterface
	{
	public:
		enum class Panels
		{
			Properties,
			Viewport,
			Count,
		};

		void Init(std::string_view path) override;
		void Update(float aTimeDelta, InputManager& inputManager) override;
		void Save() override;
		void OnAction(CommandManager::Action action) override;

		void HandleDrop() override;
		void BeginDragSelection(Vector2f mousePos) override;
		void EndDragSelection(Vector2f mousePos, bool isShiftDown) override;
		void ClickSelection(Vector2f mousePos, uint32_t selectedId, bool isShiftDown) override;

		void BeginTransformation() override;
		void UpdateTransformation(const Vector3f& referencePosition, const Matrix4x4f& transform) override;
		void EndTransformation() override;

		Vector3f CalculateSelectionPosition() override;
		Matrix4x4f CalculateSelectionOrientation() override;

		virtual bool HasTransformableSelection() override;
	private:
		void DrawPropertyPanel();
		
		EditorViewport myViewport;
		SceneCache myCache;

		StringId myPath;
		StringId myName;

		AnimationClip* myAnimationClip;

		bool myIsDockingInitialized = false;

		std::string myPanelWindowNames[(size_t)Panels::Count];

	};

}