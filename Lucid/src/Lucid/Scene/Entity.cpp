#pragma once

#include "ldpch.h"

#include "Entity.h"

Entity::Entity(const std::string& name)
	: m_Name(name), m_Transform(1.0f)
{
}

Entity::~Entity()
{
}