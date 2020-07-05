#pragma once

#include <glm/glm.hpp>

struct AABB
{
	glm::vec3 Min;
	glm::vec3 Max;

	AABB()
		: Min(0.0f), Max(0.0f) {}

	AABB(const glm::vec3& min, const glm::vec3& max)
		: Min(min), Max(max) {}

};