#pragma once

#include "ldpch.h"

#include <glad/glad.h>

#include "Renderer.h"

struct RendererData
{
	//Ref<RenderPass> m_ActiveRenderPass;
	RenderCommandQueue m_CommandQueue;
	//Scope<ShaderLibrary> m_ShaderLibrary;
	//Ref<VertexArray> m_FullscreenQuadVertexArray;
};

static RendererData s_Data;

void Renderer::Init()
{
}

void Renderer::ExecuteRenderCommands()
{
	s_Data.m_CommandQueue.Execute();
}

RenderCommandQueue& Renderer::GetRenderCommandQueue()
{
	return s_Data.m_CommandQueue;
}