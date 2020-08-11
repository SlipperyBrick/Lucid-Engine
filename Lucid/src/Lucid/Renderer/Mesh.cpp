#include "ldpch.h"

#include "Mesh.h"

#include <filesystem>

#include <glad/glad.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

#include <imgui/imgui.h>

#include "Lucid/Renderer/Renderer.h"

glm::mat4 Mat4FromAssimpMat4(const aiMatrix4x4& matrix)
{
	glm::mat4 result;

	// The a, b, c, d in assimp is the row within the matrix, the 1, 2, 3, 4 is the column
	result[0][0] = matrix.a1; result[1][0] = matrix.a2; result[2][0] = matrix.a3; result[3][0] = matrix.a4;
	result[0][1] = matrix.b1; result[1][1] = matrix.b2; result[2][1] = matrix.b3; result[3][1] = matrix.b4;
	result[0][2] = matrix.c1; result[1][2] = matrix.c2; result[2][2] = matrix.c3; result[3][2] = matrix.c4;
	result[0][3] = matrix.d1; result[1][3] = matrix.d2; result[2][3] = matrix.d3; result[3][3] = matrix.d4;

	return result;
}

static const uint32_t s_MeshImportFlags =
aiProcess_CalcTangentSpace |        // Generate bitangents/tangents if mesh has none
aiProcess_Triangulate |             // Triangulate meshes that aren't already triangulated
aiProcess_SortByPType |             // Split all meshes by primitive type
aiProcess_GenNormals |              // Generate normals if mesh has none
aiProcess_GenUVCoords |             // Generate UV coordinates if mesh has none
aiProcess_OptimizeMeshes |          // Batch meshes into single draw calls 
aiProcess_ValidateDataStructure;    // Valid mesh structure

// Create a new logger for logging assimp logstream data
struct LogStream : public Assimp::LogStream
{
	static void Initialize()
	{
		if (Assimp::DefaultLogger::isNullLogger())
		{
			Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
			Assimp::DefaultLogger::get()->attachStream(new LogStream, Assimp::Logger::Err | Assimp::Logger::Warn);
		}
	}

	virtual void write(const char* message) override
	{
		LD_CORE_ERROR("Assimp error: {0}", message);
	}
};

Mesh::Mesh(const std::string& filename)
	: m_FilePath(filename)
{
	LogStream::Initialize();

	LD_CORE_INFO("Loading mesh: {0}", filename.c_str());

	m_Importer = std::make_unique<Assimp::Importer>();

	const aiScene* scene = m_Importer->ReadFile(filename, s_MeshImportFlags);

	// Get file extension
	size_t found = m_FilePath.find_last_of(".");
	std::string fileExtension = found != std::string::npos ? m_FilePath.substr(found, std::string::npos) : m_FilePath;

	// Check if the scene has any meshes
	if (!scene || !scene->HasMeshes())
	{
		LD_CORE_ERROR("Failed to load mesh file: {0}", filename);
	}

	m_Scene = scene;

	m_MeshShader = Renderer::GetShaderLibrary()->Get("Buffer");
	m_BaseMaterial = Ref<Material>::Create(m_MeshShader);

	m_InverseTransform = glm::inverse(Mat4FromAssimpMat4(scene->mRootNode->mTransformation));

	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;

	m_Submeshes.reserve(scene->mNumMeshes);

	// For every submesh
	for (size_t m = 0; m < scene->mNumMeshes; m++)
	{
		aiMesh* mesh = scene->mMeshes[m];

		Submesh& submesh = m_Submeshes.emplace_back();

		submesh.BaseVertex = vertexCount;
		submesh.BaseIndex = indexCount;
		submesh.MaterialIndex = mesh->mMaterialIndex;
		submesh.IndexCount = mesh->mNumFaces * 3;
		submesh.MeshName = mesh->mName.C_Str();

		vertexCount += mesh->mNumVertices;
		indexCount += submesh.IndexCount;

		LD_CORE_ASSERT(mesh->HasPositions(), "Meshes require positions.");
		LD_CORE_ASSERT(mesh->HasNormals(), "Meshes require normals.");

		auto& aabb = submesh.BoundingBox;
		aabb.Min = { FLT_MAX, FLT_MAX, FLT_MAX };
		aabb.Max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

		// For ever vertex
		for (size_t i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;

			// Store vertex positions and normals
			vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
			vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

			// Set the minimum and maximum extends of the meshes AABB based off the vertex positions
			aabb.Min.x = glm::min(vertex.Position.x, aabb.Min.x);
			aabb.Min.y = glm::min(vertex.Position.y, aabb.Min.y);
			aabb.Min.z = glm::min(vertex.Position.z, aabb.Min.z);
			aabb.Max.x = glm::max(vertex.Position.x, aabb.Max.x);
			aabb.Max.y = glm::max(vertex.Position.y, aabb.Max.y);
			aabb.Max.z = glm::max(vertex.Position.z, aabb.Max.z);

			// Check if the vertex has tangents and bitangents
			if (mesh->HasTangentsAndBitangents())
			{
				vertex.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
				vertex.Bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
			}

			// Check if the mesh has texture coordinates
			if (mesh->HasTextureCoords(0))
			{
				vertex.TexCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
			}

			// Push the vertex back to the vertices list
			m_Vertices.push_back(vertex);
		}

		// For every face
		for (size_t i = 0; i < mesh->mNumFaces; i++)
		{
			LD_CORE_ASSERT(mesh->mFaces[i].mNumIndices == 3, "Must have 3 indices.");

			// Create indices for each face of the model, a single index describes a single face
			Index index = { mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] };

			// Push the indices back to the index list
			m_Indices.push_back(index);

			m_TriangleCache[m].emplace_back(m_Vertices[index.V1 + submesh.BaseVertex], m_Vertices[index.V2 + submesh.BaseVertex], m_Vertices[index.V3 + submesh.BaseVertex]);
		}
	}

	// Recursively traverse the meshes node hierarchy
	TraverseNodes(scene->mRootNode);

	// Check if materials exist
	if (scene->HasMaterials())
	{
		aiTextureType type;

		LD_MESH_LOG("---- Materials - {0} ----", filename);

		m_Textures.resize(scene->mNumMaterials);
		m_Materials.resize(scene->mNumMaterials);

		// For every material
		for (uint32_t i = 0; i < scene->mNumMaterials; i++)
		{
			auto aiMaterial = scene->mMaterials[i];
			auto aiMaterialName = aiMaterial->GetName();

			auto mi = Ref<MaterialInstance>::Create(m_BaseMaterial);

			m_Materials[i] = mi;

			LD_MESH_LOG("  {0} (Index = {1})", aiMaterialName.data, i);

			aiString aiTexPath;

			uint32_t textureCount = aiMaterial->GetTextureCount(aiTextureType_DIFFUSE);

			LD_MESH_LOG("    TextureCount = {0}", textureCount);

			aiColor3D aiColour;
			aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColour);

			float specular = 1.0f;
			float shininess = 1.0f;
			float glossiness = 1.0f;

			if (aiMaterial->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS)
			{
				specular = 1.0f - glm::sqrt(shininess / 100.0f);
			}

			if (aiMaterial->Get(AI_MATKEY_SHININESS_STRENGTH, glossiness) == AI_SUCCESS)
			{
				shininess = 1.0f - glm::sqrt(glossiness / 100.0f);
			}

			LD_MESH_LOG("    COLOUR = {0}, {1}, {2}", aiColour.r, aiColour.g, aiColour.b);
			LD_MESH_LOG("    SPECULARITY = {0}", specular);

			bool hasDiffuseMap = aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &aiTexPath) == AI_SUCCESS;

			if (hasDiffuseMap)
			{
				std::filesystem::path path = filename;

				auto parentPath = path.parent_path();
				parentPath /= std::string(aiTexPath.data);

				std::string texturePath = parentPath.string();

				LD_MESH_LOG("    Diffuse map path = {0}", texturePath);

				auto texture = Texture2D::Create(texturePath, true);

				if (texture->Loaded())
				{
					m_Textures[i] = texture;
					
					mi->Set("u_DiffuseTexture", m_Textures[i]);
					mi->Set("u_DiffuseTexToggle", 1.0f);
				}
				else
				{
					LD_CORE_ERROR("Could not load texture: {0}", texturePath);

					// Fallback to diffuse colour
					mi->Set("u_DiffuseColour", glm::vec3{ aiColour.r, aiColour.g, aiColour.b });
				}
			}
			else
			{
				mi->Set("u_DiffuseColour", glm::vec3{ aiColour.r, aiColour.g, aiColour.b });

				LD_MESH_LOG("    No diffuse map");
			}

			// Normal maps
			mi->Set("u_NormalTexToggle", 0.0f);

			// Check what file format mesh is for correctly setting normals type
			if (fileExtension == ".fbx")
			{
				type = aiTextureType_NORMALS;
			}
			else if (fileExtension == ".obj")
			{
				type = aiTextureType_HEIGHT;
			}

			if (aiMaterial->GetTexture(type, 0, &aiTexPath) == AI_SUCCESS)
			{
				std::filesystem::path path = filename;

				auto parentPath = path.parent_path();
				parentPath /= std::string(aiTexPath.data);

				std::string texturePath = parentPath.string();

				LD_MESH_LOG("    Normal map path = {0}", texturePath);

				auto texture = Texture2D::Create(texturePath);

				if (texture->Loaded())
				{
					mi->Set("u_NormalTexture", texture);
					mi->Set("u_NormalTexToggle", 1.0f);
				}
				else
				{
					LD_CORE_ERROR("    Could not load texture: {0}", texturePath);
				}
			}
			else
			{
				LD_MESH_LOG("    No normal map");
			}

			// Specular map
			if (aiMaterial->GetTexture(aiTextureType_SPECULAR, 0, &aiTexPath) == AI_SUCCESS)
			{
				std::filesystem::path path = filename;

				auto parentPath = path.parent_path();
				parentPath /= std::string(aiTexPath.data);

				std::string texturePath = parentPath.string();

				LD_MESH_LOG("    Specular map path = {0}", texturePath);

				auto texture = Texture2D::Create(texturePath);

				if (texture->Loaded())
				{
					mi->Set("u_SpecularTexture", texture);
					mi->Set("u_SpecularTexToggle", 1.0f);
				}
				else
				{
					LD_CORE_ERROR("    Could not load texture: {0}", texturePath);
				}
			}
			else
			{
				LD_MESH_LOG("    No specular map");

				mi->Set("u_Specular", specular);
			}

			// Gloss map
			if (aiMaterial->GetTexture(aiTextureType_SHININESS, 0, &aiTexPath) == AI_SUCCESS)
			{
				std::filesystem::path path = filename;

				auto parentPath = path.parent_path();
				parentPath /= std::string(aiTexPath.data);

				std::string texturePath = parentPath.string();

				LD_MESH_LOG("    Gloss map path = {0}", texturePath);

				auto texture = Texture2D::Create(texturePath);

				if (texture->Loaded())
				{
					mi->Set("u_GlossTexture", texture);
					mi->Set("u_GlossTexToggle", 1.0f);
				}
				else
				{
					LD_CORE_ERROR("    Could not load texture: {0}", texturePath);
				}
			}
			else
			{
				LD_MESH_LOG("    No gloss map");

				mi->Set("u_Gloss", glossiness);
			}
		}

		LD_MESH_LOG("------------------------");
	}

	m_VertexArray = VertexArray::Create();

	auto vb = VertexBuffer::Create(m_Vertices.data(), m_Vertices.size() * sizeof(Vertex));

	vb->SetLayout
	({
		{ ShaderDataType::Float3, "a_Position" },
		{ ShaderDataType::Float3, "a_Normal" },
		{ ShaderDataType::Float3, "a_Tangent" },
		{ ShaderDataType::Float3, "a_Bitangent" },
		{ ShaderDataType::Float2, "a_TexCoord" },
	});

	m_VertexArray->AddVertexBuffer(vb);

	auto ib = IndexBuffer::Create(m_Indices.data(), m_Indices.size() * sizeof(Index));
	m_VertexArray->SetIndexBuffer(ib);
}

Mesh::~Mesh()
{
}

static std::string LevelToSpaces(uint32_t level)
{
	std::string result = "";

	for (uint32_t i = 0; i < level; i++)
	{
		result += "--";
	}

	return result;
}

void Mesh::TraverseNodes(aiNode* node, const glm::mat4& parentTransform, uint32_t level)
{
	glm::mat4 transform = parentTransform * Mat4FromAssimpMat4(node->mTransformation);

	for (uint32_t i = 0; i < node->mNumMeshes; i++)
	{
		uint32_t mesh = node->mMeshes[i];
		auto& submesh = m_Submeshes[mesh];
		submesh.NodeName = node->mName.C_Str();
		submesh.Transform = transform;
	}

	LD_MESH_LOG("{0} {1}", LevelToSpaces(level), node->mName.C_Str());

	for (uint32_t i = 0; i < node->mNumChildren; i++)
	{
		TraverseNodes(node->mChildren[i], transform, level + 1);
	}
}

void Mesh::DumpVertexBuffer()
{
	LD_MESH_LOG("------------------------------------------------------");
	LD_MESH_LOG("Vertex Buffer Dump");
	LD_MESH_LOG("Mesh: {0}", m_FilePath);

	for (size_t i = 0; i < m_Vertices.size(); i++)
	{
		auto& vertex = m_Vertices[i];

		LD_MESH_LOG("Vertex: {0}", i);
		LD_MESH_LOG("Position: {0}, {1}, {2}", vertex.Position.x, vertex.Position.y, vertex.Position.z);
		LD_MESH_LOG("Normal: {0}, {1}, {2}", vertex.Normal.x, vertex.Normal.y, vertex.Normal.z);
		LD_MESH_LOG("Binormal: {0}, {1}, {2}", vertex.Bitangent.x, vertex.Bitangent.y, vertex.Bitangent.z);
		LD_MESH_LOG("Tangent: {0}, {1}, {2}", vertex.Tangent.x, vertex.Tangent.y, vertex.Tangent.z);
		LD_MESH_LOG("TexCoord: {0}, {1}", vertex.TexCoord.x, vertex.TexCoord.y);
		LD_MESH_LOG("--");
	}

	LD_MESH_LOG("------------------------------------------------------");
}