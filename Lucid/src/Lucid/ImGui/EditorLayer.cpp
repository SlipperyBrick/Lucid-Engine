#include "ldpch.h"

#include "EditorLayer.h"

#include "Lucid/ImGui/ImGuiGizmo.h"

#include "Lucid/Core/Application.h"

#include "Lucid/Renderer/Renderer2D.h"
#include "Lucid/Renderer/SceneRenderer.h"

#include "Lucid/Core/Math/Ray.h"

EditorLayer::EditorLayer()
{
}

EditorLayer::EditorLayer(const std::string& name)
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

	// Load our first entity
	{
		m_Scene = Ref<Scene>::Create("Scene");
		m_Scene->SetCamera(Camera(glm::perspectiveFov(glm::radians(45.0f), 1280.0f, 720.0f, 0.1f, 10000.0f)));

		m_MeshEntity = m_Scene->CreateEntity("Sponza");

		auto mesh = Ref<Mesh>::Create("assets/models/flintlock/pistol.obj");
		m_MeshEntity->SetMesh(mesh);

		m_MeshMaterial = mesh->GetMaterial();
	}

	m_ActiveScene = m_Scene;
	m_SceneHierarchy = CreateScope<SceneHierarchy>(m_ActiveScene);

	// Editor default texture
	m_CheckerboardTex = Texture2D::Create("assets/textures/Checkerboard.tga");

	// Set scenes lights
	auto& light = m_Scene->GetLight();
	light.Position = { 0.0f, 5.0f, 0.0f };
	light.Radiance = { 1.0f, 1.0f, 1.0f };

	m_CurrentlySelectedTransform = &m_MeshEntity->Transform();
}

void EditorLayer::OnDetach()
{
}

void EditorLayer::OnUpdate(Timestep ts)
{
	m_MeshMaterial->Set("u_DiffuseColour", m_DiffuseInput.Colour);
	m_MeshMaterial->Set("u_Specular", m_SpecularInput.Value);
	m_MeshMaterial->Set("u_DiffuseTexToggle", m_DiffuseInput.UseTexture ? 1.0f : 0.0f);
	m_MeshMaterial->Set("u_NormalTexToggle", m_NormalInput.UseTexture ? 1.0f : 0.0f);
	m_MeshMaterial->Set("u_SpecularTexToggle", m_SpecularInput.UseTexture ? 1.0f : 0.0f);

	m_MeshMaterial->Set("lights", m_Scene->GetLight());

	if (m_DiffuseInput.TextureMap)
	{
		m_MeshMaterial->Set("u_DiffuseTexture", m_DiffuseInput.TextureMap);
	}

	if (m_NormalInput.TextureMap)
	{
		m_MeshMaterial->Set("u_NormalTexture", m_NormalInput.TextureMap);
	}

	if (m_SpecularInput.TextureMap)
	{
		m_MeshMaterial->Set("u_SpecularTexture", m_SpecularInput.TextureMap);
	}

	if (m_AllowViewportCameraEvents)
	{
		m_Scene->GetCamera().OnUpdate(ts);
	}

	m_ActiveScene->OnUpdate(ts);

	if (m_DrawOnTopBoundingBoxes)
	{
		Renderer::BeginRenderPass(SceneRenderer::GetFinalRenderPass(), false);

		auto viewProj = m_Scene->GetCamera().GetViewProjection();

		Renderer2D::BeginScene(viewProj, false);

		Renderer::DrawAABB(m_MeshEntity->GetMesh(), m_MeshEntity->Transform());

		Renderer2D::EndScene();

		Renderer::EndRenderPass();
	}

	if (m_SelectedSubmeshes.size())
	{
		Renderer::BeginRenderPass(SceneRenderer::GetFinalRenderPass(), false);

		auto viewProj = m_Scene->GetCamera().GetViewProjection();

		Renderer2D::BeginScene(viewProj, false);

		auto& submesh = m_SelectedSubmeshes[0];
		Renderer::DrawAABB(submesh.Mesh->BoundingBox, m_MeshEntity->GetTransform() * submesh.Mesh->Transform);

		Renderer2D::EndScene();

		Renderer::EndRenderPass();
	}
}

void EditorLayer::OnImGuiRender()
{
	static bool p_open = true;

	static bool opt_fullscreen_persistant = true;
	static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_None;
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

	ImGui::Begin("Model");

	ImGui::Begin("Environment");

	ImGui::Columns(2);
	ImGui::AlignTextToFramePadding();

	auto& light = m_Scene->GetLight();
	Property("Light Position", light.Position);
	Property("Light Radiance", light.Radiance, PropertyFlag::ColourProperty);
	Property("Light Multiplier", light.Multiplier, 0.0f, 5.0f);

	if (Property("Show Bounding Boxes", m_UIShowBoundingBoxes))
	{
		ShowBoundingBoxes(m_UIShowBoundingBoxes, m_UIShowBoundingBoxesOnTop);
	}

	if (m_UIShowBoundingBoxes && Property("On Top", m_UIShowBoundingBoxesOnTop))
	{
		ShowBoundingBoxes(m_UIShowBoundingBoxes, m_UIShowBoundingBoxesOnTop);
	}

	ImGui::Columns(1);

	ImGui::End();

	ImGui::Separator();

	{
		ImGui::Text("Mesh");

		auto mesh = m_MeshEntity->GetMesh();

		std::string fullpath = mesh ? mesh->GetFilePath() : "None";
		size_t found = fullpath.find_last_of("/\\");
		std::string path = found != std::string::npos ? fullpath.substr(found + 1) : fullpath;

		ImGui::Text(path.c_str()); ImGui::SameLine();

		if (ImGui::Button("...##Mesh"))
		{
			std::string filename = Application::Get().OpenFile("");

			if (filename != "")
			{
				auto newMesh = Ref<Mesh>::Create(filename);
				m_MeshEntity->SetMesh(newMesh);
			}
		}
	}

	ImGui::Separator();

	#pragma region Texture Slots

	{
		// Diffuse
		if (ImGui::CollapsingHeader("Diffuse", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));

			ImGui::Image(m_DiffuseInput.TextureMap ? (void*)m_DiffuseInput.TextureMap->GetRendererID() : (void*)m_CheckerboardTex->GetRendererID(), ImVec2(64, 64));

			ImGui::PopStyleVar();

			if (ImGui::IsItemHovered())
			{
				if (m_DiffuseInput.TextureMap)
				{
					ImGui::BeginTooltip();
					ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
					ImGui::TextUnformatted(m_DiffuseInput.TextureMap->GetPath().c_str());
					ImGui::PopTextWrapPos();
					ImGui::Image((void*)m_DiffuseInput.TextureMap->GetRendererID(), ImVec2(384, 384));
					ImGui::EndTooltip();
				}

				if (ImGui::IsItemClicked())
				{
					std::string filename = Application::Get().OpenFile("");

					if (filename != "")
					{
						m_DiffuseInput.TextureMap = Texture2D::Create(filename, m_DiffuseInput.SRGB);
					}
				}
			}

			ImGui::SameLine();

			ImGui::BeginGroup();

			ImGui::Checkbox("Use##DiffuseMap", &m_DiffuseInput.UseTexture);

			if (ImGui::Checkbox("sRGB##DiffuseMap", &m_DiffuseInput.SRGB))
			{
				if (m_DiffuseInput.TextureMap)
				{
					m_DiffuseInput.TextureMap = Texture2D::Create(m_DiffuseInput.TextureMap->GetPath(), m_DiffuseInput.SRGB);
				}
			}

			ImGui::EndGroup();

			ImGui::SameLine();

			ImGui::ColorEdit3("Colour##Diffuse", glm::value_ptr(m_DiffuseInput.Colour), ImGuiColorEditFlags_NoInputs);
		}
	}
	{
		// Normals
		if (ImGui::CollapsingHeader("Normals", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));

			ImGui::Image(m_NormalInput.TextureMap ? (void*)m_NormalInput.TextureMap->GetRendererID() : (void*)m_CheckerboardTex->GetRendererID(), ImVec2(64, 64));

			ImGui::PopStyleVar();

			if (ImGui::IsItemHovered())
			{
				if (m_NormalInput.TextureMap)
				{
					ImGui::BeginTooltip();
					ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
					ImGui::TextUnformatted(m_NormalInput.TextureMap->GetPath().c_str());
					ImGui::PopTextWrapPos();
					ImGui::Image((void*)m_NormalInput.TextureMap->GetRendererID(), ImVec2(384, 384));
					ImGui::EndTooltip();
				}

				if (ImGui::IsItemClicked())
				{
					std::string filename = Application::Get().OpenFile("");

					if (filename != "")
					{
						m_NormalInput.TextureMap = Texture2D::Create(filename);
					}
				}
			}

			ImGui::SameLine();

			ImGui::Checkbox("Use##NormalMap", &m_NormalInput.UseTexture);
		}
	}
	{
		// Specular
		if (ImGui::CollapsingHeader("Specular", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));

			ImGui::Image(m_SpecularInput.TextureMap ? (void*)m_SpecularInput.TextureMap->GetRendererID() : (void*)m_CheckerboardTex->GetRendererID(), ImVec2(64, 64));

			ImGui::PopStyleVar();

			if (ImGui::IsItemHovered())
			{
				if (m_SpecularInput.TextureMap)
				{
					ImGui::BeginTooltip();
					ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
					ImGui::TextUnformatted(m_SpecularInput.TextureMap->GetPath().c_str());
					ImGui::PopTextWrapPos();
					ImGui::Image((void*)m_SpecularInput.TextureMap->GetRendererID(), ImVec2(384, 384));
					ImGui::EndTooltip();
				}

				if (ImGui::IsItemClicked())
				{
					std::string filename = Application::Get().OpenFile("");

					if (filename != "")
					{
						m_SpecularInput.TextureMap = Texture2D::Create(filename);
					}
				}
			}

			ImGui::SameLine();

			ImGui::Checkbox("Use##SpecularMap", &m_SpecularInput.UseTexture);

			ImGui::SameLine();

			ImGui::SliderFloat("Value##SpecularInput", &m_SpecularInput.Value, 0.0f, 1.0f);
		}
	}

	#pragma endregion

	ImGui::Separator();

	if (ImGui::TreeNode("Shaders"))
	{
		auto& shaders = Shader::s_AllShaders;

		for (auto& shader : shaders)
		{
			if (ImGui::TreeNode(shader->GetName().c_str()))
			{
				std::string buttonName = "Reload##" + shader->GetName();

				if (ImGui::Button(buttonName.c_str()))
				{
					shader->Reload();
				}

				ImGui::TreePop();
			}
		}

		ImGui::TreePop();
	}

	#pragma endregion

	ImGui::End();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	#pragma region Viewport

	ImGui::Begin("Viewport", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	// Offset includes the tab-bar
	auto viewportOffset = ImGui::GetCursorPos();
	auto viewportSize = ImGui::GetContentRegionAvail();

	SceneRenderer::SetViewportSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);

	m_ActiveScene->GetCamera().SetProjectionMatrix(glm::perspectiveFov(glm::radians(45.0f), viewportSize.x, viewportSize.y, 0.1f, 10000.0f));
	m_ActiveScene->GetCamera().SetViewportSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);

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
	if (m_GizmoType != -1 && m_CurrentlySelectedTransform)
	{
		float rw = (float)ImGui::GetWindowWidth();
		float rh = (float)ImGui::GetWindowHeight();

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, rw, rh);

		Window& window = Application::Get().GetWindow();

		bool snap = window.IsKeyPressed(LD_KEY_LEFT_CONTROL);

		ImGuizmo::Manipulate(glm::value_ptr(m_ActiveScene->GetCamera().GetViewMatrix() * m_MeshEntity->Transform()), glm::value_ptr(m_ActiveScene->GetCamera().GetProjectionMatrix()), (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(*m_CurrentlySelectedTransform), nullptr, snap ? &m_SnapValue : nullptr);
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
			if (ImGui::MenuItem("New", "", false))
			{
				
			}

			if (ImGui::MenuItem("Open", "", false))
			{

			}

			if (ImGui::MenuItem("Save", "", false))
			{

			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Copy", "", false))
			{

			}

			if (ImGui::MenuItem("Paste", "", false))
			{

			}

			if (ImGui::MenuItem("Delete", "", false))
			{

			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Renderer Info", "", false))
			{

			}

			if (ImGui::MenuItem("Grid", "", false))
			{

			}

			if (ImGui::MenuItem("Bounding Boxes", "", false))
			{

			}

			if (ImGui::MenuItem("Scene Hierarchy", "", false))
			{

			}

			if (ImGui::MenuItem("Properties", "", false))
			{

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
	if (m_AllowViewportCameraEvents)
	{
		m_Scene->GetCamera().OnEvent(e);
	}

	EventDispatcher dispatcher(e);
	dispatcher.Dispatch<KeyPressedEvent>(LD_BIND_EVENT_FN(EditorLayer::OnKeyPressedEvent));
	dispatcher.Dispatch<MouseButtonPressedEvent>(LD_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
}

bool EditorLayer::OnKeyPressedEvent(KeyPressedEvent& e)
{
	Window& window = Application::Get().GetWindow();

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

				ShowBoundingBoxes(m_UIShowBoundingBoxes, m_UIShowBoundingBoxesOnTop);
			}

			break;
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

			m_SelectedSubmeshes.clear();

			auto mesh = m_MeshEntity->GetMesh();
			auto& submeshes = mesh->GetSubmeshes();
			float lastT = std::numeric_limits<float>::max();

			for (uint32_t i = 0; i < submeshes.size(); i++)
			{
				auto& submesh = submeshes[i];

				Ray ray =
				{
					glm::inverse(m_MeshEntity->GetTransform() * submesh.Transform) * glm::vec4(origin, 1.0f),
					glm::inverse(glm::mat3(m_MeshEntity->GetTransform()) * glm::mat3(submesh.Transform)) * direction
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

							m_SelectedSubmeshes.push_back({ &submesh, t });

							break;
						}
					}
				}
			}

			std::sort(m_SelectedSubmeshes.begin(), m_SelectedSubmeshes.end(), [](auto& a, auto& b) { return a.Distance < b.Distance; });

			// TODO: Handle mesh being deleted, etc.
			if (m_SelectedSubmeshes.size())
			{
				m_CurrentlySelectedTransform = &m_SelectedSubmeshes[0].Mesh->Transform;
			}
			else
			{
				m_CurrentlySelectedTransform = &m_MeshEntity->Transform();
			}
		}
	}

	return false;
}

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

void EditorLayer::Property(const std::string& name, float& value, float min, float max, PropertyFlag flags)
{
	ImGui::Text(name.c_str());
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);

	std::string id = "##" + name;
	ImGui::SliderFloat(id.c_str(), &value, min, max);

	ImGui::PopItemWidth();
	ImGui::NextColumn();
}

void EditorLayer::Property(const std::string& name, glm::vec2& value, PropertyFlag flags)
{
	Property(name, value, -1.0f, 1.0f, flags);
}

void EditorLayer::Property(const std::string& name, glm::vec2& value, float min, float max, PropertyFlag flags)
{
	ImGui::Text(name.c_str());
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);

	std::string id = "##" + name;
	ImGui::SliderFloat2(id.c_str(), glm::value_ptr(value), min, max);

	ImGui::PopItemWidth();
	ImGui::NextColumn();
}

void EditorLayer::Property(const std::string& name, glm::vec3& value, PropertyFlag flags)
{
	Property(name, value, -1.0f, 1.0f, flags);
}

void EditorLayer::Property(const std::string& name, glm::vec3& value, float min, float max, PropertyFlag flags)
{
	ImGui::Text(name.c_str());
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);

	std::string id = "##" + name;

	if ((int)flags & (int)PropertyFlag::ColourProperty)
	{
		ImGui::ColorEdit3(id.c_str(), glm::value_ptr(value), ImGuiColorEditFlags_NoInputs);
	}
	else
	{
		ImGui::SliderFloat3(id.c_str(), glm::value_ptr(value), min, max);
	}

	ImGui::PopItemWidth();
	ImGui::NextColumn();
}

void EditorLayer::Property(const std::string& name, glm::vec4& value, PropertyFlag flags)
{
	Property(name, value, -1.0f, 1.0f, flags);
}

void EditorLayer::Property(const std::string& name, glm::vec4& value, float min, float max, PropertyFlag flags)
{
	ImGui::Text(name.c_str());
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);

	std::string id = "##" + name;

	if ((int)flags & (int)PropertyFlag::ColourProperty)
	{
		ImGui::ColorEdit4(id.c_str(), glm::value_ptr(value), ImGuiColorEditFlags_NoInputs);
	}
	else
	{
		ImGui::SliderFloat4(id.c_str(), glm::value_ptr(value), min, max);
	}

	ImGui::PopItemWidth();
	ImGui::NextColumn();
}

void EditorLayer::ShowBoundingBoxes(bool show, bool onTop)
{
	SceneRenderer::GetOptions().ShowBoundingBoxes = show && !onTop;

	m_DrawOnTopBoundingBoxes = show && onTop;
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

std::pair<glm::vec3, glm::vec3> EditorLayer::CastRay(float mx, float my)
{
	glm::vec4 mouseClipPos = { mx, my, -1.0f, 1.0f };

	auto inverseProj = glm::inverse(m_Scene->GetCamera().GetProjectionMatrix());
	auto inverseView = glm::inverse(glm::mat3(m_Scene->GetCamera().GetViewMatrix()));

	glm::vec4 ray = inverseProj * mouseClipPos;
	glm::vec3 rayPos = m_Scene->GetCamera().GetPosition();
	glm::vec3 rayDir = inverseView * glm::vec3(ray);

	return { rayPos, rayDir };
}