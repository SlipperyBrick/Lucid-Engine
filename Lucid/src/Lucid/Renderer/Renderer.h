#pragma once

#include "Lucid/Renderer/RenderCommandQueue.h"

using RendererID = uint32_t;

enum class PrimitiveType
{
	None = 0,
	Triangles,
	Lines
};

struct RenderCapabilities
{
	std::string Vendor;
	std::string Renderer;
	std::string Version;

	int MaxSamples = 0;
	float MaxAnisotropy = 0.0f;
	int MaxTextureUnits = 0;

	static RenderCapabilities& GetCapabilities()
	{
		static RenderCapabilities capabilities;

		return capabilities;
	}
};

class Renderer
{

public:

	typedef void(*RenderCommandFn)(void*);

	static void Init();

	// Allocates memory in the render command queue for each submitted command to the renderer
	template<typename FuncT>
	static void Submit(FuncT&& func)
	{
		auto renderCmd = [](void* ptr)
		{
			auto pFunc = (FuncT*)ptr;
			(*pFunc)();

			pFunc->~FuncT();
		};

		auto storageBuffer = GetRenderCommandQueue().Allocate(renderCmd, sizeof(func));
		new (storageBuffer) FuncT(std::forward<FuncT>(func));
	}

	static void ExecuteRenderCommands();

private:

	static RenderCommandQueue& GetRenderCommandQueue();
};