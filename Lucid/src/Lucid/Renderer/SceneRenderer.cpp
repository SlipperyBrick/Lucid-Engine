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

	Ref<Shader> LightingShader;
	Ref<Shader> DualDepthPeelInitShader;
	Ref<Shader> DualDepthPeelShader;
	Ref<Shader> DualDepthPeelBlendShader;
	Ref<Shader> DualDepthPeelCompositeShader;
	Ref<Shader> CompositeShader;
	Ref<Shader> EditorShader;

	Ref<RenderPass> GeometryPass;
	Ref<RenderPass> LightingPass;
	Ref<RenderPass> TransparencyPass;
	Ref<RenderPass> CompositePass;
	Ref<RenderPass> EditorPass;

	Ref<Framebuffer> TransparencyComposite;

	struct DrawCommand
	{
		Ref<Mesh> Mesh;
		Ref<MaterialInstance> Material;

		glm::mat4 Transform;
	};

	std::vector<DrawCommand> MeshDrawList;
	std::vector<DrawCommand> SelectedMeshDrawList;
	std::vector<DrawCommand> TransparentMeshDrawList;
	std::vector<DrawCommand> SelectedTransparentMeshDrawList;

	Ref<MaterialInstance> DualDepthPeelInit;
	Ref<MaterialInstance> DualDepthPeel;
	Ref<MaterialInstance> DualDepthPeelBlend;
	Ref<MaterialInstance> DualDepthPeelComposite;

	Ref<MaterialInstance> GridMaterial;
	Ref<MaterialInstance> OutlineMaterial;

	SceneRendererOptions Options;

	glm::vec2 ViewportSize;
};

static SceneRendererData s_Data;

// Initialises scene renderer by setting up all required framebuffers and framebuffer textures and render passes
void SceneRenderer::Init()
{
	#pragma region Geometry Pass

	// Geometry pass
	RenderPassSpecification geoRenderPassSpec;
	FramebufferSpecification geoFramebufferSpec;
	geoFramebufferSpec.Width = 1280;
	geoFramebufferSpec.Height = 720;

	// Position texture
	FramebufferTextureSpecification positionTexture;
	positionTexture.TextureUsage = FramebufferTextureUsage::COLOUR;
	positionTexture.Format = FramebufferTextureFormat::RGBA16F;

	// Normal texture
	FramebufferTextureSpecification normalTexture;
	normalTexture.TextureUsage = FramebufferTextureUsage::COLOUR;
	normalTexture.Format = FramebufferTextureFormat::RGBA16F;

	// Diffuse texture
	FramebufferTextureSpecification diffuseTexture;
	diffuseTexture.TextureUsage = FramebufferTextureUsage::COLOUR;
	diffuseTexture.Format = FramebufferTextureFormat::RGBA16F;

	// Specular/Gloss texture
	FramebufferTextureSpecification specularGlossTexture;
	specularGlossTexture.TextureUsage = FramebufferTextureUsage::COLOUR;
	specularGlossTexture.Format = FramebufferTextureFormat::RGBA16F;

	// Depth texture
	FramebufferTextureSpecification geoDepthTexture;
	geoDepthTexture.TextureUsage = FramebufferTextureUsage::DEPTH;
	geoDepthTexture.Format = FramebufferTextureFormat::DEPTH24STENCIL8;

	geoFramebufferSpec.Attach(positionTexture, 0);
	geoFramebufferSpec.Attach(normalTexture, 1);
	geoFramebufferSpec.Attach(diffuseTexture, 2);
	geoFramebufferSpec.Attach(specularGlossTexture, 3);
	geoFramebufferSpec.Attach(geoDepthTexture, 4);

	geoRenderPassSpec.TargetFramebuffer = Framebuffer::Create(geoFramebufferSpec);
	s_Data.GeometryPass = RenderPass::Create(geoRenderPassSpec);

	#pragma endregion

	#pragma region Lighting Pass

	// Lighting pass
	RenderPassSpecification lightingRenderPassSpec;
	FramebufferSpecification lightingFramebufferSpec;
	lightingFramebufferSpec.Width = 1280;
	lightingFramebufferSpec.Height = 720;

	// Light accumulation texture
	FramebufferTextureSpecification lightAccumulationTexture;
	lightAccumulationTexture.TextureUsage = FramebufferTextureUsage::COLOUR;
	lightAccumulationTexture.Format = FramebufferTextureFormat::RGBA16F;

	// Depth texture
	FramebufferTextureSpecification lightDepthTexture;
	lightDepthTexture.TextureUsage = FramebufferTextureUsage::DEPTH;
	lightDepthTexture.Format = FramebufferTextureFormat::DEPTH24STENCIL8;

	lightingFramebufferSpec.Attach(lightAccumulationTexture, 0);
	lightingFramebufferSpec.Attach(lightDepthTexture, 1);

	lightingRenderPassSpec.TargetFramebuffer = Framebuffer::Create(lightingFramebufferSpec);
	s_Data.LightingPass = RenderPass::Create(lightingRenderPassSpec);

	s_Data.LightingShader = Shader::Create("assets/shaders/Lighting.glsl");

	#pragma endregion

	#pragma region Transparency Pass

	// Transparency pass
	RenderPassSpecification transparencyRenderPassSpec;
	FramebufferSpecification transparencyFramebufferSpec;
	transparencyFramebufferSpec.Width = 1280;
	transparencyFramebufferSpec.Height = 720;

	// Fragment depth texture
	FramebufferTextureSpecification fragDepthTexture;
	fragDepthTexture.TextureUsage = FramebufferTextureUsage::COLOUR;
	fragDepthTexture.TextureType = FramebufferTextureType::TEXRECT;
	fragDepthTexture.Format = FramebufferTextureFormat::RG16F_RECT;

	// Front texture
	FramebufferTextureSpecification frontTexture;
	frontTexture.TextureUsage = FramebufferTextureUsage::COLOUR;
	frontTexture.TextureType = FramebufferTextureType::TEXRECT;
	frontTexture.Format = FramebufferTextureFormat::RGBA8F_RECT;

	// Back texture
	FramebufferTextureSpecification backTexture;
	backTexture.TextureUsage = FramebufferTextureUsage::COLOUR;
	backTexture.TextureType = FramebufferTextureType::TEXRECT;
	backTexture.Format = FramebufferTextureFormat::RGBA8F_RECT;

	// Colour blend texture
	FramebufferTextureSpecification colourBlendTexture;
	colourBlendTexture.TextureUsage = FramebufferTextureUsage::COLOUR;
	colourBlendTexture.TextureType = FramebufferTextureType::TEXRECT;
	colourBlendTexture.Format = FramebufferTextureFormat::RGB8F_RECT;

	transparencyFramebufferSpec.Attach(fragDepthTexture, 0);
	transparencyFramebufferSpec.Attach(fragDepthTexture, 3);
	transparencyFramebufferSpec.Attach(frontTexture, 1);
	transparencyFramebufferSpec.Attach(frontTexture, 4);
	transparencyFramebufferSpec.Attach(backTexture, 2);
	transparencyFramebufferSpec.Attach(backTexture, 5);
	transparencyFramebufferSpec.Attach(colourBlendTexture, 6);

	transparencyRenderPassSpec.TargetFramebuffer = Framebuffer::Create(transparencyFramebufferSpec);
	s_Data.TransparencyPass = RenderPass::Create(transparencyRenderPassSpec);

	FramebufferSpecification transparencyCompositeFramebufferSpec;
	transparencyCompositeFramebufferSpec.Width = 1280;
	transparencyCompositeFramebufferSpec.Height = 720;

	FramebufferTextureSpecification transparenyCompositeTexture;
	transparenyCompositeTexture.TextureUsage = FramebufferTextureUsage::COLOUR;
	transparenyCompositeTexture.TextureType = FramebufferTextureType::TEX2D;
	transparenyCompositeTexture.Format = FramebufferTextureFormat::RGBA8;

	transparencyCompositeFramebufferSpec.Attach(transparenyCompositeTexture, 0);

	s_Data.TransparencyComposite = Framebuffer::Create(transparencyCompositeFramebufferSpec);
	
	s_Data.DualDepthPeelInitShader = Shader::Create("assets/shaders/DualDepthPeelInit.glsl");
	s_Data.DualDepthPeelInit = MaterialInstance::Create(Material::Create(s_Data.DualDepthPeelInitShader));
	s_Data.DualDepthPeelInit->SetFlag(MaterialFlag::DepthTest, false);
	
	s_Data.DualDepthPeelShader = Shader::Create("assets/shaders/DualDepthPeel.glsl");
	s_Data.DualDepthPeel = MaterialInstance::Create(Material::Create(s_Data.DualDepthPeelShader));
	s_Data.DualDepthPeel->SetFlag(MaterialFlag::DepthTest, false);
	
	s_Data.DualDepthPeelBlendShader = Shader::Create("assets/shaders/DualDepthPeelBlend.glsl");
	s_Data.DualDepthPeelBlend = MaterialInstance::Create(Material::Create(s_Data.DualDepthPeelBlendShader));

	s_Data.DualDepthPeelCompositeShader = Shader::Create("assets/shaders/DualDepthPeelComposite.glsl");
	s_Data.DualDepthPeelComposite = MaterialInstance::Create(Material::Create(s_Data.DualDepthPeelCompositeShader));

	#pragma endregion

	#pragma region Composite Pass

	// Composite pass
	RenderPassSpecification compRenderPassSpec;
	FramebufferSpecification compFramebufferSpec;
	compFramebufferSpec.Width = 1280;
	compFramebufferSpec.Height = 720;

	// Composite texture
	FramebufferTextureSpecification compositeTexture;
	compositeTexture.TextureUsage = FramebufferTextureUsage::COLOUR;
	compositeTexture.Format = FramebufferTextureFormat::RGBA16F;

	// Editor composite texture
	FramebufferTextureSpecification editorCompositeTexture;
	editorCompositeTexture.TextureUsage = FramebufferTextureUsage::COLOUR;
	editorCompositeTexture.Format = FramebufferTextureFormat::RGBA16F;

	// Depth texture
	FramebufferTextureSpecification compDepthTexture;
	compDepthTexture.TextureUsage = FramebufferTextureUsage::DEPTH;
	compDepthTexture.Format = FramebufferTextureFormat::DEPTH24STENCIL8;

	compFramebufferSpec.Attach(compositeTexture, 0);
	compFramebufferSpec.Attach(editorCompositeTexture, 1);
	compFramebufferSpec.Attach(compDepthTexture, 2);

	compRenderPassSpec.TargetFramebuffer = Framebuffer::Create(compFramebufferSpec);
	s_Data.CompositePass = RenderPass::Create(compRenderPassSpec);

	s_Data.CompositeShader = Shader::Create("assets/shaders/Composite.glsl");

	#pragma endregion

	#pragma region Editor Pass

	// Editor pass
	RenderPassSpecification editorRenderPass;
	FramebufferSpecification editorFramebufferSpec;
	editorFramebufferSpec.Width = 1280;
	editorFramebufferSpec.Height = 720;

	// Editor texture
	FramebufferTextureSpecification editorTexture;
	editorTexture.TextureUsage = FramebufferTextureUsage::COLOUR;
	editorTexture.Format = FramebufferTextureFormat::RGBA16F;

	// Depth texture
	FramebufferTextureSpecification editorDepthTexture;
	editorDepthTexture.TextureUsage = FramebufferTextureUsage::DEPTH;
	editorDepthTexture.Format = FramebufferTextureFormat::DEPTH24STENCIL8;

	editorFramebufferSpec.Attach(editorTexture, 0);
	editorFramebufferSpec.Attach(editorDepthTexture, 1);

	editorRenderPass.TargetFramebuffer = Framebuffer::Create(editorFramebufferSpec);
	s_Data.EditorPass = RenderPass::Create(editorRenderPass);

	s_Data.EditorShader = Shader::Create("assets/shaders/Editor.glsl");

	#pragma endregion

	// Grid
	auto gridShader = Shader::Create("assets/shaders/Grid.glsl");
	s_Data.GridMaterial = MaterialInstance::Create(Material::Create(gridShader));
	s_Data.GridMaterial->SetFlag(MaterialFlag::DepthTest, true);

	float gridScale = 16.025f;
	float gridSize = 0.025f;

	s_Data.GridMaterial->Set("u_Scale", gridScale);
	s_Data.GridMaterial->Set("u_Res", gridSize);

	// Outline
	auto outlineShader = Shader::Create("assets/shaders/Outline.glsl");
	s_Data.OutlineMaterial = MaterialInstance::Create(Material::Create(outlineShader));
	s_Data.OutlineMaterial->SetFlag(MaterialFlag::DepthTest, false);
}

// Calls Framebuffer::Resize for resizing all framebuffers when the viewport changes size
void SceneRenderer::SetViewportSize(uint32_t width, uint32_t height)
{
	s_Data.GeometryPass->GetSpecification().TargetFramebuffer->Resize(width, height);
	s_Data.LightingPass->GetSpecification().TargetFramebuffer->Resize(width, height);
	s_Data.TransparencyPass->GetSpecification().TargetFramebuffer->Resize(width, height);
	s_Data.CompositePass->GetSpecification().TargetFramebuffer->Resize(width, height);
	s_Data.EditorPass->GetSpecification().TargetFramebuffer->Resize(width, height);

	s_Data.TransparencyComposite->Resize(width, height);

	s_Data.ViewportSize = { width, height };
}

// Sets the active scene, scene camera and scenes light environment
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

// Submits a mesh to its corresponding draw list
void SceneRenderer::SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform, Ref<MaterialInstance> overrideMaterial, bool transparency)
{
	// Culling, sorting, can be done here
	
	if (transparency)
	{
		s_Data.TransparentMeshDrawList.push_back({ mesh, nullptr, transform });
	}
	else
	{
		s_Data.MeshDrawList.push_back({ mesh, overrideMaterial, transform });
	}
}

// Submits a selected mesh to its corresponding draw list
void SceneRenderer::SubmitSelectedMesh(Ref<Mesh> mesh, const glm::mat4& transform, bool transparency)
{
	if (transparency)
	{
		s_Data.SelectedTransparentMeshDrawList.push_back({ mesh, nullptr, transform });
	}
	else
	{
		s_Data.SelectedMeshDrawList.push_back({ mesh, nullptr, transform });
	}
}

// Geometry pass to create g-buffer
void SceneRenderer::GeometryPass()
{
	Renderer::BeginRenderPass(s_Data.GeometryPass);

	auto viewProjection = s_Data.SceneData.SceneCamera.Camera.GetProjectionMatrix() * s_Data.SceneData.SceneCamera.ViewMatrix;
	glm::vec3 cameraPosition = glm::inverse(s_Data.SceneData.SceneCamera.ViewMatrix)[3];

	// Render non-selected meshes
	for (auto& dc : s_Data.MeshDrawList)
	{
		auto baseMaterial = dc.Mesh->GetMaterial();
		auto shader = baseMaterial->GetShader();

		baseMaterial->Set("u_ViewProjectionMatrix", viewProjection);

		Renderer::SubmitMesh(dc.Mesh, dc.Transform);
	}

	// Render only selected meshes
	for (auto& dc : s_Data.SelectedMeshDrawList)
	{
		auto baseMaterial = dc.Mesh->GetMaterial();
		auto shader = baseMaterial->GetShader();

		baseMaterial->Set("u_ViewProjectionMatrix", viewProjection);

		Renderer::SubmitMesh(dc.Mesh, dc.Transform);
	}

	Renderer::EndRenderPass();
}

// Lighting pass that utilises g-buffer for performing per-pixel lighting
void SceneRenderer::LightingPass()
{
	Renderer::BeginRenderPass(s_Data.LightingPass);

	glm::vec3 cameraPosition = glm::inverse(s_Data.SceneData.SceneCamera.ViewMatrix)[3];

	s_Data.LightingShader->Bind();

	s_Data.LightingShader->SetVec3("u_CameraPosition", cameraPosition);

	// Bind colour attachments
	s_Data.GeometryPass->GetSpecification().TargetFramebuffer->BindColourAttachment(0, 0); // Position
	s_Data.GeometryPass->GetSpecification().TargetFramebuffer->BindColourAttachment(1, 1); // Normals
	s_Data.GeometryPass->GetSpecification().TargetFramebuffer->BindColourAttachment(2, 2); // Diffuse
	s_Data.GeometryPass->GetSpecification().TargetFramebuffer->BindColourAttachment(3, 3); // Specular

	// Directional light
	s_Data.LightingShader->SetFloat("r_DirectionalLight.Brightness", s_Data.SceneData.DirLight.Brightness);
	s_Data.LightingShader->SetVec3("r_DirectionalLight.Direction", s_Data.SceneData.DirLight.Direction);
	s_Data.LightingShader->SetVec3("r_DirectionalLight.Diffuse", s_Data.SceneData.DirLight.Diffuse);
	s_Data.LightingShader->SetVec3("r_DirectionalLight.Ambient", s_Data.SceneData.DirLight.Ambient);
	s_Data.LightingShader->SetVec3("r_DirectionalLight.Specular", s_Data.SceneData.DirLight.Specular);

	// Point lights
	int it = 0;

	// Iterate over all point lights
	for (it; it < 100; it++)
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

void SceneRenderer::TransparencyPass()
{
	Renderer::BeginRenderPass(s_Data.TransparencyPass);

	const float MAX_DEPTH = 1.0f;

	int currentAttachment = 0;
	int previousAttachment = 0;
	int depthAttachment = 0;
	int backColourAttachment = 0;

	auto viewProjection = s_Data.SceneData.SceneCamera.Camera.GetProjectionMatrix() * s_Data.SceneData.SceneCamera.ViewMatrix;

	Renderer::Submit([]()
	{
		glEnable(GL_BLEND);
	});

	// Render targets 1 and 2 store the front and back colours
	// Clear both textures to black and use max blending to filter the written colour
	// At most, one front colour and one back colour can be written each pass
	s_Data.TransparencyPass->GetSpecification().TargetFramebuffer->DrawBuffers(1, 2);
	s_Data.TransparencyPass->GetSpecification().TargetFramebuffer->Clear(0.0f, 0.0f, 0.0f, 0.0f);

	// Render target 0 stores the minimum and maximum depth
	s_Data.TransparencyPass->GetSpecification().TargetFramebuffer->DrawBuffers(0);

	// Clear with negative max depth
	s_Data.TransparencyPass->GetSpecification().TargetFramebuffer->Clear(-MAX_DEPTH, -MAX_DEPTH, 0.0f, 0.0f);

	// Enable max blending
	Renderer::Submit([]()
	{
		glBlendEquation(GL_MAX);
	});

	// Render non-selected transparent meshes with DepthPeelingInit
	for (auto& dc : s_Data.TransparentMeshDrawList)
	{
		auto material = s_Data.DualDepthPeelInit;

		material->Set("u_ViewProjectionMatrix", viewProjection);

		Renderer::SubmitMesh(dc.Mesh, dc.Transform, material);
	}

	// Render only selected transparent meshes with DepthPeelingInit
	for (auto& dc : s_Data.SelectedTransparentMeshDrawList)
	{
		auto material = s_Data.DualDepthPeelInit;

		material->Set("u_ViewProjectionMatrix", viewProjection);

		Renderer::SubmitMesh(dc.Mesh, dc.Transform, material);
	}

	// Bind our back colour texture
	s_Data.TransparencyPass->GetSpecification().TargetFramebuffer->DrawBuffers(6);

	// Clear  back colour buffer
	s_Data.TransparencyPass->GetSpecification().TargetFramebuffer->Clear(0.0f, 0.0f, 0.0f, 0.0f);

	// Depth peeling layers
	// - Depth texture is attached to attachment point 0 and 3
	// - Front texture is attached to attachment point 1 and 4
	// - Back texture is attached to attachment point 2 and 5

	int depthAttachments[2] = { 0, 3 };
	int frontAttachments[2] = { 1, 4 };
	int backAttachments[2] = { 2, 5 };

	for (int i = 1; i < s_Data.Options.LayerPeels; i++)
	{
		// Always provide us the modulo of the current layer and 2 (currentAttachment will always be 0 or 1)
		currentAttachment = i % 2;

		// Previous attachment is 1 behind the currentAttachment (previousAttachment will always be 0 or 1)
		previousAttachment = 1 - currentAttachment;

		// Alternate between depth texture 0 and 3
		depthAttachment = currentAttachment * 3;

		// Alternate between our two front texture attachments
		s_Data.TransparencyPass->GetSpecification().TargetFramebuffer->DrawBuffers(depthAttachment + 1, depthAttachment + 2);

		// Clear front colour buffer
		s_Data.TransparencyPass->GetSpecification().TargetFramebuffer->Clear(0.0f, 0.0f, 0.0f, 0.0f);

		// Alternate between our two depth textures
		s_Data.TransparencyPass->GetSpecification().TargetFramebuffer->DrawBuffers(depthAttachment);

		// Clear with negative max depth
		s_Data.TransparencyPass->GetSpecification().TargetFramebuffer->Clear(-MAX_DEPTH, -MAX_DEPTH, 0.0f, 0.0f);

		// Draw to depth and front, back colour buffers
		s_Data.TransparencyPass->GetSpecification().TargetFramebuffer->DrawBuffers(depthAttachment, depthAttachment + 1, depthAttachment + 2);

		// Enable max blending
		Renderer::Submit([]()
		{
			glBlendEquation(GL_MAX);
		});

		// Bind our depth texture to texture unit 0 (alternate between both depth attachments)
		s_Data.TransparencyPass->GetSpecification().TargetFramebuffer->BindColourAttachment(depthAttachments[previousAttachment], 0);

		// Bind our front texture to texture unit 1 (alternate between our first and second front textures)
		s_Data.TransparencyPass->GetSpecification().TargetFramebuffer->BindColourAttachment(frontAttachments[previousAttachment], 1);

		Renderer::Submit([]()
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		});

		// Render non-selected transparent meshes with DepthPeeling
		for (auto& dc : s_Data.TransparentMeshDrawList)
		{
			auto material = s_Data.DualDepthPeel;

			material->Set("u_ViewProjectionMatrix", viewProjection);
			material->Set("u_Alpha", 0.25f);

			Renderer::SubmitMesh(dc.Mesh, dc.Transform, material);
		}

		// Render only selected transparent meshes with DepthPeeling
		for (auto& dc : s_Data.SelectedTransparentMeshDrawList)
		{
			auto material = s_Data.DualDepthPeel;

			material->Set("u_ViewProjectionMatrix", viewProjection);
			material->Set("u_Alpha", 0.25f);

			Renderer::SubmitMesh(dc.Mesh, dc.Transform, material);
		}

		// Full-screen pass to alpha-blend the back texture (this is written to our intermediate blender texture)
		s_Data.TransparencyPass->GetSpecification().TargetFramebuffer->DrawBuffers(6);

		// Enable over blending
		Renderer::Submit([]()
		{
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		});

		// Bind our back texture to texture unit 0 (alternate between our first and last back texture)
		s_Data.TransparencyPass->GetSpecification().TargetFramebuffer->BindColourAttachment(backAttachments[currentAttachment], 0);

		// Render alpha-blended results to full-screen quad
		Renderer::SubmitFullscreenQuad(s_Data.DualDepthPeelBlend);
	}

	// Disable blending
	Renderer::Submit([]()
	{
		glDisable(GL_BLEND);
	});

	// Bind our composite framebuffer here
	s_Data.TransparencyComposite->Bind();

	// Bind our depth attachment to texture unit 0 (alternate between our first and last depth texture)
	s_Data.TransparencyPass->GetSpecification().TargetFramebuffer->BindColourAttachment(depthAttachments[currentAttachment], 0);

	// Bind our front attachments to texture unit 1
	s_Data.TransparencyPass->GetSpecification().TargetFramebuffer->BindColourAttachment(frontAttachments[currentAttachment], 1);

	// Bind our colour blend texture to texture unit 2
	s_Data.TransparencyPass->GetSpecification().TargetFramebuffer->BindColourAttachment(6, 2);

	// Render final results to full-screen quad
	Renderer::SubmitFullscreenQuad(s_Data.DualDepthPeelComposite);
	Renderer::EndRenderPass();
}

void SceneRenderer::CompositePass()
{
	Renderer::BeginRenderPass(s_Data.CompositePass);

	// Disable blending
	Renderer::Submit([]()
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
	});

	s_Data.CompositeShader->Bind();
	s_Data.CompositeShader->SetFloat("u_Exposure", s_Data.SceneData.SceneCamera.Camera.GetExposure());

	// Bind our light pass texture
	s_Data.LightingPass->GetSpecification().TargetFramebuffer->BindColourAttachment();

	// Check SceneRenderer options for showing g-buffer textures individually
	if (GetOptions().ShowPosition)
	{
		s_Data.GeometryPass->GetSpecification().TargetFramebuffer->BindColourAttachment(0, 0);
	}

	if (GetOptions().ShowNormal)
	{
		s_Data.GeometryPass->GetSpecification().TargetFramebuffer->BindColourAttachment(1, 0);
	}

	if (GetOptions().ShowDiffuse)
	{
		s_Data.GeometryPass->GetSpecification().TargetFramebuffer->BindColourAttachment(2, 0);
	}

	if (GetOptions().ShowSpecular)
	{
		s_Data.GeometryPass->GetSpecification().TargetFramebuffer->BindColourAttachment(3, 0);
	}

	// Bind our front, back and colour blender textures from depth-peeled pass
	s_Data.TransparencyPass->GetSpecification().TargetFramebuffer->BindColourAttachment(0, 1);
	s_Data.TransparencyPass->GetSpecification().TargetFramebuffer->BindColourAttachment(1, 2);
	s_Data.TransparencyPass->GetSpecification().TargetFramebuffer->BindColourAttachment(6, 3);

	Renderer::SubmitFullscreenQuad(nullptr);

	Renderer::EndRenderPass();
}

void SceneRenderer::EditorPass()
{
	Renderer::BeginRenderPass(s_Data.EditorPass);

	auto viewProjection = s_Data.SceneData.SceneCamera.Camera.GetProjectionMatrix() * s_Data.SceneData.SceneCamera.ViewMatrix;

	// Grid
	if (GetOptions().ShowGrid)
	{
		s_Data.GridMaterial->Set("u_ViewProjection", viewProjection);

		Renderer::SubmitQuad(s_Data.GridMaterial, glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(16.0f)));
	}

	// Bounding boxes
	if (GetOptions().ShowBoundingBoxes)
	{
		Renderer2D::BeginScene(viewProjection);

		for (auto& dc : s_Data.MeshDrawList)
		{
			Renderer::DrawAABB(dc.Mesh, dc.Transform);
		}

		Renderer2D::EndScene();
	}

	Renderer::EndRenderPass();
}

// Executes each render pass in order and flushes both mesh draw lists (transparent and non-transparent)
void SceneRenderer::FlushDrawList()
{
	LD_CORE_ASSERT(!s_Data.ActiveScene, "");

	EditorPass();
	GeometryPass();
	LightingPass();
	TransparencyPass();
	CompositePass();

	s_Data.MeshDrawList.clear();
	s_Data.SelectedMeshDrawList.clear();
	s_Data.TransparentMeshDrawList.clear();
	s_Data.SelectedTransparentMeshDrawList.clear();
	s_Data.SceneData = {};
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