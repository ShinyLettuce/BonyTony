#include "AnimationClipDocument.h"

#include "imgui_widgets/imgui_widgets.h"
#include "imgui_internal.h" // for DockBuilder Api

#include <tge/engine.h>
#include <tge/imgui/ImGuiPropertyEditor.h>

#include <AnimationClip/Commands/ChangeAnimationClipCommand.h>

#include <Editor.h>
#include <p4/p4.h>

using namespace Tga;

void AnimationClipDocument::Init(std::string_view aPath)
{
	Document::Init(aPath);

	myViewport.Init();
	myViewport.GetGrid().SetGridLineExtreme(400.0f);

	myPath = StringRegistry::RegisterOrGetString(aPath);
	myAnimationClip = GetOrCreateAnimationClip(myPath);

	std::filesystem::path path = aPath;
	myName = StringRegistry::RegisterOrGetString(path.stem().replace_extension("").string());

	char buffer[512];
	char asterix[2] = { 0, 0 };

	sprintf_s(buffer, "%s%s###Document:%s", myName.GetString(), asterix, myPath.GetString());
	myImGuiName = StringRegistry::RegisterOrGetString(buffer);

	sprintf_s(buffer, "Properties##Document:%s", myPath.GetString());
	myPanelWindowNames[(size_t)Panels::Properties] = buffer;
	sprintf_s(buffer, "Viewport##Document:%s", myPath.GetString());
	myPanelWindowNames[(size_t)Panels::Viewport] = buffer;

	Camera& camera = myViewport.GetCamera();
	Vector2i resolution = myViewport.GetViewportSize();
	camera.SetPerspectiveProjection(
		60,
		{
			(float)resolution.x,
			(float)resolution.y
		},
		0.1f,
		50000.0f
	);

	Vector3f cameraRotation = { 45, 45, 0 };

	camera.GetTransform().SetRotation(cameraRotation);
	myViewport.SetCameraRotation(cameraRotation);
	camera.GetTransform().SetPosition((camera.GetTransform().GetForward() * -myViewport.GetCameraFocusDistance()));
}

void AnimationClipDocument::Save()
{
	SaveAnimationClip(myPath);

	mySaveUndoStackSize = myUndoStackSize;
}

void AnimationClipDocument::Update(float aTimeDelta, InputManager& inputManager)
{
	aTimeDelta; inputManager;

	// clearing out caches every frame to support updates to assets while the editor is running
	myCache.ClearCache();

	const Camera& renderCamera = myViewport.GetCamera();
	Frustum frustum = CalculateFrustum(renderCamera);

	{
		

		myViewport.BeginDraw();

		{
			myViewport.SetupIdPass();

		}

		{
			myViewport.SetupColorPass();

		}

		myViewport.EndDraw();
	}

	char buffer[512];
	char asterix[2] = { 0, 0 };

	// Todo: all of this base imgui stuff should move to the Document base class
	if (mySaveUndoStackSize != myUndoStackSize)
		asterix[0] = '*';

	sprintf_s(buffer, "%s%s###Document:%s", myName.GetString(), asterix, myPath.GetString());

	if (!myIsDockingInitialized)
	{
		ImGui::DockBuilderSetNodeSize(Editor::GetEditor()->GetDocumentDockSpaceId(), Editor::GetEditor()->GetDocumentDockSpaceSize());
		ImGui::DockBuilderDockWindow(buffer, Editor::GetEditor()->GetDocumentDockSpaceId());

		ImGui::DockBuilderFinish(Editor::GetEditor()->GetDocumentDockSpaceId());
	}

	ImGui::SetNextWindowClass(Editor::GetEditor()->GetDocumentWindowClass());
	ImGui::SetNextWindowDockID(Editor::GetEditor()->GetDocumentDockSpaceId(), ImGuiCond_Once);

	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1);

		bool open = true;
		ImGui::Begin(buffer, &open);
		if (myState == Document::State::Open && !open)
		{
			myState = Document::State::CloseRequested;
		}
		ImGui::PopStyleVar(2);

		ImVec2 docSpaceSize = ImGui::GetContentRegionAvail();
		ImGuiID dockSpaceId = ImGui::GetID("Document Dockspace");
		// todo: ImGui::GetContentRegionAvail() returns wrong result first time it seems. What to do instead?
		ImGui::DockSpace(dockSpaceId, docSpaceSize, ImGuiDockNodeFlags_None, &myDocumentWindowClass);

		if (!myIsDockingInitialized)
		{
			ImGuiID center = 0, right = 0;

			ImGui::DockBuilderRemoveNode(dockSpaceId); // clear any previous layout
			ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dockSpaceId, docSpaceSize);

			center = dockSpaceId;

			ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.25f, &right, &center);

			ImGui::DockBuilderDockWindow(myPanelWindowNames[(size_t)Panels::Properties].c_str(), right);

			ImGui::DockBuilderDockWindow(myPanelWindowNames[(size_t)Panels::Viewport].c_str(), center);

			ImGui::DockBuilderFinish(dockSpaceId);

			myIsDockingInitialized = true;
		}

		ImGui::End();
	}

	const Tga::Color color = Tga::Engine::GetInstance()->GetClearColor();

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(color.myR, color.myG, color.myB, color.myA));
	ImGui::SetNextWindowClass(&myDocumentWindowClass);

	bool isViewportOrPropertiesFocused = false;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin(myPanelWindowNames[(size_t)Panels::Viewport].c_str());
	ImGui::PopStyleVar(1);

	isViewportOrPropertiesFocused = isViewportOrPropertiesFocused || ImGui::IsWindowFocused();
	myViewport.DrawAndUpdateViewportWindow(aTimeDelta, *this);

	ImGui::End();
	ImGui::PopStyleColor();

	ImGui::SetNextWindowClass(&myDocumentWindowClass);

	isViewportOrPropertiesFocused = isViewportOrPropertiesFocused || ImGui::IsWindowFocused();
	ImGui::Begin(myPanelWindowNames[(size_t)Panels::Properties].c_str());

	DrawPropertyPanel();

	ImGui::End();
}

void AnimationClipDocument::OnAction(CommandManager::Action action)
{
	if (action == CommandManager::Action::Do)
	{
		if (myUndoStackSize == 0)
		{
			P4::CheckoutFile(myPath.GetString());
		}

		// If doing something when the undo stack is lower than when we saved, it means we can't get back to the saved state
		if (myUndoStackSize < mySaveUndoStackSize)
			mySaveUndoStackSize = -1;

		myUndoStackSize++;
	}
	if (action == CommandManager::Action::PostRedo)
	{
		myUndoStackSize++;
	}
	if (action == CommandManager::Action::PostUndo)
	{
		myUndoStackSize--;
	}
	if (action == CommandManager::Action::Clear)
	{
		myUndoStackSize = 0;
	}
}


void AnimationClipDocument::DrawPropertyPanel()
{
	bool hasModifications = false;
	AnimationClip modifiedClip = *myAnimationClip;

	if (PropertyEditor::PropertyHeader("Animation Clip"))
	{
		if (PropertyEditor::BeginPropertyTable())
		{
			PropertyEditor::PropertyLabel();
			ImGui::Text("Animation Source");
			PropertyEditor::PropertyValue();
			ImGui::Text(modifiedClip.animationSourcePath.GetString());
			if (ImGui::Button("Set From AssetBrowser##Animation Model"))
			{
				StringId newValue = Editor::GetEditor()->GetAssetBrowser().GetSelectedAsset();
				std::string stringWithExtension = newValue.GetString();
				std::string::size_type pos = stringWithExtension.find(".fbx");
				if (pos != std::string::npos)
				{
					modifiedClip.animationSourcePath = newValue;
					hasModifications = true;
				}
			}

			PropertyEditor::PropertyLabel();
			ImGui::Text("Preview Model");
			PropertyEditor::PropertyValue();
			ImGui::Text(modifiedClip.previewModelPath.GetString());
			if (ImGui::Button("Set From AssetBrowser##Preview Model"))
			{
				StringId newValue = Editor::GetEditor()->GetAssetBrowser().GetSelectedAsset();
				std::string stringWithExtension = newValue.GetString();
				std::string::size_type pos = stringWithExtension.find(".fbx");
				if (pos != std::string::npos)
				{
					modifiedClip.previewModelPath = newValue;
					hasModifications = true;
				}
			}

			PropertyEditor::PropertyLabel();
			ImGui::Text("Start Time");
			PropertyEditor::PropertyValue();
			ImGui::DragFloat("##Start Frame", &modifiedClip.startTime) && modifiedClip.startTime != myAnimationClip->startTime;
			if (ImGui::IsItemDeactivatedAfterEdit())
				hasModifications = true;

			PropertyEditor::PropertyLabel();
			ImGui::Text("End Time");
			PropertyEditor::PropertyValue();
			ImGui::DragFloat("##End Frame", &modifiedClip.endTime) && modifiedClip.endTime != myAnimationClip->endTime;
			if (ImGui::IsItemDeactivatedAfterEdit())
				hasModifications = true;

			PropertyEditor::PropertyLabel();
			ImGui::Text("Is Looping");
			PropertyEditor::PropertyValue();
			if (ImGui::Checkbox("##Is Looping", &modifiedClip.isLooping) && modifiedClip.isLooping != myAnimationClip->isLooping)
				hasModifications = true;

			PropertyEditor::PropertyLabel();
			ImGui::Text("Is Syncronized");
			PropertyEditor::PropertyValue();
			if (ImGui::Checkbox("##Is Syncronized", &modifiedClip.isSyncronized) && modifiedClip.isSyncronized != myAnimationClip->isSyncronized)
				hasModifications = true;

			PropertyEditor::PropertyLabel();
			ImGui::Text("Syncronized Cycle Offset");
			PropertyEditor::PropertyValue();
			ImGui::DragFloat("##Syncronized Cycle Offset", &modifiedClip.cycleOffsetPercentage, 1.f, 0.f, 1.f) && modifiedClip.cycleOffsetPercentage != myAnimationClip->cycleOffsetPercentage;
			if (ImGui::IsItemDeactivatedAfterEdit())
				hasModifications = true;		

			PropertyEditor::PropertyLabel();
			ImGui::Text("Syncronized Cycle Count");
			PropertyEditor::PropertyValue();
			ImGui::DragFloat("##Syncronized Cycle Count", &modifiedClip.cycleCount) && modifiedClip.cycleCount != myAnimationClip->cycleCount;
			if (ImGui::IsItemDeactivatedAfterEdit())
				hasModifications = true;

			PropertyEditor::EndPropertyTable();
		}
	}

	if (hasModifications)
	{
		std::shared_ptr<ChangeAnimationClipCommand> command = std::make_shared<ChangeAnimationClipCommand>(*myAnimationClip, modifiedClip);
		CommandManager::DoCommand(command);
	}

	/*
	StringId animationSourcePath;
	StringId selectedAnimationName;

	StringId previewModelPath;

	int startFrame;
	int endFrame;
	int cycleOffsetInFrames;
	bool isLooping;
	*/

	/*
	
	if (PropertyEditor::PropertyHeader("Default Value"))
	{
		if (PropertyEditor::BeginPropertyTable())
		{
			PropertyEditor::PropertyLabel();
			ImGui::Text("Preview Pixel Shader");
			PropertyEditor::PropertyValue();
			ImGui::Text(locPreviewSettings.previewPixelShaderPath.GetString());


			if (ImGui::Button("Set To Unlit"))
			{
				locPreviewSettings.previewPixelShaderPath = "shaders/model_shader_PS"_tgaid;
				UpdatePreviewShaders();
			}
			ImGui::SameLine();
			if (ImGui::Button("Set To PBR"))
			{
				locPreviewSettings.previewPixelShaderPath = "shaders/PbrModelShaderPS"_tgaid;
				UpdatePreviewShaders();
			}
			if (ImGui::Button("Set From AssetBrowser"))
			{
				StringId newValue = Editor::GetEditor()->GetAssetBrowser().GetSelectedAsset();
				std::string stringWithExtension = newValue.GetString();
				std::string::size_type pos = stringWithExtension.find(".hlsl");
				if (pos != std::string::npos)
				{
					std::string withoutPath = stringWithExtension.substr(0, pos);
					locPreviewSettings.previewPixelShaderPath = StringRegistry::RegisterOrGetString(withoutPath);
					UpdatePreviewShaders();
				}
			}
			PropertyEditor::PropertyLabel();
			ImGui::Text("Directional Light Yaw");
			PropertyEditor::PropertyValue();
			ImGui::DragFloat("##Light Yaw", &locPreviewSettings.directionalLightYaw);

			PropertyEditor::PropertyLabel();
			ImGui::Text("Directional Light Pitch");
			PropertyEditor::PropertyValue();
			ImGui::DragFloat("##Light Pitch", &locPreviewSettings.directionalLightPitch);

			PropertyEditor::PropertyLabel();
			ImGui::Text("Directional Light Color");
			PropertyEditor::PropertyValue();
			ImGui::ColorEdit3("##Directional Light Color", &locPreviewSettings.directionalLightColor.r, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);

			PropertyEditor::PropertyLabel();
			ImGui::Text("Directional Light Multiplier");
			PropertyEditor::PropertyValue();
			ImGui::DragFloat("##Directional Light Color", &locPreviewSettings.directionalLightColorMultiplier);

			PropertyEditor::PropertyLabel();
			ImGui::Text("Ambient Light Color");
			PropertyEditor::PropertyValue();
			ImGui::ColorEdit3("##Ambient Light Color", &locPreviewSettings.ambientColor.r, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);

			PropertyEditor::PropertyLabel();
			ImGui::Text("Ambient Light Multiplier");
			PropertyEditor::PropertyValue();
			ImGui::DragFloat("##Ambient Light Color", &locPreviewSettings.ambientColorMultiplier);

			PropertyEditor::EndPropertyTable();
		}
	}
	*/
}

void AnimationClipDocument::HandleDrop()
{

}

void AnimationClipDocument::BeginDragSelection(Vector2f mousePos)
{
	mousePos;
}

void AnimationClipDocument::EndDragSelection(Vector2f mousePos, bool isShiftDown)
{
	mousePos;
	isShiftDown;
}

void AnimationClipDocument::ClickSelection(Vector2f mousePos, uint32_t selectedId, bool isShiftDown)
{
	mousePos;
	selectedId;
	isShiftDown;
}

void AnimationClipDocument::BeginTransformation()
{

}

void AnimationClipDocument::UpdateTransformation(const Vector3f& referencePosition, const Matrix4x4f& transform)
{
	referencePosition;
	transform;
}

void AnimationClipDocument::EndTransformation()
{
}

Vector3f AnimationClipDocument::CalculateSelectionPosition()
{
	return {};
}

Matrix4x4f AnimationClipDocument::CalculateSelectionOrientation()
{
	return {};
}

bool AnimationClipDocument::HasTransformableSelection()
{
	return false;
}