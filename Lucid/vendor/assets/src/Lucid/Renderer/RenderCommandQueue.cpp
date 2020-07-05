#pragma once

#include "ldpch.h"

#include "RenderCommandQueue.h"

#define LD_RENDER_TRACE(...) LD_CORE_TRACE(__VA_ARGS__)

RenderCommandQueue::RenderCommandQueue()
{
	// Set command buffer to store 10MB of data
	m_CommandBuffer = new uint8_t[10 * 1024 * 1024];
	m_CommandBufferPtr = m_CommandBuffer;
	memset(m_CommandBuffer, 0, 10 * 1024 * 1024);
}

RenderCommandQueue::~RenderCommandQueue()
{
	delete[] m_CommandBuffer;
}

void* RenderCommandQueue::Allocate(RenderCommandFn fn, uint32_t size)
{
	*(RenderCommandFn*)m_CommandBufferPtr = fn;
	m_CommandBufferPtr += sizeof(RenderCommandFn);

	*(uint32_t*)m_CommandBufferPtr = size;
	m_CommandBufferPtr += sizeof(uint32_t);

	void* memory = m_CommandBufferPtr;
	m_CommandBufferPtr += size;

	m_CommandCount++;

	return memory;
}

void RenderCommandQueue::Execute()
{
	//LD_RENDER_TRACE("RenderCommandQueue::Execute -- {0} commands, {1} bytes", m_CommandCount, (m_CommandBufferPtr - m_CommandBuffer));

	// Starting memory location for command buffer
	byte* buffer = m_CommandBuffer;

	for (uint32_t i = 0; i < m_CommandCount; i++)
	{
		RenderCommandFn function = *(RenderCommandFn*)buffer;

		buffer += sizeof(RenderCommandFn);

		uint32_t size = *(uint32_t*)buffer;

		buffer += sizeof(uint32_t);
		function(buffer);
		buffer += size;
	}

	m_CommandBufferPtr = m_CommandBuffer;
	m_CommandCount = 0;
}