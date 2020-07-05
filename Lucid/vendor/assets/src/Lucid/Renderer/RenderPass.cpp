#pragma once

#include "ldpch.h"

#include "RenderPass.h"

Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& spec)
{
	return std::make_shared<RenderPass>(spec);
}

RenderPass::RenderPass(const RenderPassSpecification& spec)
	: m_Specification(spec)
{
}

RenderPass::~RenderPass()
{
}