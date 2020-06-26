#pragma once

#include <glm/glm.hpp>

#include "Lucid/Renderer/Renderer.h"

enum class FramebufferFormat
{
	None = 0,
	RGBA8 = 1,
	RGBA16F = 2
};

struct FramebufferSpecification
{
	uint32_t Width = 1280;
	uint32_t Height = 720;

	glm::vec4 ClearColor;

	FramebufferFormat Format;

	// Multisampling
	uint32_t Samples = 1;

	bool ScreenBufferTarget = false;
};

class Framebuffer
{

public:

	Framebuffer(const FramebufferSpecification& spec);
	~Framebuffer();

	void Bind() const;
	void Unbind() const;

	void Resize(uint32_t width, uint32_t height, bool forceRecreate = false);

	void BindTexture(uint32_t slot = 0) const;

	virtual RendererID GetRendererID() const { return m_RendererID; }
	virtual RendererID GetColorAttachmentRendererID() const { return m_ColorAttachment; }
	virtual RendererID GetDepthAttachmentRendererID() const { return m_DepthAttachment; }

	virtual const FramebufferSpecification& GetSpecification() const { return m_Specification; }

	static Ref<Framebuffer> Create(const FramebufferSpecification& spec);

private:

	FramebufferSpecification m_Specification;

	RendererID m_RendererID = 0;
	RendererID m_ColorAttachment = 0;
	RendererID m_DepthAttachment = 0;
};

class FramebufferPool final
{

public:

	FramebufferPool(uint32_t maxFBs = 32);
	~FramebufferPool();

	std::weak_ptr<Framebuffer> AllocateBuffer();
	void Add(std::weak_ptr<Framebuffer> framebuffer);

	const std::vector<std::weak_ptr<Framebuffer>>& GetAll() const { return m_Pool; }

	inline static FramebufferPool* GetGlobal() { return s_Instance; }

private:

	std::vector<std::weak_ptr<Framebuffer>> m_Pool;

	static FramebufferPool* s_Instance;
};