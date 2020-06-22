#include "ldpch.h"

#include "Application.h"

#include "Lucid/Renderer/Renderer.h"
//#include "Lucid/Renderer/Framebuffer.h"

#include <imgui/imgui.h>

// Application instance
Application* Application::s_Instance = nullptr;

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
	m_ImGuiLayer = new ImGuiLayer();
	PushOverlay(m_ImGuiLayer);

	// Initalize renderer and traverse render command queue for processing any renderer commands
	//Renderer::Init();
	//Renderer::WaitAndRender();
}

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

	for (Layer* layer : m_LayerStack)
	{
		layer->OnImGuiRender();
	}

	m_ImGuiLayer->End();
}

// Applications run loop that processes all application logic
void Application::Run()
{
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

			// Render the applications ImGui user-interface on render thread
			//Renderer::Submit([app]() { app->RenderImGui(); });

			// Traverse the render command queue to process any render commands
			//Renderer::WaitAndRender();
		}

		m_Window->OnUpdate();

		// Calcuate the applications timestep
		float time = GetTime();
		m_TimeStep = time - m_LastFrameTime;
		m_LastFrameTime = time;
	}
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

	//auto& fbs = FramebufferPool::GetGlobal()->GetAll();

	// Resize all framebuffers
	/*for (auto& fb : fbs)
	{
		if (auto fbp = fb.lock())
		{
			fbp->Resize(width, height);
		}
	}*/

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