#include "ldpch.h"

#include "EditorLayer.h"

#include <filesystem>

#include "Lucid/ImGui/ImGuiGizmo.h"

#include "Lucid/Core/Application.h"

#include "Lucid/Renderer/Renderer2D.h"
#include "Lucid/Renderer/Renderer.h"
#include "Lucid/Renderer/SceneRenderer.h"

#include "Lucid/Scene/SceneSerializer.h"

#pragma region ImGui UI Helpers

bool EditorLayer::Property(const std::string& name, bool& value)
{
	ImGui::Text(name.c_str());
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);

	std::string id = "##" + name;
	bool result = ImGui::Checkbox(id.c_str(), &value);

	ImGui::PopItemWidth();
	ImGui::NextColumn();

	return result;
}

bool EditorLayer::Property(const std::string& name, float& value, float min, float max, PropertyFlag flags)
{
	ImGui::Text(name.c_str());
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);

	std::string id = "##" + name;
	bool changed = false;

	if (flags == PropertyFlag::SliderProperty)
	{
		changed = ImGui::SliderFloat(id.c_str(), &value, min, max);
	}
	else
	{
		changed = ImGui::DragFloat(id.c_str(), &value, 1.0f, min, max);
	}

	ImGui::PopItemWidth();
	ImGui::NextColumn();

	return changed;
}

bool EditorLayer::Property(const std::string& name, glm::vec2& value, EditorLayer::PropertyFlag flags)
{
	return Property(name, value, -1.0f, 1.0f, flags);
}

bool EditorLayer::Property(const std::string& name, glm::vec2& value, float min, float max, PropertyFlag flags)
{
	ImGui::Text(name.c_str());
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);

	std::string id = "##" + name;
	bool changed = false;

	if (flags == PropertyFlag::SliderProperty)
	{
		changed = ImGui::SliderFloat2(id.c_str(), glm::value_ptr(value), min, max);
	}
	else
	{
		changed = ImGui::DragFloat2(id.c_str(), glm::value_ptr(value), 1.0f, min, max);
	}

	ImGui::PopItemWidth();
	ImGui::NextColumn();

	return changed;
}

bool EditorLayer::Property(const std::string& name, glm::vec3& value, EditorLayer::PropertyFlag flags)
{
	return Property(name, value, -10.0f, 10.0f, flags);
}

bool EditorLayer::Property(const std::string& name, glm::vec3& value, float min, float max, EditorLayer::PropertyFlag flags)
{
	ImGui::Text(name.c_str());
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);

	std::string id = "##" + name;
	bool changed = false;

	if ((int)flags & (int)PropertyFlag::ColourProperty)
	{
		changed = ImGui::ColorEdit3(id.c_str(), glm::value_ptr(value), ImGuiColorEditFlags_NoInputs);
	}
	else if (flags == PropertyFlag::SliderProperty)
	{
		changed = ImGui::SliderFloat3(id.c_str(), glm::value_ptr(value), min, max);
	}
	else
	{
		changed = ImGui::DragFloat3(id.c_str(), glm::value_ptr(value), 1.0f, min, max);
	}

	ImGui::PopItemWidth();
	ImGui::NextColumn();

	return changed;
}

bool EditorLayer::Property(const std::string& name, glm::vec4& value, EditorLayer::PropertyFlag flags)
{
	return Property(name, value, -1.0f, 1.0f, flags);
}

bool EditorLayer::Property(const std::string& name, glm::vec4& value, float min, float max, EditorLayer::PropertyFlag flags)
{
	ImGui::Text(name.c_str());
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);

	std::string id = "##" + name;
	bool changed = false;

	if ((int)flags & (int)PropertyFlag::ColourProperty)
	{
		changed = ImGui::ColorEdit4(id.c_str(), glm::value_ptr(value), ImGuiColorEditFlags_NoInputs);
	}
	else if (flags == PropertyFlag::SliderProperty)
	{
		changed = ImGui::SliderFloat4(id.c_str(), glm::value_ptr(value), min, max);
	}
	else
	{
		changed = ImGui::DragFloat4(id.c_str(), glm::value_ptr(value), 1.0f, min, max);
	}

	ImGui::PopItemWidth();
	ImGui::NextColumn();

	return changed;
}

#pragma endregion

EditorLayer::EditorLayer()
	: m_EditorCamera(glm::perspectiveFov(glm::radians(45.0f), 1280.0f, 720.0f, 0.1f, 10000.0f))
{
}

EditorLayer::~EditorLayer()
{
}

void EditorLayer::OnAttach()
{
	// Set ImGui interface colours
	ImVec4* colours = ImGui::GetStyle().Colors;

	colours[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	colours[ImGuiCol_TextDisabled] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
	colours[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
	colours[ImGuiCol_ChildBg] = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
	colours[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.9f);
	colours[ImGuiCol_Border] = ImVec4(0.4f, 0.4f, 0.4f, 0.5f);
	colours[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	colours[ImGuiCol_FrameBg] = ImVec4(0.3f, 0.3f, 0.3f, 0.5f);
	colours[ImGuiCol_FrameBgHovered] = ImVec4(0.4f, 0.4f, 0.4f, 0.4f);
	colours[ImGuiCol_FrameBgActive] = ImVec4(0.4f, 0.4f, 0.4f, 0.6f);
	colours[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.0f);
	colours[ImGuiCol_TitleBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 1.0f);
	colours[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
	colours[ImGuiCol_MenuBarBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
	colours[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.5f);
	colours[ImGuiCol_ScrollbarGrab] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
	colours[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
	colours[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
	colours[ImGuiCol_CheckMark] = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
	colours[ImGuiCol_SliderGrab] = ImVec4(0.5f, 0.5f, 0.5f, 0.7f);
	colours[ImGuiCol_SliderGrabActive] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
	colours[ImGuiCol_Button] = ImVec4(0.4f, 0.4f, 0.4f, 0.4f);
	colours[ImGuiCol_ButtonHovered] = ImVec4(0.45f, 0.45f, 0.45f, 1.0f);
	colours[ImGuiCol_ButtonActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.0f);
	colours[ImGuiCol_Header] = ImVec4(0.7f, 0.7f, 0.7f, 0.3f);
	colours[ImGuiCol_HeaderHovered] = ImVec4(0.7f, 0.7f, 0.7f, 0.8f);
	colours[ImGuiCol_HeaderActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
	colours[ImGuiCol_Separator] = ImVec4(0.4f, 0.4f, 0.4f, 0.5f);
	colours[ImGuiCol_SeparatorHovered] = ImVec4(0.7f, 0.7f, 0.7f, 0.75f);
	colours[ImGuiCol_SeparatorActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
	colours[ImGuiCol_ResizeGrip] = ImVec4(0.9f, 0.9f, 0.9f, 0.25f);
	colours[ImGuiCol_ResizeGripHovered] = ImVec4(0.8f, 0.8f, 0.8f, 0.7f);
	colours[ImGuiCol_ResizeGripActive] = ImVec4(0.45f, 0.45f, 0.45f, 0.95f);
	colours[ImGuiCol_PlotLines] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
	colours[ImGuiCol_PlotLinesHovered] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
	colours[ImGuiCol_PlotHistogram] = ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
	colours[ImGuiCol_PlotHistogramHovered] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
	colours[ImGuiCol_TextSelectedBg] = ImVec4(0.9f, 0.9f, 0.9f, 0.35f);
	colours[ImGuiCol_ModalWindowDarkening] = ImVec4(0.8f, 0.8f, 0.8f, 0.35f);
	colours[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 1.0f, 0.9f);
	colours[ImGuiCol_NavHighlight] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
	colours[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
	colours[ImGuiCol_Tab] = ImVec4(0.5f, 0.5f, 0.5f, 0.6f);
	colours[ImGuiCol_TabActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
	colours[ImGuiCol_TabHovered] = ImVec4(0.5f, 0.5f, 0.5f, 0.5f);
	colours[ImGuiCol_TabUnfocusedActive] = ImVec4(0.5f, 0.5f, 0.5f, 0.5f);

	// Editor resources
	m_CheckerboardTex = Texture2D::Create("assets/textures/Checkerboard.tga");
	m_BoundingBoxesTex = Texture2D::Create("assets/textures/BoundingBoxes.tga");
	m_PointerTex = Texture2D::Create("assets/textures/Pointer.tga");
	m_TranslateTex = Texture2D::Create("assets/textures/Translate.tga");
	m_RotateTex = Texture2D::Create("assets/textures/Rotate.tga");
	m_ScaleTex = Texture2D::Create("assets/textures/Scale.tga");
	m_GridToggleTex = Texture2D::Create("assets/textures/GridToggle.tga");
	m_DuplicateTex = Texture2D::Create("assets/textures/Duplicate.tga");
	m_PositionsTex = Texture2D::Create("assets/textures/Positions.tga");
	m_NormalsTex = Texture2D::Create("assets/textures/Normals.tga");
	m_AlbedoTex = Texture2D::Create("assets/textures/Albedo.tga");
	m_SpecularTex = Texture2D::Create("assets/textures/Specular.tga");
	m_PointLightTex = Texture2D::Create("assets/textures/PointLight.tga");
	m_DirLightTex = Texture2D::Create("assets/textures/DirectionalLight.tga");

	m_ActiveScene = Ref<Scene>::Create();

	UpdateWindowTitle("Untitled Scene");

	m_SceneHierarchy = CreateScope<SceneHierarchy>(m_ActiveScene);

	m_SceneHierarchy->SetSelectionChangedCallback(std::bind(&EditorLayer::SelectEntity, this, std::placeholders::_1));
	m_SceneHierarchy->SetEntityDeletedCallback(std::bind(&EditorLayer::OnEntityDeleted, this, std::placeholders::_1));
}

void EditorLayer::OnDetach()
{
}

void EditorLayer::OnUpdate(Timestep ts)
{
	if (m_ViewportPanelFocused)
	{
		m_EditorCamera.OnUpdate(ts);
	}

	m_ActiveScene->OnUpdate(ts, m_EditorCamera);

	if (m_SelectionContext.size() && false)
	{
		auto& selection = m_SelectionContext[0];

		if (selection.Mesh && selection.Entity.HasComponent<MeshComponent>())
		{
			Renderer::BeginRenderPass(SceneRenderer::GetFinalRenderPass(), false);

			auto viewProj = m_EditorCamera.GetViewProjection();

			Renderer2D::BeginScene(viewProj, false);

			glm::vec4 colour = (m_SelectionMode == SelectionMode::Entity) ? glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f } : glm::vec4{ 0.2f, 0.9f, 0.2f, 1.0f };
			Renderer::DrawAABB(selection.Mesh->BoundingBox, selection.Entity.GetComponent<TransformComponent>().TransformComp * selection.Mesh->Transform, colour);

			Renderer2D::EndScene();

			Renderer::EndRenderPass();
		}
	}
}

void EditorLayer::OnImGuiRender()
{
	Window& window = Application::Get().GetWindow();

	static bool p_open = true;

	static bool opt_fullscreen_persistant = true;

	static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_NoCloseButton | ImGuiDockNodeFlags_NoWindowMenuButton;

	bool opt_fullscreen = opt_fullscreen_persistant;

	// ImGuiWindowFlags_NoDocking is used to ensure that the parent window isn't a dockable region
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

	if (opt_fullscreen)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();

		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("Docker", &p_open, window_flags);
	ImGui::PopStyleVar();

	if (opt_fullscreen)
	{
		ImGui::PopStyleVar(2);
	}

	// Dockspace
	ImGuiIO& io = ImGui::GetIO();

	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("Dockspace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), opt_flags);
	}

	#pragma region Editor Panel

	ImGui::Begin("Environment");

	ImGui::Columns(2);
	ImGui::AlignTextToFramePadding();

	auto& light = m_ActiveScene->GetDirectionalLight();

	Property("Light Direction", light.Direction, PropertyFlag::SliderProperty);
	Property("Light Brightness", light.Brightness, 0.0f, 0.5f, PropertyFlag::SliderProperty);
	Property("Light Diffuse", light.Colour, PropertyFlag::ColourProperty);
	Property("Light Ambient", light.Ambient, PropertyFlag::ColourProperty);

	Property("Exposure", m_EditorCamera.GetExposure(), 0.1f, 2.0f, PropertyFlag::SliderProperty);

	ImGui::End();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12, 4));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.8f, 0.8f, 0.2f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.8f, 0.8f, 0.5f));

	ImGui::Begin("Toolbar", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiDockNodeFlags_NoTabBar);

	// Bounding boxes
	if (!m_UIShowBoundingBoxes)
	{
		if (ImGui::ImageButton((ImTextureID)(m_BoundingBoxesTex->GetRendererID()), ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), ImVec4(1.0f, 1.0f, 1.0f, 0.25f)))
		{
			m_UIShowBoundingBoxes = true;

			ShowBoundingBoxes(m_UIShowBoundingBoxes);
		}
	}

	if (m_UIShowBoundingBoxes)
	{
		if (ImGui::ImageButton((ImTextureID)(m_BoundingBoxesTex->GetRendererID()), ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f)))
		{
			m_UIShowBoundingBoxes = false;

			ShowBoundingBoxes(m_UIShowBoundingBoxes);
		}
	}

	ImGui::SameLine();

	// Pointer
	if (ImGui::ImageButton((ImTextureID)(m_PointerTex->GetRendererID()), ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f)))
	{
		m_GizmoType = -1;
	}

	ImGui::SameLine();

	// Translate gizmo
	if (ImGui::ImageButton((ImTextureID)(m_TranslateTex->GetRendererID()), ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f)))
	{
		m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
	}

	ImGui::SameLine();

	// Rotate gizmo
	if (ImGui::ImageButton((ImTextureID)(m_RotateTex->GetRendererID()), ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f)))
	{
		m_GizmoType = ImGuizmo::OPERATION::ROTATE;
	}

	ImGui::SameLine();

	// Scale gizmo
	if (ImGui::ImageButton((ImTextureID)(m_ScaleTex->GetRendererID()), ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f)))
	{
		m_GizmoType = ImGuizmo::OPERATION::SCALE;
	}

	ImGui::SameLine();

	// Grid toggle
	if (SceneRenderer::GetOptions().ShowGrid)
	{
		if (ImGui::ImageButton((ImTextureID)(m_GridToggleTex->GetRendererID()), ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f)))
		{
			SceneRenderer::GetOptions().ShowGrid = false;
		}
	}

	if (!SceneRenderer::GetOptions().ShowGrid)
	{
		if (ImGui::ImageButton((ImTextureID)(m_GridToggleTex->GetRendererID()), ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), ImVec4(1.0f, 1.0f, 1.0f, 0.25f)))
		{
			SceneRenderer::GetOptions().ShowGrid = true;
		}
	}

	ImGui::SameLine();

	// Duplicate
	if (ImGui::ImageButton((ImTextureID)(m_DuplicateTex->GetRendererID()), ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f)))
	{
		if (m_SelectionContext.size())
		{
			Entity selectedEntity = m_SelectionContext[0].Entity;

			m_ActiveScene->DuplicateEntity(selectedEntity);
		}
	}

	ImGui::SameLine();
	
	// Show position
	if (SceneRenderer::GetOptions().ShowPosition)
	{
		if (ImGui::ImageButton((ImTextureID)(m_PositionsTex->GetRendererID()), ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f)))
		{
			SceneRenderer::GetOptions().ShowPosition = false;
		}
	}

	if (!SceneRenderer::GetOptions().ShowPosition)
	{
		if (ImGui::ImageButton((ImTextureID)(m_PositionsTex->GetRendererID()), ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), ImVec4(1.0f, 1.0f, 1.0f, 0.25f)))
		{
			SceneRenderer::GetOptions().ShowPosition = true;
		}
	}

	ImGui::SameLine();

	// Show normals
	if (SceneRenderer::GetOptions().ShowNormal)
	{
		if (ImGui::ImageButton((ImTextureID)(m_NormalsTex->GetRendererID()), ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f)))
		{
			SceneRenderer::GetOptions().ShowNormal = false;
		}
	}

	if (!SceneRenderer::GetOptions().ShowNormal)
	{
		if (ImGui::ImageButton((ImTextureID)(m_NormalsTex->GetRendererID()), ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), ImVec4(1.0f, 1.0f, 1.0f, 0.25f)))
		{
			SceneRenderer::GetOptions().ShowNormal = true;
		}
	}

	ImGui::SameLine();

	// Show albedo
	if (SceneRenderer::GetOptions().ShowAlbedo)
	{
		if (ImGui::ImageButton((ImTextureID)(m_AlbedoTex->GetRendererID()), ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f)))
		{
			SceneRenderer::GetOptions().ShowAlbedo = false;
		}
	}

	if (!SceneRenderer::GetOptions().ShowAlbedo)
	{
		if (ImGui::ImageButton((ImTextureID)(m_AlbedoTex->GetRendererID()), ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), ImVec4(1.0f, 1.0f, 1.0f, 0.25f)))
		{
			SceneRenderer::GetOptions().ShowAlbedo = true;
		}
	}

	ImGui::SameLine();
	
	// Show specular
	if (SceneRenderer::GetOptions().ShowSpecular)
	{
		if (ImGui::ImageButton((ImTextureID)(m_SpecularTex->GetRendererID()), ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), ImVec4(1.0f, 1.0f, 1.0f, 1.0f)))
		{
			SceneRenderer::GetOptions().ShowSpecular = false;
		}
	}

	if (!SceneRenderer::GetOptions().ShowSpecular)
	{
		if (ImGui::ImageButton((ImTextureID)(m_SpecularTex->GetRendererID()), ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), ImVec4(1.0f, 1.0f, 1.0f, 0.25f)))
		{
			SceneRenderer::GetOptions().ShowSpecular = true;
		}
	}

	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();

	#pragma endregion

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	#pragma region Viewport

	ImGui::Begin("Viewport", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiDockNodeFlags_NoTabBar);

	m_ViewportPanelMouseOver = ImGui::IsWindowHovered();

	// Set viewport window as focused on mouse hover
	if (m_ViewportPanelMouseOver)
	{
		ImGui::SetWindowFocus();
	}

	m_ViewportPanelFocused = ImGui::IsWindowFocused();

	// Offset includes the tab-bar
	auto viewportOffset = ImGui::GetCursorPos();
	auto viewportSize = ImGui::GetContentRegionAvail();

	SceneRenderer::SetViewportSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
	m_ActiveScene->SetViewportSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);

	m_EditorCamera.SetProjectionMatrix(glm::perspectiveFov(glm::radians(45.0f), viewportSize.x, viewportSize.y, 0.1f, 10000.0f));
	m_EditorCamera.SetViewportSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);

	ImGui::Image((void*)SceneRenderer::GetFinalColourBufferRendererID(), viewportSize, { 0, 1 }, { 1, 0 });

	static int counter = 0;

	auto windowSize = ImGui::GetWindowSize();

	ImVec2 minBound = ImGui::GetWindowPos();

	minBound.x += viewportOffset.x;
	minBound.y += viewportOffset.y;

	ImVec2 maxBound = { minBound.x + windowSize.x, minBound.y + windowSize.y };

	m_ViewportBounds[0] = { minBound.x, minBound.y };
	m_ViewportBounds[1] = { maxBound.x, maxBound.y };

	m_AllowViewportCameraEvents = ImGui::IsMouseHoveringRect(minBound, maxBound);

	// Gizmos
	if (m_GizmoType != -1 && m_SelectionContext.size())
	{
		auto& selection = m_SelectionContext[0];

		float rw = (float)ImGui::GetWindowWidth();
		float rh = (float)ImGui::GetWindowHeight();

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, rw, rh);

		bool snap = window.IsKeyPressed(LD_KEY_LEFT_CONTROL);

		auto& entityTransform = selection.Entity.Transform();

		float snapValue[3] = { m_SnapValue, m_SnapValue, m_SnapValue };

		if (m_SelectionMode == SelectionMode::Entity)
		{
			ImGuizmo::Manipulate(glm::value_ptr(m_EditorCamera.GetViewMatrix()), glm::value_ptr(m_EditorCamera.GetProjectionMatrix()), (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(entityTransform), nullptr,	snap ? snapValue : nullptr);
		}
		else
		{
			glm::mat4 transformBase = entityTransform * selection.Mesh->Transform;

			ImGuizmo::Manipulate(glm::value_ptr(m_EditorCamera.GetViewMatrix()), glm::value_ptr(m_EditorCamera.GetProjectionMatrix()), (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transformBase), nullptr, snap ? snapValue : nullptr);

			selection.Mesh->Transform = glm::inverse(entityTransform) * transformBase;
		}
	}

	ImGui::End();
	ImGui::PopStyleVar();

	#pragma endregion

	#pragma region Menubar

	// Menu bar
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open", "", false))
			{
				auto& app = Application::Get();
				std::string filepath = app.OpenFile("Lucid Scene (*.lcd)\0*.lcd\0");

				if (!filepath.empty())
				{
					Ref<Scene> newScene = Ref<Scene>::Create();

					SceneSerializer serializer(newScene);
					serializer.Deserialize(filepath);

					m_ActiveScene = newScene;
					std::filesystem::path path = filepath;
					UpdateWindowTitle(path.filename().string());

					m_SceneHierarchy->SetContext(m_ActiveScene);

					m_ActiveScene->SetSelectedEntity({});
					m_SelectionContext.clear();
				}
			}

			if (ImGui::MenuItem("Save", "", false))
			{
				auto& app = Application::Get();

				std::string filepath = app.SaveFile("Lucid Scene (*.lcd)\0*.lcd\0");

				SceneSerializer serializer(m_ActiveScene);
				serializer.Serialize(filepath);

				std::filesystem::path path = filepath;
				UpdateWindowTitle(path.filename().string());
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	m_SceneHierarchy->OnImGuiRender();

	#pragma endregion

	ImGui::End();
}

void EditorLayer::OnEvent(Event& e)
{
	if (m_ViewportPanelMouseOver)
	{
		m_EditorCamera.OnEvent(e);
	}

	m_ActiveScene->OnEvent(e);

	EventDispatcher dispatcher(e);
	dispatcher.Dispatch<KeyPressedEvent>(LD_BIND_EVENT_FN(EditorLayer::OnKeyPressedEvent));
	dispatcher.Dispatch<MouseButtonPressedEvent>(LD_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
}

bool EditorLayer::OnKeyPressedEvent(KeyPressedEvent& e)
{
	Window& window = Application::Get().GetWindow();

	if (m_ViewportPanelFocused)
	{
		switch (e.GetKeyCode())
		{
			case LD_KEY_Q:
			{
				m_GizmoType = -1;

				break;
			}
			case LD_KEY_W:
			{
				m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;

				break;
			}
			case LD_KEY_E:
			{
				m_GizmoType = ImGuizmo::OPERATION::ROTATE;

				break;
			}
			case LD_KEY_R:
			{
				m_GizmoType = ImGuizmo::OPERATION::SCALE;

				break;
			}
			case LD_KEY_DELETE:
			{
				if (m_SelectionContext.size())
				{
					Entity selectedEntity = m_SelectionContext[0].Entity;

					m_ActiveScene->DestroyEntity(selectedEntity);
					m_SelectionContext.clear();

					m_ActiveScene->SetSelectedEntity({});
					m_SceneHierarchy->SetSelected({});
				}
			}
		}
	}

	if (window.IsKeyPressed(LD_KEY_LEFT_CONTROL))
	{
		switch (e.GetKeyCode())
		{
			case LD_KEY_G:
			{
				// Toggle grid
				if (window.IsKeyPressed(LD_KEY_LEFT_CONTROL))
				{
					SceneRenderer::GetOptions().ShowGrid = !SceneRenderer::GetOptions().ShowGrid;
				}

				break;
			}
			case LD_KEY_B:
			{
				// Toggle bounding boxes 
				if (window.IsKeyPressed(LD_KEY_LEFT_CONTROL))
				{
					m_UIShowBoundingBoxes = !m_UIShowBoundingBoxes;

					ShowBoundingBoxes(m_UIShowBoundingBoxes);
				}

				break;
			}
			case LD_KEY_D:
			{
				if (m_SelectionContext.size())
				{
					Entity selectedEntity = m_SelectionContext[0].Entity;

					m_ActiveScene->DuplicateEntity(selectedEntity);
				}

				break;
			}
		}
	}

	return false;
}

bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
{
	Window& window = Application::Get().GetWindow();

	auto [mx, my] = window.GetMousePosition();

	if (e.GetMouseButton() == LD_MOUSE_BUTTON_LEFT && !window.IsKeyPressed(LD_KEY_LEFT_ALT) && !ImGuizmo::IsOver())
	{
		auto [mouseX, mouseY] = GetMouseViewportSpace();

		if (mouseX > -1.0f && mouseX < 1.0f && mouseY > -1.0f && mouseY < 1.0f)
		{
			auto [origin, direction] = CastRay(mouseX, mouseY);

			m_SelectionContext.clear();
			m_ActiveScene->SetSelectedEntity({});

			auto meshEntities = m_ActiveScene->GetAllEntitiesWith<MeshComponent>();

			for (auto e : meshEntities)
			{
				Entity entity = { e, m_ActiveScene.Raw() };

				auto mesh = entity.GetComponent<MeshComponent>().MeshComp;

				if (!mesh)
				{
					continue;
				}

				auto& submeshes = mesh->GetSubmeshes();

				float lastT = std::numeric_limits<float>::max();

				for (uint32_t i = 0; i < submeshes.size(); i++)
				{
					auto& submesh = submeshes[i];

					Ray ray = {
						glm::inverse(entity.Transform() * submesh.Transform) * glm::vec4(origin, 1.0f),
						glm::inverse(glm::mat3(entity.Transform()) * glm::mat3(submesh.Transform)) * direction
					};

					float t;

					bool intersects = ray.IntersectsAABB(submesh.BoundingBox, t);

					if (intersects)
					{
						const auto& triangleCache = mesh->GetTriangleCache(i);

						for (const auto& triangle : triangleCache)
						{
							if (ray.IntersectsTriangle(triangle.V0.Position, triangle.V1.Position, triangle.V2.Position, t))
							{
								LD_CORE_WARN("INTERSECTION: {0}, t={1}", submesh.NodeName, t);

								m_SelectionContext.push_back({ entity, &submesh, t });

								break;
							}
						}
					}
				}
			}

			std::sort(m_SelectionContext.begin(), m_SelectionContext.end(), [](auto& a, auto& b) { return a.Distance < b.Distance; });

			if (m_SelectionContext.size())
			{
				OnSelected(m_SelectionContext[0]);
			}
		}
	}

	return false;
}

void EditorLayer::ShowBoundingBoxes(bool show)
{
	SceneRenderer::GetOptions().ShowBoundingBoxes = show;
}

std::pair<float, float> EditorLayer::GetMouseViewportSpace()
{
	auto [mx, my] = ImGui::GetMousePos();

	mx -= m_ViewportBounds[0].x;
	my -= m_ViewportBounds[0].y;

	auto viewportWidth = m_ViewportBounds[1].x - m_ViewportBounds[0].x;
	auto viewportHeight = m_ViewportBounds[1].y - m_ViewportBounds[0].y;

	return { (mx / viewportWidth) * 2.0f - 1.0f, ((my / viewportHeight) * 2.0f - 1.0f) * -1.0f };
}

void EditorLayer::SelectEntity(Entity entity)
{
	SelectedSubmesh selection;

	if (entity.HasComponent<MeshComponent>())
	{
		selection.Mesh = &entity.GetComponent<MeshComponent>().MeshComp->GetSubmeshes()[0];
	}

	selection.Entity = entity;

	m_SelectionContext.clear();
	m_SelectionContext.push_back(selection);

	m_ActiveScene->SetSelectedEntity(entity);
}

void EditorLayer::OnSelected(const SelectedSubmesh& selectionContext)
{
	m_SceneHierarchy->SetSelected(selectionContext.Entity);
	m_ActiveScene->SetSelectedEntity(selectionContext.Entity);
}

void EditorLayer::OnEntityDeleted(Entity e)
{
	if (m_SelectionContext[0].Entity == e)
	{
		m_SelectionContext.clear();
		m_ActiveScene->SetSelectedEntity({});
	}
}

std::pair<glm::vec3, glm::vec3> EditorLayer::CastRay(float mx, float my)
{
	glm::vec4 mouseClipPos = { mx, my, -1.0f, 1.0f };

	auto inverseProj = glm::inverse(m_EditorCamera.GetProjectionMatrix());
	auto inverseView = glm::inverse(glm::mat3(m_EditorCamera.GetViewMatrix()));

	glm::vec4 ray = inverseProj * mouseClipPos;
	glm::vec3 rayPos = m_EditorCamera.GetPosition();
	glm::vec3 rayDir = inverseView * glm::vec3(ray);

	return { rayPos, rayDir };
}

Ray EditorLayer::CastMouseRay()
{
	auto [mouseX, mouseY] = GetMouseViewportSpace();

	if (mouseX > -1.0f && mouseX < 1.0f && mouseY > -1.0f && mouseY < 1.0f)
	{
		auto [origin, direction] = CastRay(mouseX, mouseY);

		return Ray(origin, direction);
	}

	return Ray::Zero();
}

void EditorLayer::UpdateWindowTitle(const std::string& sceneName)
{
	std::string title = sceneName + " - Lucid Engine";
	Application::Get().GetWindow().SetTitle(title);
}