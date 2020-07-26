#pragma once

#include "ldpch.h"

#include <glad/glad.h>

#include "VertexBuffer.h"

#include "Lucid/Renderer/Renderer.h"

Ref<VertexBuffer> VertexBuffer::Create(void* data, uint32_t size, VertexBufferUsage usage)
{
	return Ref<VertexBuffer>::Create(data, size, usage);
}

Ref<VertexBuffer> VertexBuffer::Create(uint32_t size, VertexBufferUsage usage)
{
	return Ref<VertexBuffer>::Create(size, usage);
}

Ref<IndexBuffer> IndexBuffer::Create(void* data, uint32_t size)
{
	return Ref<IndexBuffer>::Create(data, size);
}

static GLenum OpenGLUsage(VertexBufferUsage usage)
{
	switch (usage)
	{
		case VertexBufferUsage::Static:
		{
			return GL_STATIC_DRAW;
		}
		case VertexBufferUsage::Dynamic:
		{
			return GL_DYNAMIC_DRAW;
		}
	}

	LD_CORE_ASSERT(false, "Unknown vertex buffer usage");

	return 0;
}

VertexBuffer::VertexBuffer(void* data, uint32_t size, VertexBufferUsage usage)
	: m_Size(size), m_Usage(usage)
{
	m_LocalData = Memory::Copy(data, size);

	Ref<VertexBuffer> instance = this;

	Renderer::Submit([instance]() mutable
	{
		glCreateBuffers(1, &instance->m_RendererID);
		glNamedBufferData(instance->m_RendererID, instance->m_Size, instance->m_LocalData.Data, OpenGLUsage(instance->m_Usage));
	});
}

VertexBuffer::VertexBuffer(uint32_t size, VertexBufferUsage usage)
	: m_Size(size), m_Usage(usage)
{

	Ref<VertexBuffer> instance = this;

	Renderer::Submit([instance]() mutable
	{
		glCreateBuffers(1, &instance->m_RendererID);
		glNamedBufferData(instance->m_RendererID, instance->m_Size, nullptr, OpenGLUsage(instance->m_Usage));
	});
}

VertexBuffer::~VertexBuffer()
{
	GLuint rendererID = m_RendererID;

	Renderer::Submit([rendererID]()
	{
		glDeleteBuffers(1, &rendererID);
	});

	/*Renderer::Submit([this]()
	{
		glDeleteBuffers(1, &m_RendererID);
	});*/
}

void VertexBuffer::SetData(void* data, uint32_t size, uint32_t offset)
{
	m_LocalData = Memory::Copy(data, size);
	m_Size = size;

	Ref<VertexBuffer> instance = this;

	Renderer::Submit([instance, offset]()
	{
		glNamedBufferSubData(instance->m_RendererID, offset, instance->m_Size, instance->m_LocalData.Data);
	});
}

void VertexBuffer::Bind() const
{
	Renderer::Submit([this]()
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	});
}

IndexBuffer::IndexBuffer(uint32_t size)
	: m_Size(size)
{
	// m_LocalData = Buffer(size);

	Ref<IndexBuffer> instance = this;

	Renderer::Submit([instance]() mutable 
	{
		glCreateBuffers(1, &instance->m_RendererID);
		glNamedBufferData(instance->m_RendererID, instance->m_Size, nullptr, GL_DYNAMIC_DRAW);
	});
}

IndexBuffer::IndexBuffer(void* data, uint32_t size)
	: m_RendererID(0), m_Size(size)
{
	m_LocalData = Memory::Copy(data, size);

	Ref<IndexBuffer> instance = this;

	Renderer::Submit([instance]() mutable
	{
		glCreateBuffers(1, &instance->m_RendererID);
		glNamedBufferData(instance->m_RendererID, instance->m_Size, instance->m_LocalData.Data, GL_STATIC_DRAW);
	});
}

IndexBuffer::~IndexBuffer()
{
	Renderer::Submit([this]()
	{
		glDeleteBuffers(1, &m_RendererID);
	});
}

void IndexBuffer::SetData(void* data, uint32_t size, uint32_t offset)
{
	m_LocalData = Memory::Copy(data, size);
	m_Size = size;

	Ref<IndexBuffer> instance = this;

	Renderer::Submit([instance, offset]()
	{
		glNamedBufferSubData(instance->m_RendererID, offset, instance->m_Size, instance->m_LocalData.Data);
	});
}

void IndexBuffer::Bind() const
{
	Renderer::Submit([this]()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	});
}