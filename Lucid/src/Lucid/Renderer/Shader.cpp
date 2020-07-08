#pragma once

#include "ldpch.h"

#include "Shader.h"

#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <sstream>
#include <limits>

#include "Lucid/Renderer/Renderer.h"

std::vector<Ref<Shader>> Shader::s_AllShaders;

#pragma region Parsing Helper Functions

const char* FindToken(const char* str, const std::string& token)
{
	const char* t = str;

	while (t = strstr(t, token.c_str()))
	{
		bool left = str == t || isspace(t[-1]);
		bool right = !t[token.size()] || isspace(t[token.size()]);

		if (left && right)
		{
			return t;
		}

		t += token.size();
	}

	return nullptr;
}

const char* FindToken(const std::string& string, const std::string& token)
{
	return FindToken(string.c_str(), token);
}

std::vector<std::string> SplitString(const std::string& string, const std::string& delimiters)
{
	size_t start = 0;
	size_t end = string.find_first_of(delimiters);

	std::vector<std::string> result;

	while (end <= std::string::npos)
	{
		std::string token = string.substr(start, end - start);

		if (!token.empty())
		{
			result.push_back(token);
		}

		if (end == std::string::npos)
		{
			break;
		}

		start = end + 1;
		end = string.find_first_of(delimiters, start);
	}

	return result;
}

std::vector<std::string> SplitString(const std::string& string, const char delimiter)
{
	return SplitString(string, std::string(1, delimiter));
}

std::vector<std::string> Tokenize(const std::string& string)
{
	return SplitString(string, " \t\n");
}

std::vector<std::string> GetLines(const std::string& string)
{
	return SplitString(string, "\n");
}

std::string GetBlock(const char* str, const char** outPosition)
{
	const char* end = strstr(str, "}");

	if (!end)
	{
		return str;
	}

	if (outPosition)
	{
		*outPosition = end;
	}

	uint32_t length = end - str + 1;

	return std::string(str, length);
}

std::string GetStatement(const char* str, const char** outPosition)
{
	const char* end = strstr(str, ";");

	if (!end)
	{
		return str;
	}

	if (outPosition)
	{
		*outPosition = end;
	}
	
	uint32_t length = end - str + 1;

	return std::string(str, length);
}

bool StartsWith(const std::string& string, const std::string& start)
{
	return string.find(start) == 0;
}

#pragma endregion

Shader::Shader(const std::string& filepath)
	: m_AssetPath(filepath)
{
	size_t found = filepath.find_last_of("/\\");
	m_Name = found != std::string::npos ? filepath.substr(found + 1) : filepath;

	found = m_Name.find_last_of(".");
	m_Name = found != std::string::npos ? m_Name.substr(0, found) : m_Name;

	Reload();
}

Ref<Shader> Shader::Create(const std::string& filepath)
{
	Ref<Shader> result = nullptr;

	result = Ref<Shader>::Create(filepath);

	s_AllShaders.push_back(result);

	return result;
}

Ref<Shader> Shader::CreateFromString(const std::string& source)
{
	Ref<Shader> shader = Ref<Shader>();

	shader->Load(source);

	s_AllShaders.push_back(shader);

	return shader;
}

void Shader::Load(const std::string& source)
{
	m_ShaderSource = PreProcess(source);

	if (!m_IsCompute)
	{
		Parse();
	}

	Renderer::Submit([=]()
	{
		if (m_RendererID)
		{
			glDeleteProgram(m_RendererID);
		}

		CompileAndUploadShader();

		if (!m_IsCompute)
		{
			ResolveUniforms();
		}

		if (m_Loaded)
		{
			for (auto& callback : m_ShaderReloadedCallbacks)
			{
				callback();
			}
		}

		m_Loaded = true;
	});
}

void Shader::Reload()
{
	std::string source = ReadShaderFromFile(m_AssetPath);

	Load(source);
}

void Shader::AddShaderReloadedCallback(const ShaderReloadedCallback& callback)
{
	m_ShaderReloadedCallbacks.push_back(callback);
}

void Shader::Bind()
{
	Renderer::Submit([=]()
	{
		glUseProgram(m_RendererID);
	});
}

std::string Shader::ReadShaderFromFile(const std::string& filepath) const
{
	std::string result;

	std::ifstream in(filepath, std::ios::in | std::ios::binary);

	if (in)
	{
		in.seekg(0, std::ios::end);
		result.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&result[0], result.size());
		in.close();
	}
	else
	{
		LD_CORE_ASSERT(false, "Could not load shader!");
	}

	return result;
}

std::unordered_map<GLenum, std::string> Shader::PreProcess(const std::string& source)
{
	std::unordered_map<GLenum, std::string> shaderSources;

	const char* typeToken = "#type";

	size_t typeTokenLength = strlen(typeToken);
	size_t pos = source.find(typeToken, 0);

	while (pos != std::string::npos)
	{
		size_t eol = source.find_first_of("\r\n", pos);

		LD_CORE_ASSERT(eol != std::string::npos, "Syntax error");

		size_t begin = pos + typeTokenLength + 1;

		std::string type = source.substr(begin, eol - begin);

		LD_CORE_ASSERT(type == "vertex" || type == "fragment" || type == "compute", "Invalid shader type specified");

		size_t nextLinePos = source.find_first_not_of("\r\n", eol);

		pos = source.find(typeToken, nextLinePos);

		auto shaderType = ShaderTypeFromString(type);

		shaderSources[shaderType] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));

		// Compute shaders cannot contain other types
		if (shaderType == GL_COMPUTE_SHADER)
		{
			m_IsCompute = true;

			break;
		}
	}

	return shaderSources;
}

void Shader::Parse()
{
	const char* token;
	const char* vstr;
	const char* fstr;

	m_Resources.clear();
	m_Structs.clear();

	m_VSMaterialUniformBuffer.reset();
	m_FSMaterialUniformBuffer.reset();

	auto& vertexSource = m_ShaderSource[GL_VERTEX_SHADER];
	auto& fragmentSource = m_ShaderSource[GL_FRAGMENT_SHADER];

	// Vertex shader
	vstr = vertexSource.c_str();
	while (token = FindToken(vstr, "struct"))
	{
		ParseUniformStruct(GetBlock(token, &vstr), ShaderDomain::Vertex);
	}

	vstr = vertexSource.c_str();
	while (token = FindToken(vstr, "uniform"))
	{
		ParseUniform(GetStatement(token, &vstr), ShaderDomain::Vertex);
	}

	// Fragment shader
	fstr = fragmentSource.c_str();
	while (token = FindToken(fstr, "struct"))
	{
		ParseUniformStruct(GetBlock(token, &fstr), ShaderDomain::Fragment);
	}

	fstr = fragmentSource.c_str();
	while (token = FindToken(fstr, "uniform"))
	{
		ParseUniform(GetStatement(token, &fstr), ShaderDomain::Fragment);
	}
}

static bool IsTypeStringResource(const std::string& type)
{
	if (type == "sampler2D")
	{
		return true;
	}

	if (type == "sampler2DMS")
	{
		return true;
	}

	if (type == "samplerCube")
	{
		return true;
	}

	if (type == "sampler2DShadow")
	{
		return true;
	}

	return false;
}

ShaderStruct* Shader::FindStruct(const std::string& name)
{
	for (ShaderStruct* s : m_Structs)
	{
		if (s->GetName() == name)
		{
			return s;
		}
	}

	return nullptr;
}

void Shader::ParseUniform(const std::string& statement, ShaderDomain domain)
{
	std::vector<std::string> tokens = Tokenize(statement);
	uint32_t index = 0;

	// The "uniform" keyword
	index++;

	std::string typeString = tokens[index++];
	std::string name = tokens[index++];

	// Strip the semi-colon from the name (if present)
	if (const char* s = strstr(name.c_str(), ";"))
	{
		name = std::string(name.c_str(), s - name.c_str());
	}

	std::string n(name);

	int32_t count = 1;

	const char* namestr = n.c_str();

	if (const char* s = strstr(namestr, "["))
	{
		name = std::string(namestr, s - namestr);

		const char* end = strstr(namestr, "]");

		std::string c(s + 1, end - s);

		count = atoi(c.c_str());
	}

	if (IsTypeStringResource(typeString))
	{
		ShaderResourceDeclaration* declaration = new ShaderResourceDeclaration(ShaderResourceDeclaration::StringToType(typeString), name, count);
		m_Resources.push_back(declaration);
	}
	else
	{
		ShaderUniformDeclaration::Type t = ShaderUniformDeclaration::StringToType(typeString);
		ShaderUniformDeclaration* declaration = nullptr;

		if (t == ShaderUniformDeclaration::Type::NONE)
		{
			// Find struct
			ShaderStruct* s = FindStruct(typeString);

			LD_CORE_ASSERT(s, "");

			declaration = new ShaderUniformDeclaration(domain, s, name, count);
		}
		else
		{
			declaration = new ShaderUniformDeclaration(domain, t, name, count);
		}

		if (StartsWith(name, "r_"))
		{
			if (domain == ShaderDomain::Vertex)
			{
				((ShaderUniformBufferDeclaration*)m_VSRendererUniformBuffers.front())->PushUniform(declaration);
			}
			else if (domain == ShaderDomain::Fragment)
			{
				((ShaderUniformBufferDeclaration*)m_FSRendererUniformBuffers.front())->PushUniform(declaration);
			}
		}
		else
		{
			if (domain == ShaderDomain::Vertex)
			{
				if (!m_VSMaterialUniformBuffer)
				{
					m_VSMaterialUniformBuffer.reset(new ShaderUniformBufferDeclaration("", domain));
				}

				m_VSMaterialUniformBuffer->PushUniform(declaration);
			}
			else if (domain == ShaderDomain::Fragment)
			{
				if (!m_FSMaterialUniformBuffer)
				{
					m_FSMaterialUniformBuffer.reset(new ShaderUniformBufferDeclaration("", domain));
				}

				m_FSMaterialUniformBuffer->PushUniform(declaration);
			}
		}
	}
}

void Shader::ParseUniformStruct(const std::string& block, ShaderDomain domain)
{
	std::vector<std::string> tokens = Tokenize(block);

	uint32_t index = 0;

	// The "struct" keyword
	index++;

	std::string name = tokens[index++];

	// Strip the semi-colon from the name (if present)
	if (const char* s = strstr(name.c_str(), "\r"))
	{
		name = std::string(name.c_str(), s - name.c_str());
	}

	ShaderStruct* uniformStruct = new ShaderStruct(name);

	// The opening curly brace
	index++;

	while (index < tokens.size())
	{
		if (tokens[index] == "}")
		{
			break;
		}

		if (tokens[index] == "\r" || tokens[index] == "\n")
		{
			index++;
		}

		std::string type = tokens[index++];
		std::string name = tokens[index++];

		// Strip the semi-colon from the name (if present)
		if (const char* s = strstr(name.c_str(), ";"))
		{
			name = std::string(name.c_str(), s - name.c_str());
		}

		uint32_t count = 1;

		const char* namestr = name.c_str();

		if (const char* s = strstr(namestr, "["))
		{
			name = std::string(namestr, s - namestr);

			const char* end = strstr(namestr, "]");

			std::string c(s + 1, end - s);

			count = atoi(c.c_str());
		}

		ShaderUniformDeclaration* field = new ShaderUniformDeclaration(domain, ShaderUniformDeclaration::StringToType(type), name, count);

		uniformStruct->AddField(field);
	}

	m_Structs.push_back(uniformStruct);
}

void Shader::ResolveUniforms()
{
	glUseProgram(m_RendererID);

	for (size_t i = 0; i < m_VSRendererUniformBuffers.size(); i++)
	{
		ShaderUniformBufferDeclaration* decl = (ShaderUniformBufferDeclaration*)m_VSRendererUniformBuffers[i];

		const ShaderUniformList& uniforms = decl->GetUniformDeclarations();

		for (size_t j = 0; j < uniforms.size(); j++)
		{
			ShaderUniformDeclaration* uniform = (ShaderUniformDeclaration*)uniforms[j];

			if (uniform->GetType() == ShaderUniformDeclaration::Type::STRUCT)
			{
				const ShaderStruct& s = uniform->GetShaderUniformStruct();
				const auto& fields = s.GetFields();

				for (size_t k = 0; k < fields.size(); k++)
				{
					ShaderUniformDeclaration* field = (ShaderUniformDeclaration*)fields[k];

					field->m_Location = GetUniformLocation(uniform->m_Name + "." + field->m_Name);
				}
			}
			else
			{
				uniform->m_Location = GetUniformLocation(uniform->m_Name);
			}
		}
	}

	for (size_t i = 0; i < m_FSRendererUniformBuffers.size(); i++)
	{
		ShaderUniformBufferDeclaration* decl = (ShaderUniformBufferDeclaration*)m_FSRendererUniformBuffers[i];

		const ShaderUniformList& uniforms = decl->GetUniformDeclarations();

		for (size_t j = 0; j < uniforms.size(); j++)
		{
			ShaderUniformDeclaration* uniform = (ShaderUniformDeclaration*)uniforms[j];

			if (uniform->GetType() == ShaderUniformDeclaration::Type::STRUCT)
			{
				const ShaderStruct& s = uniform->GetShaderUniformStruct();
				const auto& fields = s.GetFields();

				for (size_t k = 0; k < fields.size(); k++)
				{
					ShaderUniformDeclaration* field = (ShaderUniformDeclaration*)fields[k];

					field->m_Location = GetUniformLocation(uniform->m_Name + "." + field->m_Name);
				}
			}
			else
			{
				uniform->m_Location = GetUniformLocation(uniform->m_Name);
			}
		}
	}

	{
		const auto& decl = m_VSMaterialUniformBuffer;

		if (decl)
		{
			const ShaderUniformList& uniforms = decl->GetUniformDeclarations();

			for (size_t j = 0; j < uniforms.size(); j++)
			{
				ShaderUniformDeclaration* uniform = (ShaderUniformDeclaration*)uniforms[j];

				if (uniform->GetType() == ShaderUniformDeclaration::Type::STRUCT)
				{
					const ShaderStruct& s = uniform->GetShaderUniformStruct();
					const auto& fields = s.GetFields();

					for (size_t k = 0; k < fields.size(); k++)
					{
						ShaderUniformDeclaration* field = (ShaderUniformDeclaration*)fields[k];

						field->m_Location = GetUniformLocation(uniform->m_Name + "." + field->m_Name);
					}
				}
				else
				{
					uniform->m_Location = GetUniformLocation(uniform->m_Name);
				}
			}
		}
	}

	{
		const auto& decl = m_FSMaterialUniformBuffer;

		if (decl)
		{
			const ShaderUniformList& uniforms = decl->GetUniformDeclarations();

			for (size_t j = 0; j < uniforms.size(); j++)
			{
				ShaderUniformDeclaration* uniform = (ShaderUniformDeclaration*)uniforms[j];

				if (uniform->GetType() == ShaderUniformDeclaration::Type::STRUCT)
				{
					const ShaderStruct& s = uniform->GetShaderUniformStruct();
					const auto& fields = s.GetFields();

					for (size_t k = 0; k < fields.size(); k++)
					{
						ShaderUniformDeclaration* field = (ShaderUniformDeclaration*)fields[k];

						field->m_Location = GetUniformLocation(uniform->m_Name + "." + field->m_Name);
					}
				}
				else
				{
					uniform->m_Location = GetUniformLocation(uniform->m_Name);
				}
			}
		}
	}

	uint32_t sampler = 0;

	for (size_t i = 0; i < m_Resources.size(); i++)
	{
		ShaderResourceDeclaration* resource = (ShaderResourceDeclaration*)m_Resources[i];

		int32_t location = GetUniformLocation(resource->m_Name);

		if (resource->GetCount() == 1)
		{
			resource->m_Register = sampler;

			if (location != -1)
			{
				UploadUniformInt(location, sampler);
			}

			sampler++;
		}
		else if (resource->GetCount() > 1)
		{
			resource->m_Register = 0;

			uint32_t count = resource->GetCount();

			int* samplers = new int[count];

			for (uint32_t s = 0; s < count; s++)
			{
				samplers[s] = s;
			}

			UploadUniformIntArray(resource->GetName(), samplers, count);

			delete[] samplers;
		}
	}
}

int32_t Shader::GetUniformLocation(const std::string& name) const
{
	int32_t result = glGetUniformLocation(m_RendererID, name.c_str());

	if (result == -1)
	{
		LD_CORE_WARN("Could not find uniform '{0}' in shader", name);
	}

	return result;
}

GLenum Shader::ShaderTypeFromString(const std::string& type)
{
	if (type == "vertex")
	{
		return GL_VERTEX_SHADER;
	}

	if (type == "fragment")
	{
		return GL_FRAGMENT_SHADER;
	}

	if (type == "compute")
	{
		return GL_COMPUTE_SHADER;
	}

	return GL_NONE;
}

void Shader::CompileAndUploadShader()
{
	std::vector<GLuint> shaderRendererIDs;

	GLuint program = glCreateProgram();

	for (auto& kv : m_ShaderSource)
	{
		GLenum type = kv.first;
		std::string& source = kv.second;

		GLuint shaderRendererID = glCreateShader(type);

		const GLchar* sourceCstr = (const GLchar*)source.c_str();
		glShaderSource(shaderRendererID, 1, &sourceCstr, 0);

		glCompileShader(shaderRendererID);

		GLint isCompiled = 0;
		glGetShaderiv(shaderRendererID, GL_COMPILE_STATUS, &isCompiled);

		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(shaderRendererID, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(shaderRendererID, maxLength, &maxLength, &infoLog[0]);

			LD_CORE_ERROR("Shader compilation failed:\n{0}", &infoLog[0]);

			// Clean up, no longer need the shader anymore
			glDeleteShader(shaderRendererID);

			LD_CORE_ASSERT(false, "Failed");
		}

		shaderRendererIDs.push_back(shaderRendererID);
		glAttachShader(program, shaderRendererID);
	}

	// Link our program
	glLinkProgram(program);

	// Check if program has successfully linked
	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);

	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

		LD_CORE_ERROR("Shader compilation failed:\n{0}", &infoLog[0]);

		// Clean up, no longer need the program anymore
		glDeleteProgram(program);

		// Clean up, no longer need the shaders anymore
		for (auto id : shaderRendererIDs)
		{
			glDeleteShader(id);
		}
	}

	// Clean up, detach all shaders
	for (auto id : shaderRendererIDs)
	{
		glDetachShader(program, id);
	}

	m_RendererID = program;
}

void Shader::SetVSMaterialUniformBuffer(Memory buffer)
{
	Renderer::Submit([this, buffer]()
	{
		glUseProgram(m_RendererID);

		ResolveAndSetUniforms(m_VSMaterialUniformBuffer, buffer);
	});
}

void Shader::SetFSMaterialUniformBuffer(Memory buffer)
{
	Renderer::Submit([this, buffer]()
	{
		glUseProgram(m_RendererID);

		ResolveAndSetUniforms(m_FSMaterialUniformBuffer, buffer);
	});
}

void Shader::ResolveAndSetUniforms(const Scope<ShaderUniformBufferDeclaration>& decl, Memory buffer)
{
	const ShaderUniformList& uniforms = decl->GetUniformDeclarations();

	for (size_t i = 0; i < uniforms.size(); i++)
	{
		ShaderUniformDeclaration* uniform = (ShaderUniformDeclaration*)uniforms[i];

		if (uniform->IsArray())
		{
			ResolveAndSetUniformArray(uniform, buffer);
		}
		else
		{
			ResolveAndSetUniform(uniform, buffer);
		}
	}
}

void Shader::ResolveAndSetUniform(ShaderUniformDeclaration* uniform, Memory buffer)
{
	if (uniform->GetLocation() == -1)
	{
		return;
	}

	LD_CORE_ASSERT(uniform->GetLocation() != -1, "Uniform has invalid location!");

	uint32_t offset = uniform->GetOffset();

	switch (uniform->GetType())
	{
		case ShaderUniformDeclaration::Type::FLOAT32:
		{
			UploadUniformFloat(uniform->GetLocation(), *(float*)&buffer.Data[offset]);

			break;
		}
		case ShaderUniformDeclaration::Type::INT32:
		{
			UploadUniformInt(uniform->GetLocation(), *(int32_t*)&buffer.Data[offset]);

			break;
		}
		case ShaderUniformDeclaration::Type::VEC2:
		{
			UploadUniformFloat2(uniform->GetLocation(), *(glm::vec2*) & buffer.Data[offset]);

			break;
		}
		case ShaderUniformDeclaration::Type::VEC3:
		{
			UploadUniformFloat3(uniform->GetLocation(), *(glm::vec3*) & buffer.Data[offset]);

			break;
		}
		case ShaderUniformDeclaration::Type::VEC4:
		{
			UploadUniformFloat4(uniform->GetLocation(), *(glm::vec4*) & buffer.Data[offset]);

			break;
		}
		case ShaderUniformDeclaration::Type::MAT3:
		{
			UploadUniformMat3(uniform->GetLocation(), *(glm::mat3*) & buffer.Data[offset]);

			break;
		}
		case ShaderUniformDeclaration::Type::MAT4:
		{
			UploadUniformMat4(uniform->GetLocation(), *(glm::mat4*) & buffer.Data[offset]);

			break;
		}
		case ShaderUniformDeclaration::Type::STRUCT:
		{
			UploadUniformStruct(uniform, buffer.Data, offset);

			break;
		}
		default:
		{
			LD_CORE_ASSERT(false, "Unknown uniform type!");
		}
	}
}

void Shader::ResolveAndSetUniformArray(ShaderUniformDeclaration* uniform, Memory buffer)
{
	LD_CORE_ASSERT(uniform->GetLocation() != -1, "Uniform has invalid location!");

	uint32_t offset = uniform->GetOffset();

	switch (uniform->GetType())
	{
		case ShaderUniformDeclaration::Type::FLOAT32:
		{
			UploadUniformFloat(uniform->GetLocation(), *(float*)&buffer.Data[offset]);

			break;
		}
		case ShaderUniformDeclaration::Type::INT32:
		{
			UploadUniformInt(uniform->GetLocation(), *(int32_t*)&buffer.Data[offset]);

			break;
		}
		case ShaderUniformDeclaration::Type::VEC2:
		{
			UploadUniformFloat2(uniform->GetLocation(), *(glm::vec2*) & buffer.Data[offset]);

			break;
		}
		case ShaderUniformDeclaration::Type::VEC3:
		{
			UploadUniformFloat3(uniform->GetLocation(), *(glm::vec3*) & buffer.Data[offset]);

			break;
		}
		case ShaderUniformDeclaration::Type::VEC4:
		{
			UploadUniformFloat4(uniform->GetLocation(), *(glm::vec4*) & buffer.Data[offset]);

			break;
		}
		case ShaderUniformDeclaration::Type::MAT3:
		{
			UploadUniformMat3(uniform->GetLocation(), *(glm::mat3*) & buffer.Data[offset]);

			break;
		}
		case ShaderUniformDeclaration::Type::MAT4:
		{
			UploadUniformMat4Array(uniform->GetLocation(), *(glm::mat4*) & buffer.Data[offset], uniform->GetCount());

			break;
		}
		case ShaderUniformDeclaration::Type::STRUCT:
		{
			UploadUniformStruct(uniform, buffer.Data, offset);

			break;
		}
		default:
		{
			LD_CORE_ASSERT(false, "Unknown uniform type!");
		}
	}
}

void Shader::ResolveAndSetUniformField(const ShaderUniformDeclaration& field, byte* data, int32_t offset)
{
	switch (field.GetType())
	{
		case ShaderUniformDeclaration::Type::FLOAT32:
		{
			UploadUniformFloat(field.GetLocation(), *(float*)&data[offset]);

			break;
		}
		case ShaderUniformDeclaration::Type::INT32:
		{
			UploadUniformInt(field.GetLocation(), *(int32_t*)&data[offset]);

			break;
		}
		case ShaderUniformDeclaration::Type::VEC2:
		{
			UploadUniformFloat2(field.GetLocation(), *(glm::vec2*) & data[offset]);

			break;
		}
		case ShaderUniformDeclaration::Type::VEC3:
		{
			UploadUniformFloat3(field.GetLocation(), *(glm::vec3*) & data[offset]);

			break;
		}
		case ShaderUniformDeclaration::Type::VEC4:
		{
			UploadUniformFloat4(field.GetLocation(), *(glm::vec4*) & data[offset]);

			break;
		}
		case ShaderUniformDeclaration::Type::MAT3:
		{
			UploadUniformMat3(field.GetLocation(), *(glm::mat3*) & data[offset]);

			break;
		}
		case ShaderUniformDeclaration::Type::MAT4:
		{
			UploadUniformMat4(field.GetLocation(), *(glm::mat4*) & data[offset]);

			break;
		}
		default:
		{
			LD_CORE_ASSERT(false, "Unknown uniform type!");
		}
	}
}

void Shader::UploadUniformBuffer(const UniformBufferBase& uniformBuffer)
{
	for (unsigned int i = 0; i < uniformBuffer.GetUniformCount(); i++)
	{
		const UniformDecl& decl = uniformBuffer.GetUniforms()[i];

		switch (decl.Type)
		{
			case UniformType::Float:
			{
				const std::string& name = decl.Name;

				float value = *(float*)(uniformBuffer.GetBuffer() + decl.Offset);

				Renderer::Submit([=]()
				{
					UploadUniformFloat(name, value);
				});
			}
			case UniformType::Float3:
			{
				const std::string& name = decl.Name;

				glm::vec3& values = *(glm::vec3*)(uniformBuffer.GetBuffer() + decl.Offset);

				Renderer::Submit([=]()
				{
					UploadUniformFloat3(name, values);
				});
			}
			case UniformType::Float4:
			{
				const std::string& name = decl.Name;

				glm::vec4& values = *(glm::vec4*)(uniformBuffer.GetBuffer() + decl.Offset);

				Renderer::Submit([=]()
				{
					UploadUniformFloat4(name, values);
				});
			}
			case UniformType::Matrix4x4:
			{
				const std::string& name = decl.Name;

				glm::mat4& values = *(glm::mat4*)(uniformBuffer.GetBuffer() + decl.Offset);

				Renderer::Submit([=]()
				{
					UploadUniformMat4(name, values);
				});
			}
		}
	}
}

void Shader::SetFloat(const std::string& name, float value)
{
	Renderer::Submit([=]()
	{
		UploadUniformFloat(name, value);
	});
}

void Shader::SetInt(const std::string& name, int value)
{
	Renderer::Submit([=]()
	{
		UploadUniformInt(name, value);
	});
}

void Shader::SetMat4(const std::string& name, const glm::mat4& value)
{
	Renderer::Submit([=]()
	{
		UploadUniformMat4(name, value);
	});
}

void Shader::SetMat4FromRenderThread(const std::string& name, const glm::mat4& value, bool bind)
{
	if (bind)
	{
		UploadUniformMat4(name, value);
	}
	else
	{
		int location = glGetUniformLocation(m_RendererID, name.c_str());

		if (location != -1)
		{
			UploadUniformMat4(location, value);
		}
	}
}

void Shader::SetIntArray(const std::string& name, int* values, uint32_t size)
{
	Renderer::Submit([=]()
	{
		UploadUniformIntArray(name, values, size);
	});
}

void Shader::UploadUniformInt(uint32_t location, int32_t value)
{
	glUniform1i(location, value);
}

void Shader::UploadUniformIntArray(uint32_t location, int32_t* values, int32_t count)
{
	glUniform1iv(location, count, values);
}

void Shader::UploadUniformFloat(uint32_t location, float value)
{
	glUniform1f(location, value);
}

void Shader::UploadUniformFloat2(uint32_t location, const glm::vec2& value)
{
	glUniform2f(location, value.x, value.y);
}

void Shader::UploadUniformFloat3(uint32_t location, const glm::vec3& value)
{
	glUniform3f(location, value.x, value.y, value.z);
}

void Shader::UploadUniformFloat4(uint32_t location, const glm::vec4& value)
{
	glUniform4f(location, value.x, value.y, value.z, value.w);
}

void Shader::UploadUniformMat3(uint32_t location, const glm::mat3& value)
{
	glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::UploadUniformMat4(uint32_t location, const glm::mat4& value)
{
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::UploadUniformMat4Array(uint32_t location, const glm::mat4& values, uint32_t count)
{
	glUniformMatrix4fv(location, count, GL_FALSE, glm::value_ptr(values));
}

void Shader::UploadUniformStruct(ShaderUniformDeclaration* uniform, byte* buffer, uint32_t offset)
{
	const ShaderStruct& s = uniform->GetShaderUniformStruct();

	const auto& fields = s.GetFields();

	for (size_t k = 0; k < fields.size(); k++)
	{
		ShaderUniformDeclaration* field = (ShaderUniformDeclaration*)fields[k];

		ResolveAndSetUniformField(*field, buffer, offset);

		offset += field->m_Size;
	}
}

void Shader::UploadUniformInt(const std::string& name, int32_t value)
{
	int32_t location = GetUniformLocation(name);
	glUniform1i(location, value);
}

void Shader::UploadUniformIntArray(const std::string& name, int32_t* values, uint32_t count)
{
	int32_t location = GetUniformLocation(name);
	glUniform1iv(location, count, values);
}

void Shader::UploadUniformFloat(const std::string& name, float value)
{
	glUseProgram(m_RendererID);

	auto location = glGetUniformLocation(m_RendererID, name.c_str());

	if (location != -1)
	{
		glUniform1f(location, value);
	}
	else
	{
		LD_LOG_UNIFORM("Uniform '{0}' not found!", name);
	}
}

void Shader::UploadUniformFloat2(const std::string& name, const glm::vec2& values)
{
	glUseProgram(m_RendererID);

	auto location = glGetUniformLocation(m_RendererID, name.c_str());

	if (location != -1)
	{
		glUniform2f(location, values.x, values.y);
	}
	else
	{
		LD_LOG_UNIFORM("Uniform '{0}' not found!", name);
	}
}

void Shader::UploadUniformFloat3(const std::string& name, const glm::vec3& values)
{
	glUseProgram(m_RendererID);

	auto location = glGetUniformLocation(m_RendererID, name.c_str());

	if (location != -1)
	{
		glUniform3f(location, values.x, values.y, values.z);
	}
	else
	{
		LD_LOG_UNIFORM("Uniform '{0}' not found!", name);
	}
}

void Shader::UploadUniformFloat4(const std::string& name, const glm::vec4& values)
{
	glUseProgram(m_RendererID);

	auto location = glGetUniformLocation(m_RendererID, name.c_str());

	if (location != -1)
	{
		glUniform4f(location, values.x, values.y, values.z, values.w);
	}
	else
	{
		LD_LOG_UNIFORM("Uniform '{0}' not found!", name);
	}
}

void Shader::UploadUniformMat4(const std::string& name, const glm::mat4& values)
{
	glUseProgram(m_RendererID);

	auto location = glGetUniformLocation(m_RendererID, name.c_str());

	if (location != -1)
	{
		glUniformMatrix4fv(location, 1, GL_FALSE, (const float*)&values);
	}
	else
	{
		LD_LOG_UNIFORM("Uniform '{0}' not found!", name);
	}
}