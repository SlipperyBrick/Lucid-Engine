#pragma once

#include "Lucid/Renderer/Mesh.h"
#include "Lucid/Renderer/RenderPass.h"

#include "Lucid/Scene/Scene.h"

struct SceneRendererOptions
{
	bool ShowGrid = true;
	bool ShowBoundingBoxes = false;
};

struct SceneRendererCamera
{
	Camera Camera;
	glm::mat4 ViewMatrix;
};

struct DirectionalLight
{
	glm::vec3 Direction = { 0.0f, 0.0f, 0.0f };
	glm::vec3 Colour = { 1.0f, 1.0f, 1.0f };

	float Brightness = 0.0f;
};

struct PointLight
{
	glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 Colour = { 1.0f, 1.0f, 1.0f };

	float Brightness = 0.0f;
	float Falloff = 0.0f;
	float Slope = 0.0f;
};

struct LightEnvironment
{
	DirectionalLight DirectionalLights[4];
	PointLight PointLights[4];
};

class SceneRenderer
{

public:

	static void Init();

	static void SetViewportSize(uint32_t width, uint32_t height);

	static void BeginScene(const Scene* scene, const SceneRendererCamera& camera);
	static void EndScene();

	static void SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform = glm::mat4(1.0f), Ref<MaterialInstance> overrideMaterial = nullptr);
	static void SubmitSelectedMesh(Ref<Mesh> mesh, const glm::mat4& transform = glm::mat4(1.0f));

	static void SetLightEnvironment(const LightEnvironment& lightEnvironment);

	static Ref<RenderPass> GetFinalRenderPass();
	static Ref<Texture2D> GetFinalColourBuffer();

	static uint32_t GetFinalColourBufferRendererID();

	static SceneRendererOptions& GetOptions();

private:

	static void FlushDrawList();
	static void GeometryPass();
	static void CompositePass();
};