#include "ldpch.h"

#include <glad/glad.h>

#include <imgui/imgui.h>

#include <glfw/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>

#include "Application.h"

#include "Lucid/Renderer/Renderer.h"
#include "Lucid/Renderer/Framebuffer.h"

// Application instance
Application* Application::s_Instance = nullptr;

// Creates an application with desired application properties, initalizes core engine components and sets up window, event callbacks and renderer
Application::Application(const ApplicationProps& props)
{
	// Set application instance to newly created application
	s_Instance = this;

	// Initalize core engine components, such as logging system
	InitializeCore();

	// Create the applications window, set event callbacks and enable v-sync
	m_Window = std::unique_ptr<Window>(Window::Create(WindowProps(props.Name, props.WindowWidth, props.WindowHeight)));
	m_Window->SetEventCallback(LD_BIND_EVENT_FN(Application::OnEvent));
	m_Window->SetVSync(true);

	// Push a new ImGui layer to applications layer stack, needed for inital ImGui setup (which is done when ImGuiLayers OnAttach() method is called) 
	m_ImGuiLayer = new ImGuiLayer("Application Layer");
	PushOverlay(m_ImGuiLayer);

	// Initalize renderer and traverse render command queue for processing any renderer commands
	Renderer::Init();
	Renderer::ExecuteRenderCommands();
}

// Destroys applications window, shuts down core engine components and releases all memory pertaining to the application
Application::~Application()
{
	// Destroy window/GLFW and release from memory
	m_Window.reset();

	// Shutdown engine
	ShutdownCore();
}

// Pushes a new layer to the applications layer stack
void Application::PushLayer(Layer* layer)
{
	m_LayerStack.PushLayer(layer);
	layer->OnAttach();
}

// Pushes a new overlay to the applications layer stack
void Application::PushOverlay(Layer* layer)
{
	m_LayerStack.PushOverlay(layer);
	layer->OnAttach();
}

// Render the ImGui user-interface
void Application::RenderImGui()
{
	m_ImGuiLayer->Begin();

	ImGui::Begin("Renderer", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

	auto& caps = RendererCapabilities::GetCapabilities();

	ImGui::Text("Vendor: %s", caps.Vendor.c_str());
	ImGui::Text("Renderer: %s", caps.Renderer.c_str());
	ImGui::Text("Version: %s", caps.Version.c_str());
	ImGui::Text("Frame Time: %.2fms\n", m_TimeStep.GetMilliseconds());

	ImGui::End();

	for (Layer* layer : m_LayerStack)
	{
		layer->OnImGuiRender();
	}

	m_ImGuiLayer->End();
}

std::string Application::OpenFile(const std::string& filter) const
{
	// Common dialog box structure
	OPENFILENAMEA ofn;
	CHAR szFile[260] = { 0 };

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)m_Window.get()->GetWindowPointer());
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(&ofn) == TRUE)
	{
		return ofn.lpstrFile;
	}

	return std::string();
}

// Initalizes application specific components such as user-interface
void Application::OnInit()
{
	PushLayer(new EditorLayer("Editor Layer"));
}

// Applications run loop that processes all application logic
void Application::Run()
{
	OnInit();

	while (m_Running)
	{
		if (!m_Minimized)
		{
			// Update all the applications layers in the layer stack
			for (Layer* layer : m_LayerStack)
			{
				layer->OnUpdate(m_TimeStep);
			}

			// Instance of application for lambda capture list
			Application* app = this;

			// Render the applications user-interface on render thread
			Renderer::Submit([app]() { app->RenderImGui(); });

			// Traverse the render command queue to process any render commands
			Renderer::ExecuteRenderCommands();
		}

		m_Window->OnUpdate();

		// Calcuate the applications timestep
		float time = GetTime();
		m_TimeStep = time - m_LastFrameTime;
		m_LastFrameTime = time;
	}

	// Handle shutdown here
}

// Dispatch and process applications window events
void Application::OnEvent(Event& event)
{
	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<WindowResizeEvent>(LD_BIND_EVENT_FN(Application::OnWindowResize));
	dispatcher.Dispatch<WindowCloseEvent>(LD_BIND_EVENT_FN(Application::OnWindowClose));

	// Traverse the applications layer stack to handle events for the ImGui user-interface
	for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
	{
		(*--it)->OnEvent(event);

		// If the event is already handled, break out of the loop
		if (event.Handled)
		{
			break;
		}
	}
}

// Resize the applications window, also resize the viewport and all framebuffers
bool Application::OnWindowResize(WindowResizeEvent& e)
{
	// Log the event to console
	LD_CORE_DEBUG("Event dispatched successfully: {0}", e.ToString());

	// New width and height of application window
	int width = e.GetWidth();
	int height = e.GetHeight();

	// If the width and height are zero, application window is minimized
	if (width == 0 || height == 0)
	{
		m_Minimized = true;

		return false;
	}

	m_Minimized = false;

	Renderer::Submit([=]() { glViewport(0, 0, width, height); });

	// Retrieve all framebuffer objects
	auto& fbs = FramebufferPool::GetGlobal()->GetAll();

	// Resize all framebuffers
	for (auto& fb : fbs)
	{
		fb->Resize(width, height);
	}

	return false;
}

// Close the application
bool Application::OnWindowClose(WindowCloseEvent& e)
{
	LD_CORE_DEBUG("Event dispatched successfully: {0}", e.ToString());

	m_Running = false;

	return true;
}

// Get the applications current timestep
float Application::GetTime() const
{
	return (float)glfwGetTime();
}