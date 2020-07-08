#pragma once

#include "Lucid/Scene/Entity.h"

#include "Lucid/Renderer/Camera.h"

struct Light
{
	glm::vec3 Position;
	glm::vec3 Radiance;

	float Multiplier = 1.0f;
};

class Scene : public RefCounted
{

public:

	Scene(const std::string& debugName = "Scene");
	~Scene();

	void Init();

	void OnUpdate(Timestep ts);
	void OnEvent(Event& e);

	void SetCamera(const Camera& camera);
	Camera& GetCamera() { return m_Camera; }

	Light& GetLight() { return m_Light; }

	void AddEntity(Entity* entity);
	Entity* CreateEntity(const std::string& name = "");

private:

	std::string m_DebugName;

	std::vector<Entity*> m_Entities;

	Entity* m_SelectedEntity;

	Camera m_Camera;

	Light m_Light;
	float m_LightMultiplier = 0.3f;

	friend class SceneRenderer;
	friend class SceneHierarchy;
};