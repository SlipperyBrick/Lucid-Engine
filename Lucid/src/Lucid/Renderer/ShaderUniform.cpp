#pragma once

#include "ldpch.h"

#include "ShaderUniform.h"

ShaderStruct::ShaderStruct(const std::string& name)
	: m_Name(name), m_Size(0), m_Offset(0)
{
}

void ShaderStruct::AddField(ShaderUniformDeclaration* field)
{
	m_Size += field->GetSize();

	uint32_t offset = 0;

	if (m_Fields.size())
	{
		ShaderUniformDeclaration* previous = m_Fields.back();

		offset = previous->GetOffset() + previous->GetSize();
	}

	field->SetOffset(offset);
	m_Fields.push_back(field);
}

void ShaderStruct::SetOffset(uint32_t offset)
{
	m_Offset = offset;
}

const std::string& ShaderStruct::GetName() const
{
	return m_Name;
}

uint32_t ShaderStruct::GetSize() const
{
	return m_Size;
}

uint32_t ShaderStruct::GetOffset() const
{
	return m_Offset;
}

const std::vector<ShaderUniformDeclaration*>& ShaderStruct::GetFields() const
{
	return m_Fields;
}

ShaderUniformDeclaration::ShaderUniformDeclaration(ShaderDomain domain, Type type, const std::string& name, uint32_t count)
	: m_Type(type), m_Struct(nullptr), m_Domain(domain)
{
	m_Name = name;
	m_Count = count;
	m_Size = SizeOfUniformType(type) * count;
}

ShaderUniformDeclaration::ShaderUniformDeclaration(ShaderDomain domain, ShaderStruct* uniformStruct, const std::string& name, uint32_t count)
	: m_Struct(uniformStruct), m_Type(ShaderUniformDeclaration::Type::STRUCT), m_Domain(domain)
{
	m_Name = name;
	m_Count = count;
	m_Size = m_Struct->GetSize() * count;
}

void ShaderUniformDeclaration::SetOffset(uint32_t offset)
{
	if (m_Type == ShaderUniformDeclaration::Type::STRUCT)
	{
		m_Struct->SetOffset(offset);
	}

	m_Offset = offset;
}

uint32_t ShaderUniformDeclaration::SizeOfUniformType(Type type)
{
	switch (type)
	{
	case ShaderUniformDeclaration::Type::INT32:      return 4;
	case ShaderUniformDeclaration::Type::FLOAT32:    return 4;
	case ShaderUniformDeclaration::Type::VEC2:       return 4 * 2;
	case ShaderUniformDeclaration::Type::VEC3:       return 4 * 3;
	case ShaderUniformDeclaration::Type::VEC4:       return 4 * 4;
	case ShaderUniformDeclaration::Type::MAT3:       return 4 * 3 * 3;
	case ShaderUniformDeclaration::Type::MAT4:       return 4 * 4 * 4;
	}
	return 0;
}

ShaderUniformDeclaration::Type ShaderUniformDeclaration::StringToType(const std::string& type)
{
	if (type == "int")      return Type::INT32;
	if (type == "float")    return Type::FLOAT32;
	if (type == "vec2")     return Type::VEC2;
	if (type == "vec3")     return Type::VEC3;
	if (type == "vec4")     return Type::VEC4;
	if (type == "mat3")     return Type::MAT3;
	if (type == "mat4")     return Type::MAT4;

	return Type::NONE;
}

std::string ShaderUniformDeclaration::TypeToString(Type type)
{
	switch (type)
	{
	case ShaderUniformDeclaration::Type::INT32:      return "int32";
	case ShaderUniformDeclaration::Type::FLOAT32:    return "float";
	case ShaderUniformDeclaration::Type::VEC2:       return "vec2";
	case ShaderUniformDeclaration::Type::VEC3:       return "vec3";
	case ShaderUniformDeclaration::Type::VEC4:       return "vec4";
	case ShaderUniformDeclaration::Type::MAT3:       return "mat3";
	case ShaderUniformDeclaration::Type::MAT4:       return "mat4";
	}
	return "Invalid Type";
}

ShaderUniformBufferDeclaration::ShaderUniformBufferDeclaration(const std::string& name, ShaderDomain domain)
	: m_Name(name), m_Domain(domain), m_Size(0), m_Register(0)
{
}

void ShaderUniformBufferDeclaration::PushUniform(ShaderUniformDeclaration* uniform)
{
	uint32_t offset = 0;
	if (m_Uniforms.size())
	{
		ShaderUniformDeclaration* previous = (ShaderUniformDeclaration*)m_Uniforms.back();
		offset = previous->m_Offset + previous->m_Size;
	}
	uniform->SetOffset(offset);
	m_Size += uniform->GetSize();
	m_Uniforms.push_back(uniform);
}

ShaderUniformDeclaration* ShaderUniformBufferDeclaration::FindUniform(const std::string& name)
{
	for (ShaderUniformDeclaration* uniform : m_Uniforms)
	{
		if (uniform->GetName() == name)
			return uniform;
	}
	return nullptr;
}

ShaderResourceDeclaration::ShaderResourceDeclaration(Type type, const std::string& name, uint32_t count)
	: m_Type(type), m_Name(name), m_Count(count)
{
	m_Name = name;
	m_Count = count;
}

ShaderResourceDeclaration::Type ShaderResourceDeclaration::StringToType(const std::string& type)
{
	if (type == "sampler2D")    return Type::TEXTURE2D;
	if (type == "sampler2DMS")  return Type::TEXTURE2D;
	if (type == "samplerCube")  return Type::TEXTURECUBE;

	return Type::NONE;
}

std::string ShaderResourceDeclaration::TypeToString(Type type)
{
	switch (type)
	{
	case Type::TEXTURE2D:	return "sampler2D";
	case Type::TEXTURECUBE:	return "samplerCube";
	}
	return "Invalid Type";
}