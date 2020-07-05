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
	return std::make_shared<VertexArray>();
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
	Renderer::Submit([this]()
	{
		glDeleteVertexArrays(1, &m_RendererID);
	});
}

void VertexArray::Bind() const
{
	Renderer::Submit([this]()
	{
		glBindVertexArray(m_RendererID);
	});
}

void VertexArray::Unbind() const
{
	Renderer::Submit([this]()
	{
		glBindVertexArray(0);
	});
}

void VertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
{
	LD_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

	Bind();
	vertexBuffer->Bind();

	Renderer::Submit([this, vertexBuffer]()
	{
		const auto& layout = vertexBuffer->GetLayout();

		for (const auto& element : layout)
		{
			auto glBaseType = ShaderDataTypeToOpenGLBaseType(element.Type);
			glEnableVertexAttribArray(m_VertexBufferIndex);

			if (glBaseType == GL_INT)
			{
					glVertexAttribIPointer(m_VertexBufferIndex, element.GetComponentCount(), glBaseType, layout.GetStride(), (const void*)(intptr_t)element.Offset);
			}
			else
			{
				glVertexAttribPointer(m_VertexBufferIndex, element.GetComponentCount(), glBaseType, element.Normalized ? GL_TRUE : GL_FALSE, layout.GetStride(), (const void*)(intptr_t)element.Offset);
			}

			m_VertexBufferIndex++;
		}
	});

	m_VertexBuffers.push_back(vertexBuffer);
}

void VertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
{
	Bind();
	indexBuffer->Bind();

	m_IndexBuffer = indexBuffer;
}