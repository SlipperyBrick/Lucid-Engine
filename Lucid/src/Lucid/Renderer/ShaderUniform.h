#pragma once

#include <string>
#include <vector>

#include "Lucid/Core/Base.h"

enum class ShaderDomain
{
	None = 0,
	Vertex = 0,
	Fragment = 1
};

// Forward declaration for ShaderStruct class
class ShaderUniformDeclaration;

class ShaderStruct
{

private:

	friend class Shader;

private:

	std::string m_Name;

	std::vector<ShaderUniformDeclaration*> m_Fields;

	uint32_t m_Size;
	uint32_t m_Offset;

public:

	ShaderStruct(const std::string& name);

	void AddField(ShaderUniformDeclaration* field);

	void SetOffset(uint32_t offset);

	const std::string& GetName() const;
	uint32_t GetSize() const;
	uint32_t GetOffset() const;

	const std::vector<ShaderUniformDeclaration*>& GetFields() const;
};

typedef std::vector<ShaderStruct*> ShaderStructList;

class ShaderUniformDeclaration
{

private:

	friend class Shader;
	friend class ShaderStruct;
	friend class ShaderUniformBufferDeclaration;

public:

	enum class Type
	{
		NONE,
		FLOAT32,
		VEC2, VEC3, VEC4,
		MAT3, MAT4,
		INT32,
		STRUCT
	};

private:

	std::string m_Name;

	uint32_t m_Size;
	uint32_t m_Count;
	uint32_t m_Offset;

	ShaderDomain m_Domain;

	Type m_Type;
	ShaderStruct* m_Struct;

	mutable int32_t m_Location;

public:

	ShaderUniformDeclaration(ShaderDomain domain, Type type, const std::string& name, uint32_t count = 1);
	ShaderUniformDeclaration(ShaderDomain domain, ShaderStruct* uniformStruct, const std::string& name, uint32_t count = 1);

	inline const std::string& GetName() const { return m_Name; }
	inline uint32_t GetSize() const { return m_Size; }
	inline uint32_t GetCount() const { return m_Count; }
	inline uint32_t GetOffset() const { return m_Offset; }
	inline uint32_t GetAbsoluteOffset() const { return m_Struct ? m_Struct->GetOffset() + m_Offset : m_Offset; }
	inline ShaderDomain GetDomain() const { return m_Domain; }

	int32_t GetLocation() const { return m_Location; }
	inline Type GetType() const { return m_Type; }

	inline bool IsArray() const { return m_Count > 1; }

	inline const ShaderStruct& GetShaderUniformStruct() const { LD_CORE_ASSERT(m_Struct, ""); return *m_Struct; }

protected:

	void SetOffset(uint32_t offset);

public:

	static uint32_t SizeOfUniformType(Type type);

	static Type StringToType(const std::string& type);
	static std::string TypeToString(Type type);
};

typedef std::vector<ShaderUniformDeclaration*> ShaderUniformList;

class ShaderUniformBufferDeclaration
{

private:

	friend class Shader;

private:

	std::string m_Name;

	ShaderUniformList m_Uniforms;

	uint32_t m_Register;
	uint32_t m_Size;

	ShaderDomain m_Domain;

public:

	ShaderUniformBufferDeclaration(const std::string& name, ShaderDomain domain);

	void PushUniform(ShaderUniformDeclaration* uniform);

	inline const std::string& GetName() const { return m_Name; }
	inline uint32_t GetRegister() const { return m_Register; }
	inline uint32_t GetSize() const { return m_Size; }
	virtual ShaderDomain GetDomain() const { return m_Domain; }
	inline const ShaderUniformList& GetUniformDeclarations() const { return m_Uniforms; }

	ShaderUniformDeclaration* FindUniform(const std::string& name);
};

typedef std::vector<ShaderUniformBufferDeclaration*> ShaderUniformBufferList;

class ShaderResourceDeclaration
{

public:

	enum class Type
	{
		NONE,
		TEXTURE2D,
		TEXTURECUBE
	};

private:

	friend class Shader;

private:

	std::string m_Name;

	uint32_t m_Register = 0;
	uint32_t m_Count;

	Type m_Type;

public:

	ShaderResourceDeclaration(Type type, const std::string& name, uint32_t count);

	inline const std::string& GetName() const { return m_Name; }

	inline uint32_t GetRegister() const { return m_Register; }
	inline uint32_t GetCount() const { return m_Count; }

	inline Type GetType() const { return m_Type; }

public:

	static Type StringToType(const std::string& type);
	static std::string TypeToString(Type type);
};

typedef std::vector<ShaderResourceDeclaration*> ShaderResourceList;

struct ShaderUniformField
{
	ShaderUniformDeclaration::Type type;

	std::string name;
	uint32_t count;

	mutable uint32_t size;
	mutable int32_t location;
};