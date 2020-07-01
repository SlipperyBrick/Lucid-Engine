#pragma once

#include "Lucid/Renderer/Shader.h"

class ShaderLibrary
{

public:

	ShaderLibrary();
	~ShaderLibrary();

	void Add(const Ref<Shader>& shader);
	void Load(const std::string& path);
	void Load(const std::string& name, const std::string& path);

	Ref<Shader>& Get(const std::string& name);

private:

	std::unordered_map<std::string, Ref<Shader>> m_Shaders;
};