#include "ldpch.h"

#include <glfw/glfw3.h>

#include <imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "ImGuiLayer.h"

#include "Lucid/ImGui/ImGuiGizmo.h"

#include "Lucid/Core/Application.h"

ImGuiLayer::ImGuiLayer()
{
}

ImGuiLayer::ImGuiLayer(const std::string& name)
{
}

ImGuiLayer::~ImGuiLayer()
{
}

void ImGuiLayer::OnAttach()
{

	// Setup ImGui rendering context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();

	// Enable keyboard controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	// Enable docking
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// Enable multi-viewport/windows
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	// Set ImGui interface font
	ImFont* pFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
	io.FontDefault = io.Fonts->Fonts.back();

	// Set ImGui theme/style
	ImGui::StyleColorsDark();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, style.Colors[ImGuiCol_WindowBg].w);

	// Get application instance and retrieve GLFW window
	Application& app = Application::Get();
	GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetWindowPointer());

	// Setup platform/renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 410");
}

void ImGuiLayer::OnDetach()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiLayer::Begin()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
}

void ImGuiLayer::End()
{
	ImGuiIO& io = ImGui::GetIO();

	// Get application instance
	Application& app = Application::Get();

	// Set display size of applications window
	io.DisplaySize = ImVec2(app.GetWindow().GetWidth(), app.GetWindow().GetHeight());

	// Render ImGui interface
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}
}

void ImGuiLayer::OnImGuiRender()
{
}