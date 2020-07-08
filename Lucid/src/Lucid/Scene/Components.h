#pragma once

#include <glm/glm.hpp>

#include "Lucid/Renderer/Texture.h"
#include "Lucid/Renderer/Mesh.h"

//struct TransformComponent
//{
//	glm::mat4 Transform;
//
//	TransformComponent() = default;
//
//	TransformComponent(const TransformComponent& other)
//		: Transform(other.Transform) {}
//
//	TransformComponent(const glm::mat4& transform)
//		: Transform(transform) {}
//
//	operator glm::mat4& () { return Transform; }
//	operator const glm::mat4& () const { return Transform; }
//};

//struct MeshComponent
//{
//	Ref<Mesh> Mesh;
//
//	MeshComponent() = default;
//
//	MeshComponent(const MeshComponent& other)
//		: Mesh(other.Mesh) {}
//
//	MeshComponent(const Ref<Mesh>& mesh)
//		: Mesh(mesh) {}
//
//	operator Ref<Mesh>() { return Mesh; }
//};