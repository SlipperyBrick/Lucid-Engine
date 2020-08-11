#pragma once

#include <string>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Lucid/ImGui/ImGuiLayer.h"

#include "Lucid/Scene/SceneHierarchy.h"

#include "Lucid/ImGui/EditorCamera.h"

#include "Lucid/Core/Events/KeyEvent.h"

#include "Lucid/Core/Math/Ray.h"

class EditorLayer : public Layer
{

public:

	enum class PropertyFlag
	{
		None = 0,
		ColourProperty = 1,
		DragProperty = 2, 
		SliderProperty = 4
	};

public:

	EditorLayer();
	virtual ~EditorLayer();

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(Timestep ts) override;

	virtual void OnImGuiRender() override;
	virtual void OnEvent(Event& e) override;

	bool OnKeyPressedEvent(KeyPressedEvent& e);
	bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

	// ImGui UI helpers
	bool Property(const std::string& name, bool& value);
	bool Property(const std::string& name, float& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);
	bool Property(const std::string& name, glm::vec2& value, PropertyFlag flags);
	bool Property(const std::string& name, glm::vec2& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);
	bool Property(const std::string& name, glm::vec3& value, PropertyFlag flags);
	bool Property(const std::string& name, glm::vec3& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);
	bool Property(const std::string& name, glm::vec4& value, PropertyFlag flags);
	bool Property(const std::string& name, glm::vec4& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);
	bool Property(const Ref<Texture2D>& texture, float& value, float min = 1.0f, float max = 10.0f, float sliderWidth = 100.0, PropertyFlag flags = PropertyFlag::None);

	void ShowBoundingBoxes(bool show);

	void SelectEntity(Entity entity);

private:

	void UpdateWindowTitle(const std::string& sceneName);

	std::pair<float, float> GetMouseViewportSpace();
	std::pair<glm::vec3, glm::vec3> CastRay(float mx, float my);

	struct SelectedSubmesh
	{
		Entity Entity;

		Submesh* Mesh = nullptr;

		float Distance = 0.0f;
	};

	void OnSelected(const SelectedSubmesh& selectionContext);
	void OnEntityDeleted(Entity e);

	Ray CastMouseRay();

private:

	#pragma region Scene

	EditorCamera m_EditorCamera;

	Scope<SceneHierarchy> m_SceneHierarchy;

	Ref<Scene> m_ActiveScene;

	#pragma endregion

	#pragma region Editor

	Ref<Texture2D> m_DepthPeelingTex;
	Ref<Texture2D> m_TranslateTex;
	Ref<Texture2D> m_RotateTex;
	Ref<Texture2D> m_ScaleTex;
	Ref<Texture2D> m_BoundingBoxesTex;
	Ref<Texture2D> m_PointerTex;
	Ref<Texture2D> m_GridToggleTex;
	Ref<Texture2D> m_CameraSpeedTex;
	Ref<Texture2D> m_GridSnapTex;
	Ref<Texture2D> m_DuplicateTex;
	Ref<Texture2D> m_PositionsTex;
	Ref<Texture2D> m_NormalsTex;
	Ref<Texture2D> m_AlbedoTex;
	Ref<Texture2D> m_SpecularTex;

	float m_GridScale = 16.025f;
	float m_GridSize = 0.025f;

	glm::vec2 m_ViewportBounds[2];

	// A value of -1 indicates no gizmo
	int m_GizmoType = -1;
	float m_SnapValue = 0.5f;

	bool m_Snap = false;

	bool m_AllowViewportCameraEvents = false;
	bool m_DrawOnTopBoundingBoxes = false;

	bool m_UIShowBoundingBoxes = false;

	bool m_ViewportPanelMouseOver = false;
	bool m_ViewportPanelFocused = false;

	enum class SelectionMode
	{
		None = 0,
		Entity = 1,
		SubMesh = 2
	};

	SelectionMode m_SelectionMode = SelectionMode::Entity;

	std::vector<SelectedSubmesh> m_SelectionContext;

	glm::mat4* m_RelativeTransform = nullptr;
	glm::mat4* m_CurrentlySelectedTransform = nullptr;

	#pragma endregion

};