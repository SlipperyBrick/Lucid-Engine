#pragma once

#include "Lucid/Renderer/Camera.h"

#include "Lucid/Core/Timestep.h"
#include "Lucid/Core/Events/MouseEvent.h"

class EditorCamera : public Camera
{

public:

	EditorCamera() = default;
	EditorCamera(const glm::mat4& projectionMatrix);

	void Focus();
	void OnUpdate(Timestep ts);
	void OnEvent(Event& e);

	inline float GetDistance() const { return m_Distance; }
	inline void SetDistance(float distance) { m_Distance = distance; }

	inline void SetViewportSize(uint32_t width, uint32_t height) { m_ViewportWidth = width; m_ViewportHeight = height; }

	const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
	glm::mat4 GetViewProjection() const { return m_ProjectionMatrix * m_ViewMatrix; }

	glm::vec3 GetUpDirection();
	glm::vec3 GetRightDirection();
	glm::vec3 GetForwardDirection();

	const glm::vec3& GetPosition() const { return m_Position; }
	glm::quat GetOrientation() const;

	float GetExposure() const { return m_Exposure; }
	float& GetExposure() { return m_Exposure; }

	float GetPitch() const { return m_Pitch; }
	float GetYaw() const { return m_Yaw; }

private:

	void UpdateCameraView();

	bool OnMouseScroll(MouseScrolledEvent& e);

	void MousePan(const glm::vec2& delta);
	void MouseRotate(const glm::vec2& delta);
	void MouseZoom(float delta);

	glm::vec3 CalculatePosition();

	std::pair<float, float> PanSpeed() const;
	float RotationSpeed() const;
	float ZoomSpeed() const;

private:

	glm::mat4 m_ViewMatrix;
	glm::vec3 m_Position;
	glm::vec3 m_Rotation;
	glm::vec3 m_FocalPoint;

	bool m_Panning;
	bool m_Rotating;

	glm::vec2 m_InitialMousePosition;
	glm::vec3 m_InitialFocalPoint;
	glm::vec3 m_InitialRotation;

	float m_Distance;
	float m_Pitch, m_Yaw;

	float m_Exposure = 1.0f;

	uint32_t m_ViewportWidth = 1280;
	uint32_t m_ViewportHeight = 720;
};