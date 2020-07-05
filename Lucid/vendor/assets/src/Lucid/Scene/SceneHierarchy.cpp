#pragma once

#include "ldpch.h"

#include "SceneHierarchy.h"

#include <imgui.h>

#include <assimp/scene.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "Lucid/Renderer/Mesh.h"

glm::mat4 Mat4FromAssimpMat4(const aiMatrix4x4& matrix);

SceneHierarchy::SceneHierarchy(const Ref<Scene>& context)
	: m_Context(context)
{
}

void SceneHierarchy::SetContext(const Ref<Scene>& scene)
{
	m_Context = scene;
}

void SceneHierarchy::OnImGuiRender()
{
	ImGui::Begin("Scene Hierarchy");

	uint32_t entityCount = 0, meshCount = 0;
	auto& sceneEntities = m_Context->m_Entities;

	for (Entity* entity : sceneEntities)
	{
		DrawEntityNode(entity, entityCount, meshCount);
	}

	ImGui::End();
}

void SceneHierarchy::DrawEntityNode(Entity* entity, uint32_t& imguiEntityID, uint32_t& imguiMeshID)
{
	const char* name = entity->GetName().c_str();

	static char imguiName[128];

	memset(imguiName, 0, 128);

	sprintf(imguiName, "%s##%d", name, imguiEntityID++);

	if (ImGui::TreeNode(imguiName))
	{
		auto mesh = entity->GetMesh();
		auto material = entity->GetMaterial();
		const auto& transform = entity->GetTransform();

		if (mesh)
		{
			DrawMeshNode(mesh, imguiMeshID);
		}

		ImGui::TreePop();
	}
}

void SceneHierarchy::DrawMeshNode(const Ref<Mesh>& mesh, uint32_t& imguiMeshID)
{
	static char imguiName[128];

	memset(imguiName, 0, 128);

	sprintf(imguiName, "Mesh##%d", imguiMeshID++);

	// Mesh hierarchy
	if (ImGui::TreeNode(imguiName))
	{
		auto rootNode = mesh->m_Scene->mRootNode;

		MeshNodeHierarchy(mesh, rootNode);

		ImGui::TreePop();
	}
}

static std::tuple<glm::vec3, glm::quat, glm::vec3> GetTransformDecomposition(const glm::mat4& transform)
{
	glm::vec3 scale;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::quat orientation;

	glm::decompose(transform, scale, orientation, translation, skew, perspective);

	return { translation, orientation, scale };
}

void SceneHierarchy::MeshNodeHierarchy(const Ref<Mesh>& mesh, aiNode* node, const glm::mat4& parentTransform, uint32_t level)
{
	glm::mat4 localTransform = Mat4FromAssimpMat4(node->mTransformation);
	glm::mat4 transform = parentTransform * localTransform;

	if (ImGui::TreeNode(node->mName.C_Str()))
	{
		{
			auto [translation, rotation, scale] = GetTransformDecomposition(transform);

			ImGui::Text("World Transform");
			ImGui::Text("  Translation: %.2f, %.2f, %.2f", translation.x, translation.y, translation.z);
			ImGui::Text("  Scale: %.2f, %.2f, %.2f", scale.x, scale.y, scale.z);
		}
		{
			auto [translation, rotation, scale] = GetTransformDecomposition(localTransform);

			ImGui::Text("Local Transform");
			ImGui::Text("  Translation: %.2f, %.2f, %.2f", translation.x, translation.y, translation.z);
			ImGui::Text("  Scale: %.2f, %.2f, %.2f", scale.x, scale.y, scale.z);
		}

		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			MeshNodeHierarchy(mesh, node->mChildren[i], transform, level + 1);
		}

		ImGui::TreePop();
	}
}