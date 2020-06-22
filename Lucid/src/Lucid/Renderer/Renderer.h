#pragma once

#include "Lucid/Renderer/RenderCommandQueue.h"

class Renderer
{

public:

	static void Init();

	// Allocates memory in the render command queue for each command submission to the renderer
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