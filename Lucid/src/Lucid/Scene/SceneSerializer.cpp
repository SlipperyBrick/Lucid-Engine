#include "ldpch.h"

#include "SceneSerializer.h"

#include <yaml-cpp/yaml.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>

#include "Lucid/Scene/Entity.h"
#include "Lucid/Scene/Components.h"

#pragma region YAML-CPP Helpers

namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);

			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();

			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);

			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();

			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);

			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();

			return true;
		}
	};

	template<>
	struct convert<glm::quat>
	{
		static Node encode(const glm::quat& rhs)
		{
			Node node;
			node.push_back(rhs.w);
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);

			return node;
		}

		static bool decode(const Node& node, glm::quat& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
			{
				return false;
			}

			rhs.w = node[0].as<float>();
			rhs.x = node[1].as<float>();
			rhs.y = node[2].as<float>();
			rhs.z = node[3].as<float>();

			return true;
		}
	};
}

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;

	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;

	return out;
}


YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;

	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::quat& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.w << v.x << v.y << v.z << YAML::EndSeq;

	return out;
}

#pragma endregion

SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
	: m_Scene(scene)
{
}

static std::tuple<glm::vec3, glm::quat, glm::vec3> GetTransformDecomposition(const glm::mat4& transform)
{
	glm::vec3 scale;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::quat orientation;

	glm::decompose(transform, scale, orientation, translation, skew, perspective);

	return { translation, orientation, scale };
}

static void SerializeEntity(YAML::Emitter& out, Entity entity)
{
	LucidUUID uuid = entity.GetComponent<IDComponent>().ID;

	// Entity component
	out << YAML::BeginMap;
	out << YAML::Key << "Entity";

	out << YAML::Value << uuid;

	if (entity.HasComponent<TagComponent>())
	{
		// Tag component
		out << YAML::Key << "TagComponent";
		out << YAML::BeginMap;

		// Tag data
		auto& tag = entity.GetComponent<TagComponent>().Tag;
		out << YAML::Key << "Tag" << YAML::Value << tag;

		out << YAML::EndMap;
	}

	if (entity.HasComponent<TransformComponent>())
	{
		// Transform component
		out << YAML::Key << "TransformComponent";
		out << YAML::BeginMap;

		// Transform data
		auto& transform = entity.GetComponent<TransformComponent>().TransformComp;
		auto [pos, rot, scale] = GetTransformDecomposition(transform);
		out << YAML::Key << "Position" << YAML::Value << pos;
		out << YAML::Key << "Rotation" << YAML::Value << rot;
		out << YAML::Key << "Scale" << YAML::Value << scale;

		out << YAML::EndMap;
	}

	if (entity.HasComponent<MeshComponent>())
	{
		// Mesh component
		out << YAML::Key << "MeshComponent";
		out << YAML::BeginMap;

		// Mesh data
		auto mesh = entity.GetComponent<MeshComponent>().MeshComp;
		out << YAML::Key << "AssetPath" << YAML::Value << mesh->GetFilePath();

		out << YAML::EndMap;
	}

	if (entity.HasComponent<LightComponent>())
	{
		// Light component
		out << YAML::Key << "LightComponent";
		out << YAML::BeginMap;

		auto pointLights = entity.GetComponent<LightComponent>();

		// Point lights data
		out << YAML::Key << "Brightness" << YAML::Value << pointLights.Brightness;
		out << YAML::Key << "Falloff" << YAML::Value << pointLights.Falloff;
		out << YAML::Key << "Slope" << YAML::Value << pointLights.Slope;
		out << YAML::Key << "Diffuse" << YAML::Value << pointLights.Diffuse;
		out << YAML::Key << "Ambient" << YAML::Value << pointLights.Ambient;
		out << YAML::Key << "Specular" << YAML::Value << pointLights.Specular;

		out << YAML::EndMap;
	}

	out << YAML::EndMap;
}

static void SerializeEnvironment(YAML::Emitter& out, const Ref<Scene>& scene)
{
	// Directional lights
	out << YAML::Key << "Directional Light" << YAML::Value;
	out << YAML::BeginMap;

	const auto& dirLight = scene->GetDirectionalLight();

	// Directional lights data
	out << YAML::Key << "Direction" << YAML::Value << dirLight.Direction;
	out << YAML::Key << "Brightness" << YAML::Value << dirLight.Brightness;
	out << YAML::Key << "Diffuse" << YAML::Value << dirLight.Colour;
	out << YAML::Key << "Ambient" << YAML::Value << dirLight.Ambient;
	out << YAML::Key << "Specular" << YAML::Value << dirLight.Specular;

	out << YAML::EndMap;
}

void SceneSerializer::Serialize(const std::string& filepath)
{
	YAML::Emitter out;

	out << YAML::BeginMap;
	out << YAML::Key << "Scene";
	out << YAML::Value << "Scene Name";

	SerializeEnvironment(out, m_Scene);

	out << YAML::Key << "Entities";
	out << YAML::Value << YAML::BeginSeq;

	m_Scene->m_Registry.each([&](auto entityID)
	{
		Entity entity = { entityID, m_Scene.Raw() };

		if (!entity || !entity.HasComponent<IDComponent>())
		{
			return;
		}

		SerializeEntity(out, entity);

	});

	out << YAML::EndSeq;
	out << YAML::EndMap;

	std::ofstream fout(filepath);
	fout << out.c_str();
}

bool SceneSerializer::Deserialize(const std::string& filepath)
{
	std::ifstream stream(filepath);
	std::stringstream strStream;
	strStream << stream.rdbuf();

	YAML::Node data = YAML::Load(strStream.str());

	if (!data["Scene"])
	{
		return false;
	}

	std::string sceneName = data["Scene"].as<std::string>();
	LD_CORE_INFO("Deserializing scene '{0}'", sceneName);

	// Directional light
	auto directionalLight = data["Directional Light"];

	auto& dirLight = m_Scene->GetDirectionalLight();

	dirLight.Brightness = directionalLight["Brightness"].as<float>();
	dirLight.Direction = directionalLight["Direction"].as<glm::vec3>();
	dirLight.Colour = directionalLight["Diffuse"].as<glm::vec3>();
	dirLight.Ambient = directionalLight["Ambient"].as<glm::vec3>();
	dirLight.Specular = directionalLight["Specular"].as<glm::vec3>();

	m_Scene->m_Light = dirLight;

	// Scene data
	auto entities = data["Entities"];

	if (entities)
	{
		for (auto entity : entities)
		{
			uint32_t uuid = entity["Entity"].as<uint32_t>();

			std::string name;

			// Tag component
			auto tagComponent = entity["TagComponent"];

			if (tagComponent)
			{
				name = tagComponent["Tag"].as<std::string>();
			}

			LD_CORE_INFO("Deserialized entity with ID = {0}, name = {1}", uuid, name);

			Entity deserializedEntity = m_Scene->CreateEntityWithID(uuid, name);

			// Transform component
			auto transformComponent = entity["TransformComponent"];

			if (transformComponent)
			{
				auto& transform = deserializedEntity.GetComponent<TransformComponent>().TransformComp;

				glm::vec3 translation = transformComponent["Position"].as<glm::vec3>();
				glm::quat rotation = transformComponent["Rotation"].as<glm::quat>();
				glm::vec3 scale = transformComponent["Scale"].as<glm::vec3>();

				transform = glm::translate(glm::mat4(1.0f), translation) * glm::toMat4(rotation) * glm::scale(glm::mat4(1.0f), scale);

				LD_CORE_INFO("  Entity Transform:");
				LD_CORE_INFO("    Translation: {0}, {1}, {2}", translation.x, translation.y, translation.z);
				LD_CORE_INFO("    Rotation: {0}, {1}, {2}, {3}", rotation.w, rotation.x, rotation.y, rotation.z);
				LD_CORE_INFO("    Scale: {0}, {1}, {2}", scale.x, scale.y, scale.z);
			}

			// Mesh component
			auto meshComponent = entity["MeshComponent"];

			if (meshComponent)
			{
				std::string meshPath = meshComponent["AssetPath"].as<std::string>();

				if (!deserializedEntity.HasComponent<MeshComponent>())
				{
					deserializedEntity.AddComponent<MeshComponent>(Ref<Mesh>::Create(meshPath));
				}

				LD_CORE_INFO("  Mesh Asset Path: {0}", meshPath);
			}

			// Light component
			auto lightComponent = entity["LightComponent"];

			if (lightComponent)
			{
				if (!deserializedEntity.HasComponent<LightComponent>())
				{
					deserializedEntity.AddComponent<LightComponent>();

					auto& pointLight = m_Scene->GetLightEnvironment().PointLights;

					// Get the point lights translation
					auto& transform = deserializedEntity.GetComponent<TransformComponent>().TransformComp;
					glm::vec3 translation = transformComponent["Position"].as<glm::vec3>();
					pointLight->Position = translation;

					auto& pntLight = deserializedEntity.GetComponent<LightComponent>();
					
					pntLight.Brightness = lightComponent["Brightness"].as<float>();
					pntLight.Diffuse = lightComponent["Diffuse"].as<glm::vec3>();
					pntLight.Ambient = lightComponent["Ambient"].as<glm::vec3>();
					pntLight.Specular = lightComponent["Specular"].as<glm::vec3>();
					pntLight.Falloff = lightComponent["Falloff"].as<float>();
					pntLight.Slope = lightComponent["Slope"].as<float>();
				}

				// Could output data to console here about each light
			}
		}
	}

	return true;
}