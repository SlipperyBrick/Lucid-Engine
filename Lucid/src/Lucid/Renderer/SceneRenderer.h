#pragma once

#include "Lucid/Renderer/RenderPass.h"
#include "Lucid/Renderer/Texture.h"

#include "Lucid/Scene/Scene.h"
#include "Lucid/Scene/Entity.h"

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

	static std::pair<Ref<TextureCube>, Ref<TextureCube>> CreateEnvironmentMap(const std::string& filepath);

	static Ref<RenderPass> GetFinalRenderPass();
	static Ref<Texture2D> GetFinalColorBuffer();

	static uint32_t GetFinalColorBufferRendererID();

	static SceneRendererOptions& GetOptions();

private:

	static void FlushDrawList();
	static void GeometryPass();
	static void CompositePass();
};