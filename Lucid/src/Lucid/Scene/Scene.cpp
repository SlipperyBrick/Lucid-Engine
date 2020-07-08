#pragma once

#include "ldpch.h"

#include "Scene.h"

#include "Lucid/Renderer/SceneRenderer.h"

#include "Lucid/Scene/Components.h"

static const std::string DefaultEntityName = "Entity";

Scene::Scene(const std::string& debugName)
	: m_DebugName(debugName)
{
	Init();
}

Scene::~Scene()
{
	for (Entity* entity : m_Entities)
	{
		delete entity;
	}
}

void Scene::Init()
{
}

void Scene::OnUpdate(Timestep ts)
{
	SceneRenderer::BeginScene(this);

	// Render all entities
	for (auto entity : m_Entities)
	{
		if (m_SelectedEntity == entity)
		{
			SceneRenderer::SubmitSelectedMesh(entity, transformComponent);
		}
		else
		{
			SceneRenderer::SubmitMesh(entity, transformComponent, nullptr);
		}
	}

	SceneRenderer::EndScene();
}

void Scene::OnEvent(Event& e)
{
	m_Camera.OnEvent(e);
}

void Scene::SetCamera(const Camera& camera)
{
	m_Camera = camera;
}

void Scene::AddEntity(Entity* entity)
{
	m_Entities.push_back(entity);
}

Entity* Scene::CreateEntity(const std::string& name)
{
	const std::string& entityName = name.empty() ? DefaultEntityName : name;

	Entity* entity = new Entity(entityName);

	AddEntity(entity);

	return entity;
}