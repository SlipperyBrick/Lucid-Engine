#pragma once

#include "ldpch.h"

#include <glad/glad.h>

#include "VertexArray.h"

#include "Lucid/Renderer/Renderer.h"

static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
{
	switch (type)
	{
		case ShaderDataType::Float:
		{
			return GL_FLOAT;
		}
		case ShaderDataType::Float2:
		{
			return GL_FLOAT;
		}
		case ShaderDataType::Float3:
		{
			return GL_FLOAT;
		}
		case ShaderDataType::Float4:
		{
			return GL_FLOAT;
		}
		case ShaderDataType::Mat3:
		{
			return GL_FLOAT;
		}
		case ShaderDataType::Mat4:
		{
			return GL_FLOAT;
		}
		case ShaderDataType::Int:
		{
			return GL_INT;
		}
		case ShaderDataType::Int2:
		{
			return GL_INT;
		}
		case ShaderDataType::Int3:
		{
			return GL_INT;
		}
		case ShaderDataType::Int4:
		{
			return GL_INT;
		}
		case ShaderDataType::Bool:
		{
			return GL_BOOL;
		}
	}

	LD_CORE_ASSERT(false, "Unknown ShaderDataType!");

	return 0;
}

Ref<VertexArray> VertexArray::Create()
{
	return Ref<VertexArray>::Create();
}

VertexArray::VertexArray()
{
	Renderer::Submit([this]()
	{
		glCreateVertexArrays(1, &m_RendererID);
	});
}

VertexArray::~VertexArray()
{
	GLuint rendererID = m_RendererID;

	Renderer::Submit([rendererID]()
	{
		glDeleteVertexArrays(1, &rendererID);
	});
}

void VertexArray::Bind() const
{
	Ref<const VertexArray> instance = this;

	Renderer::Submit([instance]()
	{
		glBindVertexArray(instance->m_RendererID);
	});
}

void VertexArray::Unbind() const
{
	Ref<const VertexArray> instance = this;

	Renderer::Submit([instance]()
	{
		glBindVertexArray(0);
	});
}

void VertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
{
	LD_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

	Bind();
	vertexBuffer->Bind();

	Ref<VertexArray> instance = this;

	Renderer::Submit([instance, vertexBuffer]() mutable
	{
		const auto& layout = vertexBuffer->GetLayout();

		for (const auto& element : layout)
		{
			auto glBaseType = ShaderDataTypeToOpenGLBaseType(element.Type);

			glEnableVertexAttribArray(instance->m_VertexBufferIndex);

			if (glBaseType == GL_INT)
			{
					glVertexAttribIPointer(instance->m_VertexBufferIndex, element.GetComponentCount(), glBaseType, layout.GetStride(), (const void*)(intptr_t)element.Offset);
			}
			else
			{
				glVertexAttribPointer(instance->m_VertexBufferIndex, element.GetComponentCount(), glBaseType, element.Normalized ? GL_TRUE : GL_FALSE, layout.GetStride(), (const void*)(intptr_t)element.Offset);
			}

			instance->m_VertexBufferIndex++;
		}
	});

	m_VertexBuffers.push_back(vertexBuffer);
}

void VertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
{
	Bind();
	indexBuffer->Bind();

	m_IndexBuffer = indexBuffer;
}