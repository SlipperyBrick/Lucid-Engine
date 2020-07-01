#pragma once

#include "Lucid/Core/Base.h"

// Forward declaration for Framebuffer
class Framebuffer;

struct RenderPassSpecification
{
	Ref<Framebuffer> TargetFramebuffer;
};

class RenderPass
{

public:

	RenderPass(const RenderPassSpecification& spec);
	~RenderPass();

	const RenderPassSpecification& GetSpecification() const { return m_Specification; }

	static Ref<RenderPass> Create(const RenderPassSpecification& spec);

private:

	RenderPassSpecification m_Specification;
};