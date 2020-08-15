#pragma once

#include "Lucid/Core/Timestep.h"
#include "Lucid/Core/Window.h"
#include "Lucid/Core/LayerStack.h"

#include "Lucid/Core/Events/ApplicationEvent.h"

#include "Lucid/ImGui/ImGuiLayer.h"
#include "Lucid/ImGui/EditorLayer.h"

#include "Lucid/Renderer/Camera.h"

struct ApplicationProps
{
	std::string Name;
	uint32_t WindowWidth;
	uint32_t WindowHeight;
};

// Handles applications layer stack, on update and event data and the run loop
class Application
{

public:

	Application(const ApplicationProps& props = { "Lucid Engine", 1280, 720 });
	~Application();

	void OnInit();
	void Run();

	void OnUpdate(Timestep ts) {}

	void OnEvent(Event& event);

	void PushLayer(Layer* layer);
	void PushOverlay(Layer* layer);
	void RenderImGui();

	std::string OpenFile(const char* filter = "All\0*.*\0") const;
	std::string SaveFile(const char* filter = "All\0*.*\0") const;

	// Get the applications window
	inline Window& GetWindow() { return *m_Window; }

	// Get instance of the application
	static inline Application& Get() { return *s_Instance; }

	float GetTime() const;

private:

	bool OnWindowResize(WindowResizeEvent& e);
	bool OnWindowClose(WindowCloseEvent& e);

private:

	std::unique_ptr<Window> m_Window;

	bool m_Running = true;
	bool m_Minimized = false;

	LayerStack m_LayerStack;
	ImGuiLayer* m_ImGuiLayer;

	Timestep m_TimeStep;

	float m_LastFrameTime = 0.0f;

	static Application* s_Instance;
};