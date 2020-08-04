#include "ldpch.h"

#include "RenderPass.h"

Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& spec)
{
	return Ref<RenderPass>::Create(spec);
}

RenderPass::RenderPass(const RenderPassSpecification& spec)
	: m_Specification(spec)
{
}

RenderPass::~RenderPass()
{
}