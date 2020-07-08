#pragma once

#include "Lucid/Renderer/VertexBuffer.h"

class VertexArray : public RefCounted
{

public:

	VertexArray();
	~VertexArray();

	void Bind() const;
	void Unbind() const;

	void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer);
	void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer);

	const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() { return m_VertexBuffers; }
	const Ref<IndexBuffer>& GetIndexBuffer() { return m_IndexBuffer; }

	RendererID GetRendererID() { return m_RendererID; };

	static Ref<VertexArray> Create();

private:

	RendererID m_RendererID = 0;
	uint32_t m_VertexBufferIndex = 0;
	std::vector<Ref<VertexBuffer>> m_VertexBuffers;
	Ref<IndexBuffer> m_IndexBuffer;
};