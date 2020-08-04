#include "ldpch.h"

#include "ShaderLibrary.h"

ShaderLibrary::ShaderLibrary()
{
}

ShaderLibrary::~ShaderLibrary()
{
}

void ShaderLibrary::Add(const Ref<Shader>& shader)
{
	auto& name = shader->GetName();

	LD_CORE_ASSERT(m_Shaders.find(name) == m_Shaders.end());

	m_Shaders[name] = shader;
}

void ShaderLibrary::Load(const std::string& path)
{
	auto shader = Ref<Shader>(Shader::Create(path));

	auto& name = shader->GetName();

	LD_CORE_ASSERT(m_Shaders.find(name) == m_Shaders.end());

	m_Shaders[name] = shader;
}

void ShaderLibrary::Load(const std::string& name, const std::string& path)
{
	LD_CORE_ASSERT(m_Shaders.find(name) == m_Shaders.end());

	m_Shaders[name] = Ref<Shader>(Shader::Create(path));
}

Ref<Shader>& ShaderLibrary::Get(const std::string& name)
{
	LD_CORE_ASSERT(m_Shaders.find(name) != m_Shaders.end());

	return m_Shaders[name];
}