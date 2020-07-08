#pragma once

#include <string>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Lucid/Core/Layer.h"
#include "Lucid/Core/Events/KeyEvent.h"
#include "Lucid/Core/Events/MouseEvent.h"

#include "Lucid/Scene/SceneHierarchy.h"

#include "Lucid/Renderer/Renderer.h"

class EditorLayer : public Layer
{

public:

	enum class PropertyFlag
	{
		None = 0,
		ColourProperty = 1
	};

public:

	EditorLayer();
	EditorLayer(const std::string& name);
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

	void ShowBoundingBoxes(bool show, bool onTop = false);

private:

	std::pair<float, float> GetMouseViewportSpace();
	std::pair<glm::vec3, glm::vec3> CastRay(float mx, float my);

	#pragma region Scene

	Scope<SceneHierarchy> m_SceneHierarchy;

	Ref<Scene> m_Scene;
	Ref<Scene> m_ActiveScene;

	Entity* m_MeshEntity = nullptr;

	#pragma endregion

	#pragma region Textures

	struct DiffuseInput
	{
		glm::vec3 Colour = { 0.5f, 0.5f, 0.5f };
		Ref<Texture2D> TextureMap;
		bool SRGB = true;
		bool UseTexture = false;
	};

	DiffuseInput m_DiffuseInput;

	struct NormalInput
	{
		Ref<Texture2D> TextureMap;
		bool UseTexture = false;
	};

	NormalInput m_NormalInput;

	struct SpecularInput
	{
		float Value = 0.2f;
		Ref<Texture2D> TextureMap;
		bool UseTexture = false;
	};

	SpecularInput m_SpecularInput;

	Ref<Material> m_MeshMaterial;

	#pragma endregion

	#pragma region Editor

	Ref<Texture2D> m_CheckerboardTex;

	float m_GridScale = 16.025f;
	float m_GridSize = 0.025f;

	glm::vec2 m_ViewportBounds[2];

	// A value of -1 indicates no gizmo
	int m_GizmoType = -1;
	float m_SnapValue = 0.5f;

	bool m_AllowViewportCameraEvents = false;
	bool m_DrawOnTopBoundingBoxes = false;

	bool m_UIShowBoundingBoxes = false;
	bool m_UIShowBoundingBoxesOnTop = false;

	struct SelectedSubmesh
	{
		Submesh* Mesh;
		float Distance;
	};

	std::vector<SelectedSubmesh> m_SelectedSubmeshes;

	glm::mat4* m_CurrentlySelectedTransform = nullptr;

	#pragma endregion

};