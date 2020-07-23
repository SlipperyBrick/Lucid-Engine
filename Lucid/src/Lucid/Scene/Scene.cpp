#pragma once

#include "ldpch.h"

#include "Scene.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "Lucid/Renderer/SceneRenderer.h"

#include "Lucid/Scene/Entity.h"
#include "Lucid/Scene/Components.h"

static const std::string DefaultEntityName = "Entity";

std::unordered_map<LucidUUID, Scene*> s_ActiveScenes;

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

struct SceneComponent
{
	LucidUUID SceneID;
};

Scene::Scene(const std::string& debugName)
	: m_DebugName(debugName)
{
	m_SceneEntity = m_Registry.create();
	m_Registry.emplace<SceneComponent>(m_SceneEntity, m_SceneID);

	s_ActiveScenes[m_SceneID] = this;

	Init();
}

Scene::~Scene()
{
	m_Registry.clear();
	s_ActiveScenes.erase(m_SceneID);
}

void Scene::Init()
{
}

void Scene::OnUpdate(Timestep ts, const EditorCamera& editorCamera)
{
	SceneRenderer::BeginScene(this, { editorCamera, editorCamera.GetViewMatrix() });

	// Iterate over all lights
	{
		const auto& group = m_Registry.group<LightComponent>(entt::get<TransformComponent>);

		int directionalLightIndex = 0;
		int pointLightIndex = 0;

		LightEnvironment lightEnvironment;

		for (auto entity : group)
		{
			const auto& [transformComponent, lightComponent] = group.get<TransformComponent, LightComponent>(entity);

			if (lightComponent.Brightness <= 0.0f)
			{
				continue;
			}

			const auto& [translation, rotation, scale] = GetTransformDecomposition(transformComponent);

			switch (lightComponent.LightType)
			{
				case LightComponent::Type::Directional:
				{
					DirectionalLight& light = lightEnvironment.DirectionalLights[directionalLightIndex++];

					light.Direction = rotation * glm::vec3(0.0, 1.0, 1.0);
					light.Brightness = lightComponent.Brightness;
					light.Colour = lightComponent.Diffuse;

					break;
				}
				case LightComponent::Type::Point:
				{
					PointLight& light = lightEnvironment.PointLights[pointLightIndex++];

					light.Position = translation;
					light.Brightness = lightComponent.Brightness;
					light.Colour = lightComponent.Diffuse;
					light.Falloff = lightComponent.Falloff;
					light.Slope = lightComponent.Slope;

					break;
				}
			}
		}

		SceneRenderer::SetLightEnvironment(lightEnvironment);
	}

	// Iterate over all meshes
	{
		const auto& group = m_Registry.group<MeshComponent>(entt::get<TransformComponent>);

		for (auto entity : group)
		{
			const auto& [transformComponent, meshComponent] = group.get<TransformComponent, MeshComponent>(entity);

			if (meshComponent.MeshComp)
			{
				if (m_SelectedEntity == entity)
				{
					SceneRenderer::SubmitSelectedMesh(meshComponent, transformComponent);
				}
				else
				{
					SceneRenderer::SubmitMesh(meshComponent, transformComponent, nullptr);
				}
			}
		}
	}

	SceneRenderer::EndScene();
}

void Scene::OnEvent(Event& e)
{
}

void Scene::SetViewportSize(uint32_t width, uint32_t height)
{
	m_ViewportWidth = width;
	m_ViewportHeight = height;
}

void Scene::SetLightEnvironment(const LightEnvironment& lightEnvironment)
{
	SceneRenderer::SetLightEnvironment(lightEnvironment);
}

const LightEnvironment& Scene::GetLightEnvironment()
{
	return SceneRenderer::GetLightEnvironment();
}

Entity Scene::CreateEntity(const std::string& name)
{
	auto entity = Entity{ m_Registry.create(), this };

	auto& idComponent = entity.AddComponent<IDComponent>();

	idComponent.ID = {};

	entity.AddComponent<TransformComponent>(glm::mat4(1.0f));

	if (!name.empty())
	{
		entity.AddComponent<TagComponent>(name);
	}

	m_EntityIDMap[idComponent.ID] = entity;

	return entity;
}

Entity Scene::CreateEntityWithID(LucidUUID uuid, const std::string& name, bool runtimeMap)
{
	auto entity = Entity{ m_Registry.create(), this };

	auto& idComponent = entity.AddComponent<IDComponent>();
	idComponent.ID = uuid;

	entity.AddComponent<TransformComponent>(glm::mat4(1.0f));

	if (!name.empty())
	{
		entity.AddComponent<TagComponent>(name);
	}

	LD_CORE_ASSERT(m_EntityIDMap.find(uuid) == m_EntityIDMap.end());

	m_EntityIDMap[uuid] = entity;

	return entity;
}

void Scene::DestroyEntity(Entity entity)
{
	m_Registry.destroy(entity.m_EntityHandle);
}

template<typename T>
static void CopyComponent(entt::registry& dstRegistry, entt::registry& srcRegistry, const std::unordered_map<LucidUUID, entt::entity>& enttMap)
{
	auto components = srcRegistry.view<T>();

	for (auto srcEntity : components)
	{
		entt::entity destEntity = enttMap.at(srcRegistry.get<IDComponent>(srcEntity).ID);

		auto& srcComponent = srcRegistry.get<T>(srcEntity);
		auto& destComponent = dstRegistry.emplace_or_replace<T>(destEntity, srcComponent);
	}
}

template<typename T>
static void CopyComponentIfExists(entt::entity dst, entt::entity src, entt::registry& registry)
{
	if (registry.has<T>(src))
	{
		auto& srcComponent = registry.get<T>(src);
		registry.emplace_or_replace<T>(dst, srcComponent);
	}
}

void Scene::DuplicateEntity(Entity entity)
{
	Entity newEntity;

	if (entity.HasComponent<TagComponent>())
	{
		newEntity = CreateEntity(entity.GetComponent<TagComponent>().Tag);
	}
	else
	{
		newEntity = CreateEntity();
	}

	CopyComponentIfExists<TagComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
	CopyComponentIfExists<TransformComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
	CopyComponentIfExists<MeshComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
	CopyComponentIfExists<LightComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
}

void Scene::CopyTo(Ref<Scene>& target)
{
	std::unordered_map<LucidUUID, entt::entity> enttMap;

	auto idComponents = m_Registry.view<IDComponent>();

	for (auto entity : idComponents)
	{
		auto uuid = m_Registry.get<IDComponent>(entity).ID;
		Entity e = target->CreateEntityWithID(uuid, "", true);
		enttMap[uuid] = e.m_EntityHandle;
	}

	CopyComponent<TagComponent>(target->m_Registry, m_Registry, enttMap);
	CopyComponent<TransformComponent>(target->m_Registry, m_Registry, enttMap);
	CopyComponent<MeshComponent>(target->m_Registry, m_Registry, enttMap);
	CopyComponent<LightComponent>(target->m_Registry, m_Registry, enttMap);
}

Ref<Scene> Scene::GetScene(LucidUUID uuid)
{
	if (s_ActiveScenes.find(uuid) != s_ActiveScenes.end())
	{
		return s_ActiveScenes.at(uuid);
	}

	return {};
}