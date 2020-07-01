#pragma once

#include "Lucid/Renderer/RenderCommandQueue.h"
#include "Lucid/Renderer/RenderPass.h"
#include "Lucid/Renderer/ShaderLibrary.h"

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

	static void Clear();
	static void Clear(float r, float g, float b, float a = 1.0f);

	static void SetClearColour(float r, float g, float b, float a);
	static void SetLineThickness(float thickness);

	static void DrawIndexed(uint32_t count, PrimitiveType type, bool depthTest = true);

	static const Scope<ShaderLibrary>& GetShaderLibrary();

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

	static void BeginRenderPass(const Ref<RenderPass>& renderPass, bool clear = true);
	static void EndRenderPass();

private:

	static RenderCommandQueue& GetRenderCommandQueue();
};