#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "Lucid/Renderer/VertexArray.h"
#include "Lucid/Renderer/VertexBuffer.h"
#include "Lucid/Renderer/Shader.h"
#include "Lucid/Renderer/Material.h"

#include "Lucid/Core/Math/AABB.h"

struct aiNode;
struct aiScene;

namespace Assimp
{
	class Importer;
}

struct Vertex
{
	glm::vec3 Position; 
	glm::vec3 Normal;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;
	glm::vec2 TexCoord;
};

static const int NumAttributes = 5;

struct Index
{
	uint32_t V1;
	uint32_t V2;
	uint32_t V3;
};

static_assert(sizeof(Index) == 3 * sizeof(uint32_t));

struct Triangle
{
	Vertex V0;
	Vertex V1;
	Vertex V2;

	Triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
		: V0(v0), V1(v1), V2(v2) {}
};

class Submesh
{

public:

	uint32_t BaseVertex;
	uint32_t BaseIndex;
	uint32_t MaterialIndex;
	uint32_t IndexCount;

	glm::mat4 Transform;

	AABB BoundingBox;

	std::string NodeName;
	std::string MeshName;
};

class Mesh : public RefCounted
{

public:

	Mesh(const std::string& filename);
	~Mesh();

	void DumpVertexBuffer();

	std::vector<Submesh>& GetSubmeshes() { return m_Submeshes; }
	const std::vector<Submesh>& GetSubmeshes() const { return m_Submeshes; }

	Ref<Shader> GetMeshShader() { return m_MeshShader; }
	Ref<Material> GetMaterial() { return m_BaseMaterial; }

	std::vector<Ref<MaterialInstance>> GetMaterials() { return m_Materials; }

	const std::vector<Ref<Texture2D>>& GetTextures() const { return m_Textures; }

	const std::string& GetFilePath() const { return m_FilePath; }

	const std::vector<Triangle> GetTriangleCache(uint32_t index) const { return m_TriangleCache.at(index); }

private:

	void TraverseNodes(aiNode* node, const glm::mat4& parentTransform = glm::mat4(1.0f), uint32_t level = 0);

private:

	std::vector<Submesh> m_Submeshes;

	std::unique_ptr<Assimp::Importer> m_Importer;

	glm::mat4 m_InverseTransform;

	Ref<VertexArray> m_VertexArray;

	std::vector<Vertex> m_Vertices;
	std::vector<Index> m_Indices;

	const aiScene* m_Scene;

	// Materials
	Ref<Shader> m_MeshShader;
	Ref<Material> m_BaseMaterial;

	std::vector<Ref<Texture2D>> m_Textures;
	std::vector<Ref<Texture2D>> m_NormalMaps;

	std::vector<Ref<MaterialInstance>> m_Materials;

	std::unordered_map<uint32_t, std::vector<Triangle>> m_TriangleCache;

	std::string m_FilePath;

	friend class Renderer;
	friend class SceneHierarchy;
};