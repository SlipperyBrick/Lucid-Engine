#include "ldpch.h"

#include "EditorCamera.h"

#include <glfw/glfw3.h>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "Lucid/Core/Application.h"

// Definition of PI used for calculating camera angles
#define M_PI 3.14159f

EditorCamera::EditorCamera(const glm::mat4& projectionMatrix)
	: Camera(projectionMatrix)
{
	m_Rotation = glm::vec3(90.0f, 0.0f, 0.0f);
	m_FocalPoint = glm::vec3(0.0f);

	glm::vec3 position = { -5, 5, 5 };
	m_Distance = glm::distance(position, m_FocalPoint);

	m_Yaw = 3.0f * (float)M_PI / 4.0f;
	m_Pitch = M_PI / 4.0f;

	m_Speed = 10.0f;

	UpdateCameraView();
}

void EditorCamera::UpdateCameraView()
{
	m_Position = CalculatePosition();

	glm::quat orientation = GetOrientation();

	m_Rotation = glm::eulerAngles(orientation) * (180.0f / (float)M_PI);

	m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);

	m_ViewMatrix = glm::inverse(m_ViewMatrix);
}

void EditorCamera::Focus()
{
}

std::pair<float, float> EditorCamera::PanSpeed() const
{
	// Max pan speed is 2.4
	float x = std::min(m_ViewportWidth / 1000.0f, 2.4f);
	float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

	// Max pan speed is 2.4
	float y = std::min(m_ViewportHeight / 1000.0f, 2.4f);
	float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

	return { xFactor, yFactor };
}

float EditorCamera::RotationSpeed() const
{
	return 0.8f;
}

float EditorCamera::ZoomSpeed() const
{
	float distance = m_Distance * 0.2f;

	distance = std::max(distance, 0.0f);

	float speed = distance * distance;

	// Max speed is 100
	speed = std::min(speed, 100.0f);

	return speed;
}

void EditorCamera::OnUpdate(Timestep ts)
{
	Window& window = Application::Get().GetWindow();

	if (window.IsKeyPressed(LD_KEY_LEFT_ALT))
	{
		const glm::vec2& mouse{ window.GetMouseX(), window.GetMouseY() };

		glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;

		m_InitialMousePosition = mouse;

		if (window.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE))
		{
			MousePan(delta);
		}
		else if (window.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
		{
			MouseRotate(delta);
		}
		else if (window.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
		{
			MouseZoom(delta.y);
		}
	}

	// Store initial mouse position before clicking RMB
	/*const glm::vec2& mouse{ window.GetMouseX(), window.GetMouseY() };

	glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;

	m_InitialMousePosition = mouse;*/

	//if (window.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
	//{
	//	// Set the cursor to be invisible and locked to center of window
	//	glfwSetInputMode(window.GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//	MouseRotate(delta);

	//	if (window.IsKeyPressed(LD_KEY_W))
	//	{
	//		float velocity = m_Speed * ts;
	//		m_FocalPoint += GetForwardDirection() * velocity;
	//	} 
	//	else if (window.IsKeyPressed(LD_KEY_S))
	//	{
	//		float velocity = m_Speed * ts;
	//		m_FocalPoint += -GetForwardDirection() * velocity;
	//	}
	//	else if (window.IsKeyPressed(LD_KEY_A))
	//	{
	//		float velocity = m_Speed * ts;
	//		m_FocalPoint += -GetRightDirection() * velocity;
	//	}
	//	else if (window.IsKeyPressed(LD_KEY_D))
	//	{
	//		float velocity = m_Speed * ts;
	//		m_FocalPoint += GetRightDirection() * velocity;
	//	}
	//	else if (window.IsKeyPressed(LD_KEY_E))
	//	{
	//		float velocity = m_Speed * ts;
	//		m_FocalPoint += GetUpDirection() * velocity;
	//	}
	//	else if (window.IsKeyPressed(LD_KEY_Q))
	//	{
	//		MouseRotate(delta);

	//		float velocity = m_Speed * ts;
	//		m_FocalPoint += -GetUpDirection() * velocity;
	//	}
	//}

	UpdateCameraView();
}

void EditorCamera::OnEvent(Event& e)
{
	EventDispatcher dispatcher(e);
	dispatcher.Dispatch<MouseScrolledEvent>(LD_BIND_EVENT_FN(EditorCamera::OnMouseScroll));
}

bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
{
	float delta = e.GetYOffset() * 0.1f;

	MouseZoom(delta);

	UpdateCameraView();

	return false;
}

void EditorCamera::MousePan(const glm::vec2& delta)
{
	auto [xSpeed, ySpeed] = PanSpeed();

	m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
	m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
}

void EditorCamera::MouseRotate(const glm::vec2& delta)
{
	float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;

	m_Yaw += yawSign * delta.x * RotationSpeed();
	m_Pitch += delta.y * RotationSpeed();
}

void EditorCamera::MouseZoom(float delta)
{
	m_Distance -= delta * ZoomSpeed();

	if (m_Distance < 1.0f)
	{
		m_FocalPoint += GetForwardDirection();
		m_Distance = 1.0f;
	}
}

glm::vec3 EditorCamera::GetUpDirection()
{
	return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::vec3 EditorCamera::GetRightDirection()
{
	return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
}

glm::vec3 EditorCamera::GetForwardDirection()
{
	return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
}

glm::vec3 EditorCamera::CalculatePosition()
{
	return m_FocalPoint - GetForwardDirection() * m_Distance;
}

glm::quat EditorCamera::GetOrientation() const
{
	return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
}