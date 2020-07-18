#pragma once

#include "ldpch.h"

#include "Scene.h"

#include "Lucid/Renderer/SceneRenderer.h"

#include "Lucid/Scene/Entity.h"
#include "Lucid/Scene/Components.h"

static const std::string DefaultEntityName = "Entity";

std::unordered_map<LucidUUID, Scene*> s_ActiveScenes;

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
	auto group = m_Registry.group<MeshComponent>(entt::get<TransformComponent>);

	SceneRenderer::BeginScene(this, { editorCamera, editorCamera.GetViewMatrix() });

	for (auto entity : group)
	{
		auto [transformComponent, meshComponent] = group.get<TransformComponent, MeshComponent>(entity);

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

	CopyComponentIfExists<TransformComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
	CopyComponentIfExists<MeshComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
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
}

Ref<Scene> Scene::GetScene(LucidUUID uuid)
{
	if (s_ActiveScenes.find(uuid) != s_ActiveScenes.end())
	{
		return s_ActiveScenes.at(uuid);
	}

	return {};
}