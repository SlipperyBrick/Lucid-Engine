#pragma once

#include <string>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Lucid/Core/Layer.h"
#include "Lucid/Core/Events/KeyEvent.h"
#include "Lucid/Core/Events/MouseEvent.h"

class EditorLayer : public Layer
{

public:

	enum class PropertyFlag
	{
		None = 0,
		ColorProperty = 1
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
	void Property(const std::string& name, float& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);
	void Property(const std::string& name, glm::vec2& value, PropertyFlag flags);
	void Property(const std::string& name, glm::vec2& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);
	void Property(const std::string& name, glm::vec3& value, PropertyFlag flags);
	void Property(const std::string& name, glm::vec3& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);
	void Property(const std::string& name, glm::vec4& value, PropertyFlag flags);
	void Property(const std::string& name, glm::vec4& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);

private:

	float m_GridScale = 16.025f;
	float m_GridSize = 0.025f;

	glm::vec2 m_ViewportBounds[2];

	// -1 indicates no gizmo
	int m_GizmoType = -1;
	float m_SnapValue = 0.5f;

	bool m_AllowViewportCameraEvents = false;
	bool m_DrawOnTopBoundingBoxes = false;

	glm::mat4* m_CurrentlySelectedTransform = nullptr;
};