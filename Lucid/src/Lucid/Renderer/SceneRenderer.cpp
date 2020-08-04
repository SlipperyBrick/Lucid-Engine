#include "ldpch.h"

#include "SceneRenderer.h"

#include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp>

#include "Lucid/Renderer/Renderer.h"
#include "Lucid/Renderer/Renderer2D.h"

#include "Lucid/ImGui/EditorLayer.h"

struct SceneRendererData
{
	const Scene* ActiveScene = nullptr;

	struct SceneInfo
	{
		SceneRendererCamera SceneCamera;

		DirectionalLight DirLight;
		LightEnvironment LightEnv;

	} SceneData;

	Ref<Shader> CompositeShader;
	Ref<Shader> LightingShader;

	Ref<RenderPass> GeometryPass;
	Ref<RenderPass> LightingPass;
	Ref<RenderPass> CompositePass;

	struct DrawCommand
	{
		Ref<Mesh> Mesh;
		Ref<MaterialInstance> Material;

		glm::mat4 Transform;
	};

	std::vector<DrawCommand> DrawList;
	std::vector<DrawCommand> SelectedMeshDrawList;

	Ref<MaterialInstance> GridMaterial;
	Ref<MaterialInstance> OutlineMaterial;

	SceneRendererOptions Options;

	glm::vec2 ViewportSize;
};

static SceneRendererData s_Data;

void SceneRenderer::Init()
{
	// Geometry pass
	FramebufferSpecification geoFramebufferSpec;
	geoFramebufferSpec.Width = 1280;
	geoFramebufferSpec.Height = 720;
	geoFramebufferSpec.Format = FramebufferFormat::RGBA16F;
	geoFramebufferSpec.BufferCount = 4;
	geoFramebufferSpec.Samples = 1;
	geoFramebufferSpec.ClearColour = { 0.0f, 0.0f, 0.0f, 1.0f };

	RenderPassSpecification geoRenderPassSpec;
	geoRenderPassSpec.TargetFramebuffer = Framebuffer::Create(geoFramebufferSpec);
	s_Data.GeometryPass = RenderPass::Create(geoRenderPassSpec);

	// Lighting Pass
	FramebufferSpecification lightingFramebufferSpec;
	lightingFramebufferSpec.Width = 1280;
	lightingFramebufferSpec.Height = 720;
	lightingFramebufferSpec.Format = FramebufferFormat::RGBA16F;
	lightingFramebufferSpec.BufferCount = 1;
	lightingFramebufferSpec.Samples = 1;
	lightingFramebufferSpec.ClearColour = { 0.0f, 0.0f, 0.0f, 1.0f };

	RenderPassSpecification lightingRenderPassSpec;
	lightingRenderPassSpec.TargetFramebuffer = Framebuffer::Create(lightingFramebufferSpec);
	s_Data.LightingPass = RenderPass::Create(lightingRenderPassSpec);

	s_Data.LightingShader = Shader::Create("assets/shaders/Lighting.glsl");

	// Composite pass
	FramebufferSpecification compFramebufferSpec;
	compFramebufferSpec.Width = 1280;
	compFramebufferSpec.Height = 720;
	compFramebufferSpec.Format = FramebufferFormat::RGBA8;
	compFramebufferSpec.BufferCount = 1;
	compFramebufferSpec.Samples = 1;
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

	// Outline
	auto outlineShader = Shader::Create("assets/shaders/Outline.glsl");

	s_Data.OutlineMaterial = MaterialInstance::Create(Material::Create(outlineShader));
	s_Data.OutlineMaterial->SetFlag(MaterialFlag::DepthTest, false);
}

void SceneRenderer::SetViewportSize(uint32_t width, uint32_t height)
{
	s_Data.GeometryPass->GetSpecification().TargetFramebuffer->Resize(width, height);
	s_Data.LightingPass->GetSpecification().TargetFramebuffer->Resize(width, height);
	s_Data.CompositePass->GetSpecification().TargetFramebuffer->Resize(width, height);

	s_Data.ViewportSize = { width, height };
}

void SceneRenderer::BeginScene(const Scene* scene, const SceneRendererCamera& camera)
{
	LD_CORE_ASSERT(!s_Data.ActiveScene, "");

	s_Data.ActiveScene = scene;

	s_Data.SceneData.SceneCamera = camera;
	s_Data.SceneData.DirLight = scene->m_Light;
	s_Data.SceneData.LightEnv = scene->m_LightEnvironment;
}

void SceneRenderer::EndScene()
{
	LD_CORE_ASSERT(s_Data.ActiveScene, "");

	s_Data.ActiveScene = nullptr;

	FlushDrawList();
}

void SceneRenderer::SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform, Ref<MaterialInstance> overrideMaterial)
{
	// Culling, sorting, can be done here

	s_Data.DrawList.push_back({ mesh, overrideMaterial, transform });
}

void SceneRenderer::SubmitSelectedMesh(Ref<Mesh> mesh, const glm::mat4& transform)
{
	s_Data.SelectedMeshDrawList.push_back({ mesh, nullptr, transform });
}

void SceneRenderer::GeometryPass()
{
	bool outline = s_Data.SelectedMeshDrawList.size() > 0;

	if (outline)
	{
		Renderer::Submit([]()
		{
			glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
		});
	}

	Renderer::BeginRenderPass(s_Data.GeometryPass);

	if (outline)
	{
		Renderer::Submit([]()
		{
			glStencilMask(0);
		});
	}

	auto viewProjection = s_Data.SceneData.SceneCamera.Camera.GetProjectionMatrix() * s_Data.SceneData.SceneCamera.ViewMatrix;
	glm::vec3 cameraPosition = glm::inverse(s_Data.SceneData.SceneCamera.ViewMatrix)[3];

	// Render meshes
	for (auto& dc : s_Data.DrawList)
	{
		auto baseMaterial = dc.Mesh->GetMaterial();
		auto shader = baseMaterial->GetShader();

		baseMaterial->Set("u_ViewProjectionMatrix", viewProjection);

		auto overrideMaterial = nullptr;
		Renderer::SubmitMesh(dc.Mesh, dc.Transform, overrideMaterial);
	}

	if (outline)
	{
		Renderer::Submit([]()
		{
			glStencilFunc(GL_ALWAYS, 1, 0xff);
			glStencilMask(0xff);
		});
	}

	for (auto& dc : s_Data.SelectedMeshDrawList)
	{
		auto baseMaterial = dc.Mesh->GetMaterial();
		auto shader = baseMaterial->GetShader();

		baseMaterial->Set("u_ViewProjectionMatrix", viewProjection);

		auto overrideMaterial = nullptr;
		Renderer::SubmitMesh(dc.Mesh, dc.Transform, overrideMaterial);
	}

	if (outline)
	{
		Renderer::Submit([]()
		{
			glStencilFunc(GL_NOTEQUAL, 1, 0xff);
			glStencilMask(0);

			glLineWidth(10);
			glEnable(GL_LINE_SMOOTH);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDisable(GL_DEPTH_TEST);
		});

		// Draw outline
		s_Data.OutlineMaterial->Set("u_ViewProjection", viewProjection);

		for (auto& dc : s_Data.SelectedMeshDrawList)
		{
			Renderer::SubmitMesh(dc.Mesh, dc.Transform, s_Data.OutlineMaterial);
		}

		Renderer::Submit([]()
		{
			glPointSize(10);
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		});

		for (auto& dc : s_Data.SelectedMeshDrawList)
		{
			Renderer::SubmitMesh(dc.Mesh, dc.Transform, s_Data.OutlineMaterial);
		}

		Renderer::Submit([]()
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glStencilMask(0xff);
			glStencilFunc(GL_ALWAYS, 1, 0xff);
			glEnable(GL_DEPTH_TEST);
		});
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

void SceneRenderer::LightingPass()
{
	Renderer::BeginRenderPass(s_Data.LightingPass);

	glm::vec3 cameraPosition = glm::inverse(s_Data.SceneData.SceneCamera.ViewMatrix)[3];

	s_Data.LightingShader->Bind();

	// Bind colour attachments
	s_Data.GeometryPass->GetSpecification().TargetFramebuffer->BindColourAttachment(0, 0); // Position
	s_Data.GeometryPass->GetSpecification().TargetFramebuffer->BindColourAttachment(1, 1); // Normals
	s_Data.GeometryPass->GetSpecification().TargetFramebuffer->BindColourAttachment(2, 2); // Albedo
	s_Data.GeometryPass->GetSpecification().TargetFramebuffer->BindColourAttachment(3, 3); // Specular

	s_Data.LightingShader->SetVec3("u_CameraPosition", cameraPosition);

	// Directional light
	s_Data.LightingShader->SetFloat("r_DirectionalLight.Brightness", s_Data.SceneData.DirLight.Brightness);
	s_Data.LightingShader->SetVec3("r_DirectionalLight.Direction", s_Data.SceneData.DirLight.Direction);
	s_Data.LightingShader->SetVec3("r_DirectionalLight.Diffuse", s_Data.SceneData.DirLight.Diffuse);
	s_Data.LightingShader->SetVec3("r_DirectionalLight.Ambient", s_Data.SceneData.DirLight.Ambient);
	s_Data.LightingShader->SetVec3("r_DirectionalLight.Specular", s_Data.SceneData.DirLight.Specular);

	// Point lights
	int it = 0;

	// Iterate over all point lights
	for (it; it < 4; it++)
	{
		auto& pointLight = s_Data.SceneData.LightEnv.PointLights[it];

		if (pointLight.Brightness == 0.0f)
		{
			break;
		}

		s_Data.LightingShader->SetVec3("r_PointLights[" + std::to_string(it) + "].Position", pointLight.Position);
		s_Data.LightingShader->SetVec3("r_PointLights[" + std::to_string(it) + "].Diffuse", pointLight.Diffuse);
		s_Data.LightingShader->SetFloat("r_PointLights[" + std::to_string(it) + "].Brightness", pointLight.Brightness);
		s_Data.LightingShader->SetFloat("r_PointLights[" + std::to_string(it) + "].Quadratic", pointLight.Quadratic);
		s_Data.LightingShader->SetVec3("r_PointLights[" + std::to_string(it) + "].Specular", pointLight.Specular);
	}

	s_Data.LightingShader->SetInt("r_PointLightCount", it);

	Renderer::SubmitFullscreenQuad(nullptr);

	Renderer::EndRenderPass();
}

void SceneRenderer::CompositePass()
{
	Renderer::BeginRenderPass(s_Data.CompositePass);

	s_Data.CompositeShader->Bind();
	s_Data.CompositeShader->SetFloat("u_Exposure", s_Data.SceneData.SceneCamera.Camera.GetExposure());
	s_Data.CompositeShader->SetInt("u_TextureSamples", s_Data.LightingPass->GetSpecification().TargetFramebuffer->GetSpecification().Samples);
	s_Data.LightingPass->GetSpecification().TargetFramebuffer->BindColourAttachment();

	// Check SceneRenderer options for showing buffers individually
	if (GetOptions().ShowPosition)
	{
		s_Data.GeometryPass->GetSpecification().TargetFramebuffer->BindColourAttachment(0, 0);
	}
	
	if (GetOptions().ShowNormal)
	{
		s_Data.GeometryPass->GetSpecification().TargetFramebuffer->BindColourAttachment(1, 0);
	}
	
	if (GetOptions().ShowAlbedo)
	{
		s_Data.GeometryPass->GetSpecification().TargetFramebuffer->BindColourAttachment(2, 0);
	}
	
	if (GetOptions().ShowSpecular)
	{
		s_Data.GeometryPass->GetSpecification().TargetFramebuffer->BindColourAttachment(3, 0);
	}

	Renderer::SubmitFullscreenQuad(nullptr);

	Renderer::EndRenderPass();
}

void SceneRenderer::FlushDrawList()
{
	LD_CORE_ASSERT(!s_Data.ActiveScene, "");

	GeometryPass();
	LightingPass();
	CompositePass();

	s_Data.DrawList.clear();
	s_Data.SelectedMeshDrawList.clear();
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