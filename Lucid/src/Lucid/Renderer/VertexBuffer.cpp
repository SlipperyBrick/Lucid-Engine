#include "ldpch.h"

#include "VertexBuffer.h"

#include "glad/glad.h"

Ref<VertexBuffer> VertexBuffer::Create(void* data, uint32_t size, VertexBufferUsage usage)
{
	return std::make_shared<VertexBuffer>(data, size, usage);
}

Ref<VertexBuffer> VertexBuffer::Create(uint32_t size, VertexBufferUsage usage)
{
	return std::make_shared<VertexBuffer>(size, usage);
}

Ref<IndexBuffer> IndexBuffer::Create(void* data, uint32_t size)
{
	return std::make_shared<IndexBuffer>(data, size);
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

	Renderer::Submit([=]()
	{
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, m_Size, m_LocalData.Data, OpenGLUsage(m_Usage));
	});
}

VertexBuffer::VertexBuffer(uint32_t size, VertexBufferUsage usage)
	: m_Size(size), m_Usage(usage)
{
	Renderer::Submit([this]()
	{
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, m_Size, nullptr, OpenGLUsage(m_Usage));
	});
}

VertexBuffer::~VertexBuffer()
{
	Renderer::Submit([this]()
	{
		glDeleteBuffers(1, &m_RendererID);
	});
}

void VertexBuffer::SetData(void* data, uint32_t size, uint32_t offset)
{
	m_LocalData = Memory::Copy(data, size);
	m_Size = size;

	Renderer::Submit([this, offset]()
	{
		glNamedBufferSubData(m_RendererID, offset, m_Size, m_LocalData.Data);
	});
}

void VertexBuffer::Bind() const
{
	Renderer::Submit([this]()
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	});
}

IndexBuffer::IndexBuffer(void* data, uint32_t size)
	: m_RendererID(0), m_Size(size)
{
	m_LocalData = Memory::Copy(data, size);

	Renderer::Submit([this]()
	{
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, m_Size, m_LocalData.Data, GL_STATIC_DRAW);
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

	Renderer::Submit([this, offset]()
	{
		glNamedBufferSubData(m_RendererID, offset, m_Size, m_LocalData.Data);
	});
}

void IndexBuffer::Bind() const
{
	Renderer::Submit([this]()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	});
}