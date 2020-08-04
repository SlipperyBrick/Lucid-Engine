#include "ldpch.h"

#include "Camera.h"

Camera::Camera(const glm::mat4& projectionMatrix)
	: m_ProjectionMatrix(projectionMatrix)
{
}