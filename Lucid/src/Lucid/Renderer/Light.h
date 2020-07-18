#pragma once

#include <glm/glm.hpp>

class Light : public RefCounted
{

	enum class Type
	{
		Point = 0,
		Spot = 1,
		Directional = 2
	};

public:

	Light() = default;
	Light(glm::vec3 position, glm::vec3 colour);
	virtual ~Light();

public:

	void GetLightType();
	void SetLightType();

public:

	glm::vec3 m_Position;
	glm::vec4 m_Colour;

	float m_Brightness;
	float m_Multiplier;

private:

};

class Point : public Light
{

};

class Spot : public Light
{

};

class Directional : public Light
{

};