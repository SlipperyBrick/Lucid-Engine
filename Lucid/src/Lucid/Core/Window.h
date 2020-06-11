#pragma once

#include <string>
#include <functional>

#include <glfw/glfw3.h>

#include "Events/Event.h"

struct WindowProps
{
	std::string Title;
	unsigned int Width;
	unsigned int Height;

	WindowProps(const std::string& title = "Lucid Engine", unsigned int width = 1280, unsigned int height = 720)
		: Title(title), Width(width), Height(height) {}
};

class Window
{

public:

	using EventCallbackFn = std::function<void(Event&)>;

	Window(const WindowProps& props);
	~Window();

	void OnUpdate();

	inline unsigned int GetWidth() const { return m_Data.Width; }
	inline unsigned int GetHeight() const { return m_Data.Height; }

	std::pair<uint32_t, uint32_t> GetSize() const { return { m_Data.Width, m_Data.Height }; }
	std::pair<float, float> GetWindowPos() const;

	inline void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }
	void SetVSync(bool enabled);
	bool IsVSync() const;

	inline void* GetWindow() const { return m_Window; }

private:

	void Init(const WindowProps& props);
	void Shutdown();

private:

	GLFWwindow* m_Window;
	GLFWcursor* m_ImGuiMouseCursors[9] = { 0 };

	struct WindowData
	{
		std::string Title;

		uint32_t Width;
		uint32_t Height;

		bool VSync;

		EventCallbackFn EventCallback;
	};

	WindowData m_Data;

	float m_LastFrameTime = 0.0f;
};