#pragma once

#include <glfw/glfw3.h>

#include <string>
#include <functional>

#include "Lucid/Core/Events/Event.h"

// Struct for defining the properties of a window such as, width, height and window title
struct WindowProps
{
	std::string Title;
	unsigned int Width;
	unsigned int Height;

	WindowProps(const std::string& title = "Lucid Engine", unsigned int width = 1280, unsigned int height = 720)
		: Title(title), Width(width), Height(height) {}
};

class Window : public RefCounted
{

public:

	using EventCallbackFn = std::function<void(Event&)>;

	Window(const WindowProps& props);
	~Window();

	static Window* Create(const WindowProps& props);

	void OnUpdate();

	inline unsigned int GetWidth() const { return m_Data.Width; }
	inline unsigned int GetHeight() const { return m_Data.Height; }

	std::pair<uint32_t, uint32_t> GetSize() const { return { m_Data.Width, m_Data.Height }; }
	std::pair<float, float> GetWindowPos() const;

	inline void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }
	void SetVSync(bool enabled);
	bool IsVSync() const;

	inline void* GetWindowPointer() const { return m_Window; }

	bool IsKeyPressed(int keycode);
	bool IsMouseButtonPressed(int button);

	float GetMouseX();
	float GetMouseY();

	std::pair<float, float> GetMousePosition();

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