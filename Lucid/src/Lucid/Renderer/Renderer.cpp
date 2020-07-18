#pragma once

#include "ldpch.h"

#include <glad/glad.h>

#include "Renderer.h"

#include "Lucid/Renderer/SceneRenderer.h"
#include "Lucid/Renderer/Renderer2D.h"

struct RendererData
{
	Ref<RenderPass> m_ActiveRenderPass;
	RenderCommandQueue m_CommandQueue;
	Ref<ShaderLibrary> m_ShaderLibrary;
	Ref<VertexArray> m_FullscreenQuadVertexArray;
};

static RendererData s_Data;

static void GLAPIENTRY OpenGLErrorLog(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	switch (severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:
		{
			LD_CORE_ERROR("[OpenGL Debug HIGH] {0}", message);
			LD_CORE_ASSERT(false, "GL_DEBUG_SEVERITY_HIGH");

			break;
		}
		case GL_DEBUG_SEVERITY_MEDIUM:
		{
			LD_CORE_WARN("[OpenGL Debug MEDIUM] {0}", message);

			break;
		}
		case GL_DEBUG_SEVERITY_LOW:
		{
			LD_CORE_INFO("[OpenGL Debug LOW] {0}", message);

			break;
		}
		case GL_DEBUG_SEVERITY_NOTIFICATION:
		{
			//LD_CORE_TRACE("[OpenGL Debug NOTIFICATION] {0}", message);

			break;
		}
	}
}

// Initalize OpenGL operations
static void InitOpenGL()
{
	#pragma region Initalize OpenGL Renderer

	// Set error logging
	glDebugMessageCallback(OpenGLErrorLog, nullptr);
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

	// Generate and bind vertex array
	unsigned int vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Enable depth-testing by default and set winding-order of indices
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);

	// Enable blending and set blending operate (over blending)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Enable multisample and stencil test
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_STENCIL_TEST);

	// Set and log renderer capabilities
	auto& caps = RendererCapabilities::GetCapabilities();

	caps.Vendor = (const char*)glGetString(GL_VENDOR);
	caps.Renderer = (const char*)glGetString(GL_RENDERER);
	caps.Version = (const char*)glGetString(GL_VERSION);

	LD_CORE_DEBUG("{0}", caps.Vendor);
	LD_CORE_DEBUG("{0}", caps.Renderer);
	LD_CORE_DEBUG("{0}", caps.Version);

	glGetIntegerv(GL_MAX_SAMPLES, &caps.MaxSamples);
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &caps.MaxAnisotropy);

	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &caps.MaxTextureUnits);

	GLenum error = glGetError();

	// Ensure OpenGL successfully initialized
	while (error != GL_NO_ERROR)
	{
		LD_CORE_ERROR("OpenGL Error {0}", error);

		error = glGetError();
	}

	#pragma endregion
}

void Renderer::Init()
{
	s_Data.m_ShaderLibrary = Ref<ShaderLibrary>::Create();

	// Submit OpenGL initialization to renderer command queue
	Renderer::Submit([]() { InitOpenGL(); });

	Renderer::GetShaderLibrary()->Load("assets/shaders/Scene.glsl");

	SceneRenderer::Init();

	#pragma region Create Fullscreen Quad

	float x = -1;
	float y = -1;

	float width = 2;
	float height = 2;

	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoord;
	};

	QuadVertex* data = new QuadVertex[4];

	data[0].Position = glm::vec3(x, y, 0.1f);
	data[0].TexCoord = glm::vec2(0, 0);

	data[1].Position = glm::vec3(x + width, y, 0.1f);
	data[1].TexCoord = glm::vec2(1, 0);

	data[2].Position = glm::vec3(x + width, y + height, 0.1f);
	data[2].TexCoord = glm::vec2(1, 1);

	data[3].Position = glm::vec3(x, y + height, 0.1f);
	data[3].TexCoord = glm::vec2(0, 1);

	s_Data.m_FullscreenQuadVertexArray = VertexArray::Create();

	auto quadVB = VertexBuffer::Create(data, 4 * sizeof(QuadVertex));

	quadVB->SetLayout
	({
		{ ShaderDataType::Float3, "a_Position" },
		{ ShaderDataType::Float2, "a_TexCoord" }
	});

	uint32_t indices[6] = { 0, 1, 2, 2, 3, 0, };

	auto quadIB = IndexBuffer::Create(indices, 6 * sizeof(uint32_t));

	s_Data.m_FullscreenQuadVertexArray->AddVertexBuffer(quadVB);
	s_Data.m_FullscreenQuadVertexArray->SetIndexBuffer(quadIB);

	#pragma endregion

	Renderer2D::Init();
}

void Renderer::Clear()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void Renderer::Clear(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer::SetClearColour(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
}

void Renderer::SetLineThickness(float thickness)
{
	glLineWidth(thickness);
}

void Renderer::DrawIndexed(uint32_t count, PrimitiveType type, bool depthTest)
{
	Renderer::Submit([=]()
	{
		if (!depthTest)
		{
			glDisable(GL_DEPTH_TEST);
		}

		GLenum glPrimitiveType = 0;

		switch (type)
		{
			case PrimitiveType::Triangles:
			{
				glPrimitiveType = GL_TRIANGLES;

				break;
			}
			case PrimitiveType::Lines:
			{
				glPrimitiveType = GL_LINES;

				break;
			}
		}

		glDrawElements(glPrimitiveType, count, GL_UNSIGNED_INT, nullptr);

		if (!depthTest)
		{
			glEnable(GL_DEPTH_TEST);
		}
	});
}

Ref<ShaderLibrary> Renderer::GetShaderLibrary()
{
	return s_Data.m_ShaderLibrary;
}

// Executes all render commands (in sequential order) that are waiting in the render command queue
void Renderer::ExecuteRenderCommands()
{
	s_Data.m_CommandQueue.Execute();
}

// Retrieves the current state of the render command queue
RenderCommandQueue& Renderer::GetRenderCommandQueue()
{
	return s_Data.m_CommandQueue;
}

void Renderer::BeginRenderPass(Ref<RenderPass> renderPass, bool clear)
{
	LD_CORE_ASSERT(renderPass, "Render pass cannot be null!");

	s_Data.m_ActiveRenderPass = renderPass;

	renderPass->GetSpecification().TargetFramebuffer->Bind();

	if (clear)
	{
		const glm::vec4& clearColour = renderPass->GetSpecification().TargetFramebuffer->GetSpecification().ClearColour;

		Renderer::Submit([=]()
		{
			Renderer::Clear(clearColour.r, clearColour.g, clearColour.b, clearColour.a);
		});
	}
}

void Renderer::EndRenderPass()
{
	LD_CORE_ASSERT(s_Data.m_ActiveRenderPass, "No active render pass!");

	s_Data.m_ActiveRenderPass->GetSpecification().TargetFramebuffer->Unbind();
	s_Data.m_ActiveRenderPass = nullptr;
}

void Renderer::SubmitQuad(Ref<MaterialInstance> material, const glm::mat4& transform)
{
	bool depthTest = true;

	if (material)
	{
		material->Bind();
		depthTest = material->GetFlag(MaterialFlag::DepthTest);

		auto shader = material->GetShader();
		shader->SetMat4("u_Transform", transform);
	}

	s_Data.m_FullscreenQuadVertexArray->Bind();
	Renderer::DrawIndexed(6, PrimitiveType::Triangles, depthTest);
}

void Renderer::SubmitFullscreenQuad(Ref<MaterialInstance> material)
{
	bool depthTest = true;

	if (material)
	{
		material->Bind();
		depthTest = material->GetFlag(MaterialFlag::DepthTest);
	}

	s_Data.m_FullscreenQuadVertexArray->Bind();
	Renderer::DrawIndexed(6, PrimitiveType::Triangles, depthTest);
}

void Renderer::SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform, Ref<MaterialInstance> overrideMaterial)
{
	mesh->m_VertexArray->Bind();

	const auto& materials = mesh->GetMaterials();

	for (Submesh& submesh : mesh->m_Submeshes)
	{
		// Material
		auto material = overrideMaterial ? overrideMaterial : materials[submesh.MaterialIndex];
		auto shader = material->GetShader();
		material->Bind();

		shader->SetMat4("u_Transform", transform * submesh.Transform);

		Renderer::Submit([submesh, material]()
		{
			if (material->GetFlag(MaterialFlag::DepthTest))
			{
				glEnable(GL_DEPTH_TEST);
			}
			else
			{
				glDisable(GL_DEPTH_TEST);
			}

			glDrawElementsBaseVertex(GL_TRIANGLES, submesh.IndexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * submesh.BaseIndex), submesh.BaseVertex);
		});
	}
}

void Renderer::DrawAABB(const AABB& aabb, const glm::mat4& transform, const glm::vec4& colour)
{
	glm::vec4 min = { aabb.Min.x, aabb.Min.y, aabb.Min.z, 1.0f };
	glm::vec4 max = { aabb.Max.x, aabb.Max.y, aabb.Max.z, 1.0f };

	glm::vec4 corners[8] =
	{
		transform * glm::vec4 { aabb.Min.x, aabb.Min.y, aabb.Max.z, 1.0f },
		transform * glm::vec4 { aabb.Min.x, aabb.Max.y, aabb.Max.z, 1.0f },
		transform * glm::vec4 { aabb.Max.x, aabb.Max.y, aabb.Max.z, 1.0f },
		transform * glm::vec4 { aabb.Max.x, aabb.Min.y, aabb.Max.z, 1.0f },

		transform * glm::vec4 { aabb.Min.x, aabb.Min.y, aabb.Min.z, 1.0f },
		transform * glm::vec4 { aabb.Min.x, aabb.Max.y, aabb.Min.z, 1.0f },
		transform * glm::vec4 { aabb.Max.x, aabb.Max.y, aabb.Min.z, 1.0f },
		transform * glm::vec4 { aabb.Max.x, aabb.Min.y, aabb.Min.z, 1.0f }
	};

	for (uint32_t i = 0; i < 4; i++)
	{
		Renderer2D::DrawLine(corners[i], corners[(i + 1) % 4], colour);
	}

	for (uint32_t i = 0; i < 4; i++)
	{
		Renderer2D::DrawLine(corners[i + 4], corners[((i + 1) % 4) + 4], colour);
	}

	for (uint32_t i = 0; i < 4; i++)
	{
		Renderer2D::DrawLine(corners[i], corners[i + 4], colour);
	}
}

void Renderer::DrawAABB(Ref<Mesh> mesh, const glm::mat4& transform, const glm::vec4& colour)
{
	for (Submesh& submesh : mesh->m_Submeshes)
	{
		auto& aabb = submesh.BoundingBox;
		auto aabbTransform = transform * submesh.Transform;

		DrawAABB(aabb, aabbTransform);
	}
}