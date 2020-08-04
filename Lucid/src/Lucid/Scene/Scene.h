#pragma once

#include "Lucid/Core/LucidUUID.h"

#include <entt/entt.hpp>

#include "Lucid/ImGui/EditorCamera.h"

struct DirectionalLight
{
	glm::vec3 Direction = { 0.0f, 0.0f, -1.0f };
	glm::vec3 Diffuse = { 1.0f, 1.0f, 1.0f };
	glm::vec3 Ambient = { 0.1f, 0.1f, 0.1f };
	glm::vec3 Specular = { 1.0f, 1.0f, 1.0f };

	float Brightness = 0.5f;
};

struct PointLight
{
	glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 Diffuse = { 1.0f, 1.0f, 1.0f };
	glm::vec3 Specular = { 1.0f, 1.0f, 1.0f };

	float Brightness = 0.0f;
	float Quadratic = 1.0f;
};

struct LightEnvironment
{
	PointLight PointLights[4];
};

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

	void SetLightEnvironment(const LightEnvironment& lightEnvironment);

	DirectionalLight& GetDirectionalLight() { return m_Light; }
	const DirectionalLight& GetDirectionalLight() const { return m_Light; }

	LightEnvironment& GetLightEnvironment() { return m_LightEnvironment; }
	const LightEnvironment& GetLightEnvironment() const { return m_LightEnvironment; }

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

	LightEnvironment m_LightEnvironment;

	DirectionalLight m_Light;

	entt::entity m_SelectedEntity;

	friend class Entity;
	friend class SceneRenderer;
	friend class SceneHierarchy;
	friend class SceneSerializer;
};