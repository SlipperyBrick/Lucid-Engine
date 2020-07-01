#pragma once

#include "ldpch.h"

#include <glad/glad.h>

#include <glm/glm.hpp>

#include "Renderer.h"

#include "Lucid/Renderer/VertexArray.h"
#include "Lucid/Renderer/SceneRenderer.h"
#include "Lucid/Renderer/Framebuffer.h"
#include "Lucid/Renderer/Renderer2D.h"

struct RendererData
{
	Ref<RenderPass> m_ActiveRenderPass;
	RenderCommandQueue m_CommandQueue;
	Scope<ShaderLibrary> m_ShaderLibrary;
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

void Renderer::Init()
{
	s_Data.m_ShaderLibrary = std::make_unique<ShaderLibrary>();

	#pragma region Initalize OpenGL Renderer

	glDebugMessageCallback(OpenGLErrorLog, nullptr);
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

	unsigned int vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glFrontFace(GL_CCW);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_MULTISAMPLE);

	auto& caps = RenderCapabilities::GetCapabilities();

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

	while (error != GL_NO_ERROR)
	{
		LD_CORE_ERROR("OpenGL Error {0}", error);

		error = glGetError();
	}

	#pragma endregion

	Renderer::GetShaderLibrary()->Load("assets/shaders/Flat.glsl");

	//SceneRenderer::Init();

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
	Renderer::Submit([]()
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	});
}

void Renderer::Clear(float r, float g, float b, float a)
{
	Renderer::Submit([=]()
	{
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	});
}

void Renderer::SetClearColour(float r, float g, float b, float a)
{
	Renderer::Submit([=]()
	{
		glClearColor(r, g, b, a);
	});
}

void Renderer::SetLineThickness(float thickness)
{
	Renderer::Submit([=]()
	{
		glLineWidth(thickness);
	});
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

const Scope<ShaderLibrary>& Renderer::GetShaderLibrary()
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

void Renderer::BeginRenderPass(const Ref<RenderPass>& renderPass, bool clear)
{
	LD_CORE_ASSERT(renderPass, "Render pass cannot be null!");

	s_Data.m_ActiveRenderPass = renderPass;

	renderPass->GetSpecification().TargetFramebuffer->Bind();

	if (clear)
	{
		const glm::vec4& clearColor = renderPass->GetSpecification().TargetFramebuffer->GetSpecification().ClearColor;

		Renderer::Submit([=]()
		{
			Renderer::Clear(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		});
	}
}

void Renderer::EndRenderPass()
{
	LD_CORE_ASSERT(s_Data.m_ActiveRenderPass, "No active render pass!");

	s_Data.m_ActiveRenderPass->GetSpecification().TargetFramebuffer->Unbind();
	s_Data.m_ActiveRenderPass = nullptr;
}