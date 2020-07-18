#pragma once

#include <glm/glm.hpp>

#include "Lucid/Core/LucidUUID.h"

#include "Lucid/Renderer/Texture.h"
#include "Lucid/Renderer/Mesh.h"

struct IDComponent
{
	LucidUUID ID = 0;
};

struct TagComponent
{
	std::string Tag;

	TagComponent() = default;

	TagComponent(const TagComponent& other)
		: Tag(other.Tag) {}

	TagComponent(const std::string& tag)
		: Tag(tag) {}

	operator std::string& () { return Tag; }
	operator const std::string& () const { return Tag; }
};

struct TransformComponent
{
	glm::mat4 TransformComp;

	TransformComponent() = default;

	TransformComponent(const TransformComponent& other)
		: TransformComp(other.TransformComp) {}

	TransformComponent(const glm::mat4& transform)
		: TransformComp(transform) {}

	operator glm::mat4& () { return TransformComp; }
	operator const glm::mat4& () const { return TransformComp; }
};

struct MeshComponent
{
	Ref<Mesh> MeshComp;

	MeshComponent() = default;

	MeshComponent(const MeshComponent& other)
		: MeshComp(other.MeshComp) {}

	MeshComponent(const Ref<Mesh>& mesh)
		: MeshComp(mesh) {}

	operator Ref<Mesh>() { return MeshComp; }
};

struct LightComponent
{
	Ref<Light> LightComp;

	LightComponent() = default;

	LightComponent(const LightComponent& other)
		: LightComp(other.LightComp) {}

	LightComponent(const Ref<Light>& light)
		: LightComp(light) {}

	operator Ref<Light>() { return LightComp; }
};