#pragma once

#include "ldpch.h"

#include <glad/glad.h>

#include "Renderer.h"

#include "Lucid/Renderer/VertexArray.h"

struct RendererData
{
	//Ref<RenderPass> m_ActiveRenderPass;
	RenderCommandQueue m_CommandQueue;
	//Scope<ShaderLibrary> m_ShaderLibrary;
	Ref<VertexArray> m_FullscreenQuadVertexArray;
};

static RendererData s_Data;

void Renderer::Init()
{
}

// Executes all render commands (in sequential order) that are waiting in the render command queue
void Renderer::ExecuteRenderCommands()
{
	s_Data.m_CommandQueue.Execute();
}

// Retrieves the current state of the render command queue
RenderCommandQueue& Renderer::GetRenderCommandQueue()
{
	return s_Data.m_CommandQueue;
}