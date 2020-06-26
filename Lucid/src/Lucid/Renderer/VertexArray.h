#include "Lucid/Renderer/Renderer.h"

#include "Lucid/Renderer/VertexBuffer.h"

class VertexArray
{

public:

	VertexArray();
	~VertexArray();

	void Bind() const;
	void Unbind() const;

	void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer);
	void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer);

	const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() { return m_VertexBuffers; }
	const std::shared_ptr<IndexBuffer>& GetIndexBuffer() { return m_IndexBuffer; }

	RendererID GetRendererID() { return m_RendererID; };

private:

	RendererID m_RendererID = 0;
	uint32_t m_VertexBufferIndex = 0;
	std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
	std::shared_ptr<IndexBuffer> m_IndexBuffer;
};