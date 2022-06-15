#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>

#include "Lucid/Core/Base.h"
#include "Lucid/Core/Memory.h"

#include "Lucid/Renderer/ShaderUniform.h"

enum class UniformType
{
	None = 0,
	Float, Float2, Float3, Float4,
	Matrix3x3, Matrix4x4,
	Int32, Uint32
};

struct UniformDecl
{
	UniformType Type;
	std::ptrdiff_t Offset;
	std::string Name;
};

struct UniformBuffer
{
	// CPU-side buffer abstraction, represents a byte buffer that is packed with uniforms
	byte* Buffer;

	std::vector<UniformDecl> Uniforms;
};

struct UniformBufferBase
{
	virtual const byte* GetBuffer() const = 0;
	virtual const UniformDecl* GetUniforms() const = 0;
	virtual unsigned int GetUniformCount() const = 0;
};

template<unsigned int N, unsigned int U>
struct UniformBufferDeclaration : public UniformBufferBase
{
	byte Buffer[N];
	UniformDecl Uniforms[U];
	std::ptrdiff_t Cursor = 0;
	int Index = 0;

	virtual const byte* GetBuffer() const override { return Buffer; }
	virtual const UniformDecl* GetUniforms() const override { return Uniforms; }
	virtual unsigned int GetUniformCount() const { return U; }

	template<typename T>
	void Push(const std::string& name, const T& data) {}

	template<>
	void Push(const std::string& name, const float& data)
	{
		Uniforms[Index++] = { UniformType::Float, Cursor, name };
		memcpy(Buffer + Cursor, &data, sizeof(float));
		Cursor += sizeof(float);
	}

	template<>
	void Push(const std::string& name, const glm::vec3& data)
	{
		Uniforms[Index++] = { UniformType::Float3, Cursor, name };
		memcpy(Buffer + Cursor, glm::value_ptr(data), sizeof(glm::vec3));
		Cursor += sizeof(glm::vec3);
	}

	template<>
	void Push(const std::string& name, const glm::vec4& data)
	{
		Uniforms[Index++] = { UniformType::Float4, Cursor, name };
		memcpy(Buffer + Cursor, glm::value_ptr(data), sizeof(glm::vec4));
		Cursor += sizeof(glm::vec4);
	}

	template<>
	void Push(const std::string& name, const glm::mat4& data)
	{
		Uniforms[Index++] = { UniformType::Matrix4x4, Cursor, name };
		memcpy(Buffer + Cursor, glm::value_ptr(data), sizeof(glm::mat4));
		Cursor += sizeof(glm::mat4);
	}

};

class Shader : public RefCounted
{

public:

	using ShaderReloadedCallback = std::function<void()>;

	Shader() = default;
	Shader(const std::string& filepath);

	// Represents a complete shader program, stored in a single file
	static Ref<Shader> Create(const std::string& filepath);
	static Ref<Shader> CreateFromString(const std::string& source);

	void Reload();
	void AddShaderReloadedCallback(const ShaderReloadedCallback& callback);

	void Bind();

	RendererID GetRendererID() const { return m_RendererID; }

	void UploadUniformBuffer(const UniformBufferBase& uniformBuffer);

	void SetVSMaterialUniformBuffer(Memory buffer);
	void SetFSMaterialUniformBuffer(Memory buffer);

	void SetFloat(const std::string& name, float value);
	void SetInt(const std::string& name, int value);
	void SetVec2(const std::string& name, const glm::vec2& value);
	void SetVec3(const std::string& name, const glm::vec3& value);
	void SetVec4(const std::string& name, const glm::vec4& value);
	void SetIntArray(const std::string& name, int* values, uint32_t size);
	void SetMat4(const std::string& name, const glm::mat4& value);
	void SetMat4FromRenderThread(const std::string& name, const glm::mat4& value, bool bind = true);

	const std::string& GetName() const { return m_Name; }

	const ShaderUniformBufferList& GetVSRendererUniforms() const { return m_VSRendererUniformBuffers; }
	const ShaderUniformBufferList& GetFSRendererUniforms() const { return m_FSRendererUniformBuffers; }

	bool HasVSMaterialUniformBuffer() const { return (bool)m_VSMaterialUniformBuffer; }
	bool HasFSMaterialUniformBuffer() const { return (bool)m_FSMaterialUniformBuffer; }

	const ShaderUniformBufferDeclaration& GetVSMaterialUniformBuffer() const { return *m_VSMaterialUniformBuffer; }
	const ShaderUniformBufferDeclaration& GetFSMaterialUniformBuffer() const { return *m_FSMaterialUniformBuffer; }

	const ShaderResourceList& GetResources() const { return m_Resources; }

	static std::vector<Ref<Shader>> s_AllShaders;

private:

	void Load(const std::string& source);

	std::string ReadShaderFromFile(const std::string& filepath) const;
	std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);

	void Parse();
	void ParseUniform(const std::string& statement, ShaderDomain domain);
	void ParseUniformStruct(const std::string& block, ShaderDomain domain);

	ShaderStruct* FindStruct(const std::string& name);

	int32_t GetUniformLocation(const std::string& name) const;

	void ResolveUniforms();
	void CompileAndUploadShader();

	static GLenum ShaderTypeFromString(const std::string& type);

	void ResolveAndSetUniforms(const Scope<ShaderUniformBufferDeclaration>& decl, Memory buffer);
	void ResolveAndSetUniform(ShaderUniformDeclaration* uniform, Memory buffer);
	void ResolveAndSetUniformArray(ShaderUniformDeclaration* uniform, Memory buffer);
	void ResolveAndSetUniformField(const ShaderUniformDeclaration& field, byte* data, int32_t offset);

	void UploadUniformInt(uint32_t location, int32_t value);
	void UploadUniformIntArray(uint32_t location, int32_t* values, int32_t count);
	void UploadUniformFloat(uint32_t location, float value);
	void UploadUniformFloat2(uint32_t location, const glm::vec2& value);
	void UploadUniformFloat3(uint32_t location, const glm::vec3& value);
	void UploadUniformFloat4(uint32_t location, const glm::vec4& value);
	void UploadUniformMat3(uint32_t location, const glm::mat3& values);
	void UploadUniformMat4(uint32_t location, const glm::mat4& values);
	void UploadUniformMat4Array(uint32_t location, const glm::mat4& values, uint32_t count);

	void UploadUniformStruct(ShaderUniformDeclaration* uniform, byte* buffer, uint32_t offset);

	void UploadUniformInt(const std::string& name, int32_t value);
	void UploadUniformIntArray(const std::string& name, int32_t* values, uint32_t count);

	void UploadUniformFloat(const std::string& name, float value);
	void UploadUniformFloat2(const std::string& name, const glm::vec2& value);
	void UploadUniformFloat3(const std::string& name, const glm::vec3& value);
	void UploadUniformFloat4(const std::string& name, const glm::vec4& value);

	void UploadUniformMat4(const std::string& name, const glm::mat4& value);

private:

	RendererID m_RendererID = 0;

	bool m_Loaded = false;
	bool m_IsCompute = false;

	std::string m_Name, m_AssetPath;
	std::unordered_map<GLenum, std::string> m_ShaderSource;

	std::vector<ShaderReloadedCallback> m_ShaderReloadedCallbacks;

	ShaderUniformBufferList m_VSRendererUniformBuffers;
	ShaderUniformBufferList m_FSRendererUniformBuffers;

	Scope<ShaderUniformBufferDeclaration> m_VSMaterialUniformBuffer;
	Scope<ShaderUniformBufferDeclaration> m_FSMaterialUniformBuffer;

	ShaderResourceList m_Resources;
	ShaderStructList m_Structs;
};