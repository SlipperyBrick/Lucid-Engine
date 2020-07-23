#include "ldpch.h"

#include <glad/glad.h>

#include <imgui/imgui.h>

#include "Window.h"

#include "Lucid/Core/Events/ApplicationEvent.h"
#include "Lucid/Core/Events/KeyEvent.h"
#include "Lucid/Core/Events/MouseEvent.h"

// Logs GLFW errors by printing the error code and a short description
static void GLFWErrorCallback(int error, const char* description)
{
	LD_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
}

// Static member for setting GLFW initalization
static bool s_GLFWInitialized = false;

// Constructor that sets the windows properties and calls Init() for initalizing GLFW and GLAD
Window::Window(const WindowProps& props)
{
	Init(props);
}

// Calls Shutdown() when window is deconstructed
Window::~Window()
{
	Shutdown();
}

// Returns a pointer of a Window instance
Window* Window::Create(const WindowProps& props)
{
	return new Window(props);
}

// Initalizes the window by setting the windows properties and initalizing GLFW and GLAD a long with setting up GLFW window callbacks
void Window::Init(const WindowProps& props)
{
	m_Data.Title = props.Title;
	m_Data.Width = props.Width;
	m_Data.Height = props.Height;

	// Log window creation details
	LD_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

	// Check if GLFW initialized correctly
	if (!s_GLFWInitialized)
	{
		int success = glfwInit();

		// Sets function pointer for retrieving GLFW error information
		glfwSetErrorCallback(GLFWErrorCallback);

		// Log and assert if GLFW didn't initalize
		LD_CORE_ASSERT(success, "Could not intialize GLFW!");

		s_GLFWInitialized = true;
	}

	// Create the window
	m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);

	// Make the window the current rendering context
	glfwMakeContextCurrent(m_Window);
	glfwMaximizeWindow(m_Window);

	// Initalize GLAD
	int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	// Log and assert if GLAD didn't initialize
	LD_CORE_ASSERT(status, "Failed to initialize GLAD!");

	// Set the window pointer to the newly created window
	glfwSetWindowUserPointer(m_Window, &m_Data);

	// Set window resize GLFW callback
	glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

			WindowResizeEvent event((unsigned int)width, (unsigned int)height);
			data.EventCallback(event);
			data.Width = width;
			data.Height = height;
		});

	// Set window close GLFW callback
	glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

			WindowCloseEvent event;
			data.EventCallback(event);
		});

	// Set window key input GLFW callback
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

	// Set window character input GLFW callback
	glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int codepoint)
		{
			auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

			KeyTypedEvent event((int)codepoint);
			data.EventCallback(event);
		});

	// Set window mouse input GLFW callback
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

	// Set window mouse scroll GLFW callback
	glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});

	// Set window mouse position GLFW callback
	glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double x, double y)
		{
			auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

			MouseMovedEvent event((float)x, (float)y);
			data.EventCallback(event);
		});

	// Set ImGui mouse cursors using GLFW cursor functions
	m_ImGuiMouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	m_ImGuiMouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
	m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
	m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
	m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
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

// On window shutdown destroy the current window and terminate GLFW
void Window::Shutdown()
{
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

// Getter to retrieve position of the window
inline std::pair<float, float> Window::GetWindowPos() const
{
	int x;
	int y;

	glfwGetWindowPos(m_Window, &x, &y);

	return { x, y };
}

// Processes events and swaps render buffers, also gets the mouse position and processes the windows timestep
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

// Sets if the window uses vertical-sync
void Window::SetVSync(bool isEnabled)
{
	if (isEnabled)
	{
		glfwSwapInterval(1);
	}
	else
	{
		glfwSwapInterval(0);
	}

	m_Data.VSync = isEnabled;
}

// Returns true if the window has vertical-sync enabled
bool Window::IsVSync() const
{
	return m_Data.VSync;
}

// Handles if a key is pressed
bool Window::IsKeyPressed(int keycode)
{
	auto state = glfwGetKey(m_Window, keycode);

	return state == GLFW_PRESS || state == GLFW_REPEAT;
}

// Handles if a mouse button is pressed
bool Window::IsMouseButtonPressed(int button)
{
	auto state = glfwGetMouseButton(m_Window, button);

	return state == GLFW_PRESS;
}

// Retrieves the mouse x position
float Window::GetMouseX()
{
	auto [x, y] = GetMousePosition();

	return (float)x;
}

// Retrieves the mouse y position
float Window::GetMouseY()
{
	auto [x, y] = GetMousePosition();

	return (float)y;
}

// Retrieves the mouse x and y coordinates
std::pair<float, float> Window::GetMousePosition()
{
	double x;
	double y;

	glfwGetCursorPos(m_Window, &x, &y);

	return { (float)x, (float)y };
}

void Window::SetTitle(const std::string& title)
{
	m_Data.Title = title;
	glfwSetWindowTitle(m_Window, m_Data.Title.c_str());
}