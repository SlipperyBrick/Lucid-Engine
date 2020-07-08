#pragma once

#include "Lucid/Core/Base.h"

#include "Lucid/Renderer/Framebuffer.h"

struct RenderPassSpecification
{
	Ref<Framebuffer> TargetFramebuffer;
};

class RenderPass : public RefCounted
{

public:

	RenderPass(const RenderPassSpecification& spec);
	~RenderPass();

	RenderPassSpecification& GetSpecification() { return m_Specification; }

	static Ref<RenderPass> Create(const RenderPassSpecification& spec);

private:

	RenderPassSpecification m_Specification;
};