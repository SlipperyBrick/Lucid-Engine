#pragma once

#include "ldpch.h"

#include "SceneRenderer.h"

#include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp>

#include "Lucid/Renderer/Renderer.h"
#include "Lucid/Renderer/Renderer2D.h"

struct SceneRendererData
{
	const Scene* ActiveScene = nullptr;

	struct SceneInfo
	{
		Camera SceneCamera;

		// Resources
		Light ActiveLight;

	} SceneData;

	Ref<Shader> CompositeShader;

	Ref<RenderPass> GeoPass;
	Ref<RenderPass> CompositePass;

	struct DrawCommand
	{
		Ref<Mesh> Mesh;
		Ref<MaterialInstance> Material;
		glm::mat4 Transform;
	};

	std::vector<DrawCommand> DrawList;

	Ref<MaterialInstance> GridMaterial;

	SceneRendererOptions Options;
};

static SceneRendererData s_Data;

void SceneRenderer::Init()
{
	FramebufferSpecification geoFramebufferSpec;
	geoFramebufferSpec.Width = 1280;
	geoFramebufferSpec.Height = 720;
	geoFramebufferSpec.Format = FramebufferFormat::RGBA16F;
	geoFramebufferSpec.Samples = 8;
	geoFramebufferSpec.ClearColour = { 0.1f, 0.1f, 0.1f, 1.0f };

	RenderPassSpecification geoRenderPassSpec;
	geoRenderPassSpec.TargetFramebuffer = Framebuffer::Create(geoFramebufferSpec);
	s_Data.GeoPass = RenderPass::Create(geoRenderPassSpec);

	FramebufferSpecification compFramebufferSpec;
	compFramebufferSpec.Width = 1280;
	compFramebufferSpec.Height = 720;
	compFramebufferSpec.Format = FramebufferFormat::RGBA8;
	compFramebufferSpec.ClearColour = { 0.1f, 0.1f, 0.1f, 1.0f };

	RenderPassSpecification compRenderPassSpec;
	compRenderPassSpec.TargetFramebuffer = Framebuffer::Create(compFramebufferSpec);
	s_Data.CompositePass = RenderPass::Create(compRenderPassSpec);

	s_Data.CompositeShader = Shader::Create("assets/shaders/Composite.glsl");

	// Grid
	auto gridShader = Shader::Create("assets/shaders/Grid.glsl");

	s_Data.GridMaterial = MaterialInstance::Create(Material::Create(gridShader));

	float gridScale = 16.025f;
	float gridSize = 0.025f;

	s_Data.GridMaterial->Set("u_Scale", gridScale);
	s_Data.GridMaterial->Set("u_Res", gridSize);
}

void SceneRenderer::SetViewportSize(uint32_t width, uint32_t height)
{
	s_Data.GeoPass->GetSpecification().TargetFramebuffer->Resize(width, height);
	s_Data.CompositePass->GetSpecification().TargetFramebuffer->Resize(width, height);
}

void SceneRenderer::BeginScene(const Scene* scene)
{
	LD_CORE_ASSERT(!s_Data.ActiveScene, "");

	s_Data.ActiveScene = scene;

	s_Data.SceneData.SceneCamera = scene->m_Camera;
	s_Data.SceneData.ActiveLight = scene->m_Light;
}

void SceneRenderer::EndScene()
{
	LD_CORE_ASSERT(s_Data.ActiveScene, "");

	s_Data.ActiveScene = nullptr;

	FlushDrawList();
}

void SceneRenderer::SubmitEntity(Entity* entity)
{
	// Culling, sorting, etc, can be done here

	auto mesh = entity->GetMesh();

	if (!mesh)
	{
		return;
	}

	s_Data.DrawList.push_back({ mesh, entity->GetMaterial(), entity->GetTransform() });
}

void SceneRenderer::GeometryPass()
{
	Renderer::BeginRenderPass(s_Data.GeoPass);

	auto viewProjection = s_Data.SceneData.SceneCamera.GetViewProjection();

	// Render entities
	for (auto& dc : s_Data.DrawList)
	{
		auto baseMaterial = dc.Mesh->GetMaterial();
		baseMaterial->Set("u_ViewProjectionMatrix", viewProjection);
		baseMaterial->Set("u_CameraPosition", s_Data.SceneData.SceneCamera.GetPosition());

		// Set lights (TODO: move to light environment and don't do per mesh)
		baseMaterial->Set("lights", s_Data.SceneData.ActiveLight);

		auto overrideMaterial = nullptr;

		Renderer::SubmitMesh(dc.Mesh, dc.Transform, overrideMaterial);
	}

	// Grid
	if (GetOptions().ShowGrid)
	{
		s_Data.GridMaterial->Set("u_ViewProjection", viewProjection);
		Renderer::SubmitQuad(s_Data.GridMaterial, glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(16.0f)));
	}

	if (GetOptions().ShowBoundingBoxes)
	{
		Renderer2D::BeginScene(viewProjection);

		for (auto& dc : s_Data.DrawList)
		{
			Renderer::DrawAABB(dc.Mesh, dc.Transform);
		}

		Renderer2D::EndScene();
	}

	Renderer::EndRenderPass();
}

void SceneRenderer::CompositePass()
{
	Renderer::BeginRenderPass(s_Data.CompositePass);

	s_Data.CompositeShader->Bind();
	s_Data.CompositeShader->SetInt("u_TextureSamples", s_Data.GeoPass->GetSpecification().TargetFramebuffer->GetSpecification().Samples);
	s_Data.GeoPass->GetSpecification().TargetFramebuffer->BindTexture();

	Renderer::SubmitFullscreenQuad(nullptr);

	Renderer::EndRenderPass();
}

void SceneRenderer::FlushDrawList()
{
	LD_CORE_ASSERT(!s_Data.ActiveScene, "");

	GeometryPass();
	CompositePass();

	s_Data.DrawList.clear();
	s_Data.SceneData = {};
}

Ref<Texture2D> SceneRenderer::GetFinalColourBuffer()
{
	LD_CORE_ASSERT(false, "Not implemented");

	return nullptr;
}

Ref<RenderPass> SceneRenderer::GetFinalRenderPass()
{
	return s_Data.CompositePass;
}

uint32_t SceneRenderer::GetFinalColourBufferRendererID()
{
	return s_Data.CompositePass->GetSpecification().TargetFramebuffer->GetColourAttachmentRendererID();
}

SceneRendererOptions& SceneRenderer::GetOptions()
{
	return s_Data.Options;
}