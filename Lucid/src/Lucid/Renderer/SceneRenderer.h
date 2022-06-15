#pragma once

#include "Lucid/Renderer/Mesh.h"
#include "Lucid/Renderer/RenderPass.h"

#include "Lucid/Scene/Scene.h"

struct SceneRendererOptions
{
	bool ShowDepthPeeling = false;
	bool ShowPosition = false;
	bool ShowNormal = false;
	bool ShowDiffuse = false;
	bool ShowSpecular = false;
	bool ShowGrid = true;
	bool ShowBoundingBoxes = false;
	bool SetCameraMode = false;

	int LayerPeels = 4;
};

struct SceneRendererCamera
{
	Camera Camera;
	glm::mat4 ViewMatrix;
};

// Interface between renderer and scene, manages all state that is required to render a given scene
class SceneRenderer
{

public:

	static void Init();

	static void SetViewportSize(uint32_t width, uint32_t height);

	static void BeginScene(const Scene* scene, const SceneRendererCamera& camera);
	static void EndScene();

	static void SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform = glm::mat4(1.0f), Ref<MaterialInstance> overrideMaterial = nullptr, bool transparency = false);
	static void SubmitSelectedMesh(Ref<Mesh> mesh, const glm::mat4& transform = glm::mat4(1.0f), bool transparency = false);

	static Ref<RenderPass> GetFinalRenderPass();

	static uint32_t GetFinalColourBufferRendererID();

	static SceneRendererOptions& GetOptions();

private:

	static void FlushDrawList();

	static void GeometryPass();
	static void LightingPass();
	static void TransparencyPass();
	static void CompositePass();
};