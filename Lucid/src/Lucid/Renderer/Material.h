#pragma once

#include <unordered_set>

#include "Lucid/Core/Base.h"

#include "Lucid/Renderer/Shader.h"
#include "Lucid/Renderer/Texture.h"

class MaterialInstance;

enum class MaterialFlag
{
	None = 0,
	DepthTest = 1,
	Blend = 2,
};

class Material : public RefCounted
{

	friend class MaterialInstance;

public:

	Material(const Ref<Shader>& shader);
	virtual ~Material();

	void Bind();

	uint32_t GetFlags() const { return m_MaterialFlags; }
	void SetFlag(MaterialFlag flag) { m_MaterialFlags |= (uint32_t)flag; }

	template <typename T>
	void Set(const std::string& name, const T& value)
	{
		auto decl = FindUniformDeclaration(name);

		LD_CORE_ASSERT("Could not find uniform with name '{0}'", name);

		auto& buffer = GetUniformBufferTarget(decl);
		buffer.Write((byte*)&value, decl->GetSize(), decl->GetOffset());

		for (auto mi : m_MaterialInstances)
		{
			mi->OnMaterialValueUpdated(decl);
		}
	}

	void Set(const std::string& name, const Ref<Texture2D>& texture)
	{
		auto decl = FindResourceDeclaration(name);

		uint32_t slot = decl->GetRegister();

		if (m_Textures.size() <= slot)
		{
			m_Textures.resize((size_t)slot + 1);
		}

		m_Textures[slot] = texture;
	}

public:

	static Ref<Material> Create(const Ref<Shader>& shader);

	Ref<Shader> GetShader() { return m_Shader; }

private:

	void AllocateStorage();
	void OnShaderReloaded();
	void BindTextures();

	ShaderUniformDeclaration* FindUniformDeclaration(const std::string& name);
	ShaderResourceDeclaration* FindResourceDeclaration(const std::string& name);
	Memory& GetUniformBufferTarget(ShaderUniformDeclaration* uniformDeclaration);

private:

	Ref<Shader> m_Shader;

	std::unordered_set<MaterialInstance*> m_MaterialInstances;

	Memory m_VSUniformStorageBuffer;
	Memory m_FSUniformStorageBuffer;

	std::vector<Ref<Texture2D>> m_Textures;

	uint32_t m_MaterialFlags;
};

class MaterialInstance : public RefCounted
{

	friend class Material;

public:

	MaterialInstance(const Ref<Material>& material);
	virtual ~MaterialInstance();

	template <typename T>
	void Set(const std::string& name, const T& value)
	{
		auto decl = m_Material->FindUniformDeclaration(name);

		if (!decl)
		{
			return;
		}

		LD_CORE_ASSERT(decl, "Could not find uniform with name 'x'");

		auto& buffer = GetUniformBufferTarget(decl);
		buffer.Write((byte*)& value, decl->GetSize(), decl->GetOffset());

		m_OverriddenValues.insert(name);
	}

	void Set(const std::string& name, const Ref<Texture2D>& texture)
	{
		auto decl = m_Material->FindResourceDeclaration(name);

		if (!decl)
		{
			LD_CORE_WARN("Cannot find material property: ", name);
		}

		uint32_t slot = decl->GetRegister();

		if (m_Textures.size() <= slot)
		{
			m_Textures.resize((size_t)slot + 1);
		}

		m_Textures[slot] = texture;
	}

	void Bind();

	uint32_t GetFlags() const { return m_Material->m_MaterialFlags; }
	bool GetFlag(MaterialFlag flag) const { return (uint32_t)flag & m_Material->m_MaterialFlags; }
	void SetFlag(MaterialFlag flag, bool value = true);

	Ref<Shader>GetShader() { return m_Material->m_Shader; }

public:

	static Ref<MaterialInstance> Create(const Ref<Material>& material);

private:

	void AllocateStorage();
	void OnShaderReloaded();

	Memory& GetUniformBufferTarget(ShaderUniformDeclaration* uniformDeclaration);
	void OnMaterialValueUpdated(ShaderUniformDeclaration* decl);

private:

	Ref<Material> m_Material;

	Memory m_VSUniformStorageBuffer;
	Memory m_FSUniformStorageBuffer;

	std::vector<Ref<Texture2D>> m_Textures;

	std::unordered_set<std::string> m_OverriddenValues;
};