#pragma once

#include "Lucid/Renderer/RenderPass.h"

#include "Lucid/Scene/Scene.h"

struct SceneRendererOptions
{
	bool ShowGrid = true;
	bool ShowBoundingBoxes = false;
};

class SceneRenderer
{

public:

	static void Init();

	static void SetViewportSize(uint32_t width, uint32_t height);

	static void BeginScene(const Scene* scene);
	static void EndScene();

	static void SubmitEntity(Entity* entity);

	static Ref<RenderPass> GetFinalRenderPass();
	static Ref<Texture2D> GetFinalColourBuffer();

	static uint32_t GetFinalColourBufferRendererID();

	static SceneRendererOptions& GetOptions();

private:

	static void FlushDrawList();
	static void GeometryPass();
	static void CompositePass();
};