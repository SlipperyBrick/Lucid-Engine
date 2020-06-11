#include "ldpch.h"

#include <glad/glad.h>

#include "Window.h"

#include "Lucid/Core/Log.h"
#include "Lucid/Core/Base.h"

#include "Lucid/Core/Events/ApplicationEvent.h"
#include "Lucid/Core/Events/KeyEvent.h"
#include "Lucid/Core/Events/MouseEvent.h"

#include <imgui/imgui.h>

static void GLFWErrorCallback(int error, const char* description)
{
	LD_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
}

static bool s_GLFWInitialized = false;

Window::Window(const WindowProps& props)
{
	Init(props);
}

Window::~Window()
{
	Shutdown();
}

void Window::Init(const WindowProps& props)
{
	m_Data.Title = props.Title;
	m_Data.Width = props.Width;
	m_Data.Height = props.Height;

	LD_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

	if (!s_GLFWInitialized)
	{
		int success = glfwInit();

		LD_CORE_ASSERT(success, "Could not intialize GLFW!");

		glfwSetErrorCallback(GLFWErrorCallback);

		s_GLFWInitialized = true;
	}

	m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);

	glfwMakeContextCurrent(m_Window);
	glfwMaximizeWindow(m_Window);

	int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	LD_CORE_ASSERT(status, "Failed to initialize GLAD!");

	glfwSetWindowUserPointer(m_Window, &m_Data);

	// Set GLFW callbacks
	glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

			WindowResizeEvent event((unsigned int)width, (unsigned int)height);
			data.EventCallback(event);
			data.Width = width;
			data.Height = height;
		});

	glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

			WindowCloseEvent event;
			data.EventCallback(event);
		});

	glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, 1);
					data.EventCallback(event);
					break;
				}
			}
		});

	glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int codepoint)
		{
			auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

			KeyTypedEvent event((int)codepoint);
			data.EventCallback(event);
		});

	glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
		{
			auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					data.EventCallback(event);
					break;
				}
			}
		});

	glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});

	glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double x, double y)
		{
			auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

			MouseMovedEvent event((float)x, (float)y);
			data.EventCallback(event);
		});

	m_ImGuiMouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	m_ImGuiMouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
	m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);   // FIXME: GLFW doesn't have this.
	m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
	m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
	m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
	m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
	m_ImGuiMouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

	// Update window size to actual size
	{
		int width;
		int height;

		glfwGetWindowSize(m_Window, &width, &height);

		m_Data.Width = width;
		m_Data.Height = height;
	}
}

void Window::Shutdown()
{
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

inline std::pair<float, float> Window::GetWindowPos() const
{
	int x, y;
	glfwGetWindowPos(m_Window, &x, &y);

	return { x, y };
}

void Window::OnUpdate()
{
	glfwPollEvents();
	glfwSwapBuffers(m_Window);

	ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();

	glfwSetCursor(m_Window, m_ImGuiMouseCursors[imgui_cursor] ? m_ImGuiMouseCursors[imgui_cursor] : m_ImGuiMouseCursors[ImGuiMouseCursor_Arrow]);
	glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	float time = glfwGetTime();
	float delta = time - m_LastFrameTime;
	m_LastFrameTime = time;
}

void Window::SetVSync(bool enabled)
{
	if (enabled)
	{
		glfwSwapInterval(1);
	}
	else
	{
		glfwSwapInterval(0);
	}

	m_Data.VSync = enabled;
}

bool Window::IsVSync() const
{
	return m_Data.VSync;
}