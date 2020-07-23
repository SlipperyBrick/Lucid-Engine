#pragma once

#include "Lucid/Core/LucidUUID.h"

#include <entt/entt.hpp>

#include "Lucid/ImGui/EditorCamera.h"

class Entity;

using EntityMap = std::unordered_map<LucidUUID, Entity>;

class Scene : public RefCounted
{

public:

	Scene(const std::string& debugName = "Scene");
	~Scene();

	void Init();

	void OnUpdate(Timestep ts, const EditorCamera& editorCamera);
	void OnEvent(Event& e);

	void SetViewportSize(uint32_t width, uint32_t height);

	Entity CreateEntity(const std::string& name = "");
	Entity CreateEntityWithID(LucidUUID uuid, const std::string& name = "", bool runtimeMap = false);

	void DestroyEntity(Entity entity);

	void DuplicateEntity(Entity entity);

	template<typename T>
	auto GetAllEntitiesWith()
	{
		return m_Registry.view<T>();
	}

	const EntityMap& GetEntityMap() const { return m_EntityIDMap; }

	void CopyTo(Ref<Scene>& target);

	LucidUUID GetUUID() const { return m_SceneID; }

	static Ref<Scene> GetScene(LucidUUID uuid);

	void SetSelectedEntity(entt::entity entity) { m_SelectedEntity = entity; }

private:

	LucidUUID m_SceneID;

	entt::entity m_SceneEntity;
	entt::registry m_Registry;

	EntityMap m_EntityIDMap;

	std::string m_DebugName;

	uint32_t m_ViewportWidth = 0;
	uint32_t m_ViewportHeight = 0;

	entt::entity m_SelectedEntity;

	friend class Entity;
	friend class SceneRenderer;
	friend class SceneHierarchy;
};