#include "ldpch.h"

#include "SceneHierarchy.h"

#include <imgui.h>

#include <assimp/scene.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "Lucid/Renderer/Mesh.h"

#include "Lucid/Core/Application.h"

#pragma region ImGui UI Helpers

static int s_UIContextID = 0;

static uint32_t s_Counter = 0;

static char s_IDBuffer[16];

static void PushID()
{
	ImGui::PushID(s_UIContextID++);
	s_Counter = 0;
}

static void PopID()
{
	ImGui::PopID();
	s_UIContextID--;
}

static void BeginPropertyGrid()
{
	PushID();
	ImGui::Columns(2);
}

static bool Property(const char* label, std::string& value, bool error = false)
{
	bool modified = false;

	ImGui::Text(label);
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);

	char buffer[256];
	strcpy(buffer, value.c_str());

	s_IDBuffer[0] = '#';
	s_IDBuffer[1] = '#';
	memset(s_IDBuffer + 2, 0, 14);
	itoa(s_Counter++, s_IDBuffer + 2, 16);

	if (error)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
	}

	if (ImGui::InputText(s_IDBuffer, buffer, 256))
	{
		value = buffer;
		modified = true;
	}

	if (error)
	{
		ImGui::PopStyleColor();
	}

	ImGui::PopItemWidth();
	ImGui::NextColumn();

	return modified;
}

static void Property(const char* label, const char* value)
{
	ImGui::Text(label);
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);

	s_IDBuffer[0] = '#';
	s_IDBuffer[1] = '#';
	memset(s_IDBuffer + 2, 0, 14);
	itoa(s_Counter++, s_IDBuffer + 2, 16);

	ImGui::InputText(s_IDBuffer, (char*)value, 256, ImGuiInputTextFlags_ReadOnly);

	ImGui::PopItemWidth();
	ImGui::NextColumn();
}

static bool Property(const char* label, bool& value)
{
	bool modified = false;

	ImGui::Text(label);
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);

	s_IDBuffer[0] = '#';
	s_IDBuffer[1] = '#';
	memset(s_IDBuffer + 2, 0, 14);
	itoa(s_Counter++, s_IDBuffer + 2, 16);

	if (ImGui::Checkbox(s_IDBuffer, &value))
	{
		modified = true;
	}

	ImGui::PopItemWidth();
	ImGui::NextColumn();

	return modified;
}

static bool Property(const char* label, int& value, float min = 0.0f, float max = 1.0f)
{
	bool modified = false;

	ImGui::Text(label);
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);

	s_IDBuffer[0] = '#';
	s_IDBuffer[1] = '#';
	memset(s_IDBuffer + 2, 0, 14);
	itoa(s_Counter++, s_IDBuffer + 2, 16);

	if (ImGui::DragInt(s_IDBuffer, &value, 0.1f, min, max))
	{
		modified = true;
	}

	ImGui::PopItemWidth();
	ImGui::NextColumn();

	return modified;
}

static bool Property(const char* label, float& value, float min = 0.0f, float max = 1.0f)
{
	bool modified = false;

	ImGui::Text(label);
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);

	s_IDBuffer[0] = '#';
	s_IDBuffer[1] = '#';
	memset(s_IDBuffer + 2, 0, 14);
	itoa(s_Counter++, s_IDBuffer + 2, 16);

	if (ImGui::DragFloat(s_IDBuffer, &value, 0.1f, min, max))
	{
		modified = true;
	}

	ImGui::PopItemWidth();
	ImGui::NextColumn();

	return modified;
}

static bool Property(const char* label, glm::vec2& value, float min = 0.0f, float max = 1.0f)
{
	bool modified = false;

	ImGui::Text(label);
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);

	s_IDBuffer[0] = '#';
	s_IDBuffer[1] = '#';
	memset(s_IDBuffer + 2, 0, 14);
	itoa(s_Counter++, s_IDBuffer + 2, 16);

	if (ImGui::DragFloat2(s_IDBuffer, glm::value_ptr(value), 0.1f, min, max))
	{
		modified = true;
	}

	ImGui::PopItemWidth();
	ImGui::NextColumn();

	return modified;
}

static bool Property(const char* label, glm::vec3& value, float min = 0.0f, float max = 1.0f)
{
	bool modified = false;

	ImGui::Text(label);
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);

	s_IDBuffer[0] = '#';
	s_IDBuffer[1] = '#';
	memset(s_IDBuffer + 2, 0, 14);
	itoa(s_Counter++, s_IDBuffer + 2, 16);

	if (ImGui::DragFloat3(s_IDBuffer, glm::value_ptr(value), 0.1f, min, max))
	{
		modified = true;
	}

	ImGui::PopItemWidth();
	ImGui::NextColumn();

	return modified;
}

static bool Property(const char* label, glm::vec4& value, float min = 0.0f, float max = 1.0f)
{
	bool modified = false;

	ImGui::Text(label);
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);

	s_IDBuffer[0] = '#';
	s_IDBuffer[1] = '#';
	memset(s_IDBuffer + 2, 0, 14);
	itoa(s_Counter++, s_IDBuffer + 2, 16);

	if (ImGui::DragFloat4(s_IDBuffer, glm::value_ptr(value), 0.1f, min, max))
	{
		modified = true;
	}

	ImGui::PopItemWidth();
	ImGui::NextColumn();

	return modified;
}

static bool PropertyColour(const char* label, glm::vec3& value)
{
	bool modified = false;

	ImGui::Text(label);
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);

	s_IDBuffer[0] = '#';
	s_IDBuffer[1] = '#';
	memset(s_IDBuffer + 2, 0, 14);
	itoa(s_Counter++, s_IDBuffer + 2, 16);

	if (ImGui::ColorEdit3(s_IDBuffer, glm::value_ptr(value)))
	{
		modified = true;
	}

	ImGui::PopItemWidth();
	ImGui::NextColumn();

	return modified;
}

static void EndPropertyGrid()
{
	ImGui::Columns(1);
	PopID();
}

#pragma endregion

glm::mat4 Mat4FromAssimpMat4(const aiMatrix4x4& matrix);

SceneHierarchy::SceneHierarchy(const Ref<Scene>& context)
	: m_Context(context)
{
}

void SceneHierarchy::SetContext(const Ref<Scene>& scene)
{
	m_Context = scene;
	m_SelectionContext = {};

	if (m_SelectionContext && false)
	{
		auto& entityMap = m_Context->GetEntityMap();

		LucidUUID selectedEntityID = m_SelectionContext.GetUUID();

		if (entityMap.find(selectedEntityID) != entityMap.end())
		{
			m_SelectionContext = entityMap.at(selectedEntityID);
		}
	}
}

void SceneHierarchy::SetSelected(Entity entity)
{
	m_SelectionContext = entity;
}

void SceneHierarchy::OnImGuiRender()
{
	ImGui::Begin("Scene Hierarchy");

	if (m_Context)
	{
		uint32_t entityCount = 0;
		uint32_t meshCount = 0;

		m_Context->m_Registry.each([&](auto entity)
		{
			Entity e(entity, m_Context.Raw());

			if (e.HasComponent<IDComponent>())
			{
				DrawEntityNode(e);
			}
		});

		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			if (ImGui::MenuItem("Create Empty Entity"))
			{
				m_Context->CreateEntity("Empty Entity");
			}

			ImGui::EndPopup();
		}

		ImGui::End();

		ImGui::Begin("Properties");

		if (m_SelectionContext)
		{
			DrawComponents(m_SelectionContext);

			if (ImGui::Button("Add Component"))
			{
				ImGui::OpenPopup("AddComponentPanel");
			}

			if (ImGui::BeginPopup("AddComponentPanel"))
			{
				if (!m_SelectionContext.HasComponent<LightComponent>())
				{
					if (ImGui::Button("Light"))
					{
						m_SelectionContext.AddComponent<LightComponent>();
						ImGui::CloseCurrentPopup();
					}
				}

				if (!m_SelectionContext.HasComponent<MeshComponent>())
				{
					if (ImGui::Button("Mesh"))
					{
						m_SelectionContext.AddComponent<MeshComponent>();
						ImGui::CloseCurrentPopup();
					}
				}

				ImGui::EndPopup();
			}
		}
	}

	ImGui::End();
}

void SceneHierarchy::DrawEntityNode(Entity entity)
{
	const char* name = "Unnamed Entity";

	if (entity.HasComponent<TagComponent>())
	{
		name = entity.GetComponent<TagComponent>().Tag.c_str();
	}

	ImGuiTreeNodeFlags node_flags = (entity == m_SelectionContext ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;

	bool opened = ImGui::TreeNodeEx((void*)(uint32_t)entity, node_flags, name);

	if (ImGui::IsItemClicked())
	{
		m_SelectionContext = entity;

		if (m_SelectionChangedCallback)
		{
			m_SelectionChangedCallback(m_SelectionContext);
		}
	}

	bool entityDeleted = false;

	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::MenuItem("Delete"))
		{
			entityDeleted = true;
		}

		ImGui::EndPopup();
	}

	if (opened)
	{
		if (entity.HasComponent<MeshComponent>())
		{
			auto mesh = entity.GetComponent<MeshComponent>().MeshComp;
		}

		ImGui::TreePop();
	}

	// Defer deletion until end of node UI
	if (entityDeleted)
	{
		m_Context->DestroyEntity(entity);

		if (entity == m_SelectionContext)
		{
			m_SelectionContext = {};
		}

		m_EntityDeletedCallback(entity);
	}
}

void SceneHierarchy::DrawMeshNode(const Ref<Mesh>& mesh, uint32_t& imguiMeshID)
{
	static char imguiName[128];

	memset(imguiName, 0, 128);

	sprintf(imguiName, "Mesh##%d", imguiMeshID++);

	// Mesh hierarchy
	if (ImGui::TreeNode(imguiName))
	{
		auto rootNode = mesh->m_Scene->mRootNode;

		MeshNodeHierarchy(mesh, rootNode);

		ImGui::TreePop();
	}
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

void SceneHierarchy::MeshNodeHierarchy(const Ref<Mesh>& mesh, aiNode* node, const glm::mat4& parentTransform, uint32_t level)
{
	glm::mat4 localTransform = Mat4FromAssimpMat4(node->mTransformation);
	glm::mat4 transform = parentTransform * localTransform;

	if (ImGui::TreeNode(node->mName.C_Str()))
	{
		{
			auto [translation, rotation, scale] = GetTransformDecomposition(transform);

			ImGui::Text("World Transform");
			ImGui::Text("  Translation: %.2f, %.2f, %.2f", translation.x, translation.y, translation.z);
			ImGui::Text("  Scale: %.2f, %.2f, %.2f", scale.x, scale.y, scale.z);
		}
		{
			auto [translation, rotation, scale] = GetTransformDecomposition(localTransform);

			ImGui::Text("Local Transform");
			ImGui::Text("  Translation: %.2f, %.2f, %.2f", translation.x, translation.y, translation.z);
			ImGui::Text("  Scale: %.2f, %.2f, %.2f", scale.x, scale.y, scale.z);
		}

		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			MeshNodeHierarchy(mesh, node->mChildren[i], transform, level + 1);
		}

		ImGui::TreePop();
	}
}

void SceneHierarchy::DrawComponents(Entity entity)
{
	ImGui::AlignTextToFramePadding();

	auto id = entity.GetComponent<IDComponent>().ID;

	if (entity.HasComponent<TagComponent>())
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;
		char buffer[256];

		memset(buffer, 0, 256);
		memcpy(buffer, tag.c_str(), tag.length());

		if (ImGui::InputText("##Tag", buffer, 256))
		{
			tag = std::string(buffer);
		}
	}

	// UUID
	ImGui::SameLine();
	ImGui::TextDisabled("%llx", id);

	ImGui::Separator();

	if (entity.HasComponent<TransformComponent>())
	{
		auto& tc = entity.GetComponent<TransformComponent>();

		if (ImGui::TreeNodeEx((void*)((uint32_t)entity | typeid(TransformComponent).hash_code()), ImGuiTreeNodeFlags_DefaultOpen, "Transform"))
		{
			auto [translation, rotationQuat, scale] = GetTransformDecomposition(tc);

			glm::vec3 rotation = glm::degrees(glm::eulerAngles(rotationQuat));

			ImGui::Columns(2);

			ImGui::Text("Translation");

			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);

			bool updateTransform = false;

			if (ImGui::DragFloat3("##translation", glm::value_ptr(translation), 0.25f))
			{
				updateTransform = true;
			}

			ImGui::PopItemWidth();
			ImGui::NextColumn();

			ImGui::Text("Rotation");

			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);

			if (ImGui::DragFloat3("##rotation", glm::value_ptr(rotation), 0.25f))
			{
				updateTransform = true;
			}

			ImGui::PopItemWidth();
			ImGui::NextColumn();

			ImGui::Text("Scale");

			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);

			if (ImGui::DragFloat3("##scale", glm::value_ptr(scale), 0.25f))
			{
				updateTransform = true;
			}

			ImGui::PopItemWidth();
			ImGui::NextColumn();

			ImGui::Columns(1);

			if (updateTransform)
			{
				tc.TransformComp = glm::translate(glm::mat4(1.0f), translation) * glm::toMat4(glm::quat(glm::radians(rotation))) * glm::scale(glm::mat4(1.0f), scale);
			}

			ImGui::TreePop();
		}

		ImGui::Separator();
	}


	if (entity.HasComponent<MeshComponent>())
	{
		auto& mc = entity.GetComponent<MeshComponent>();

		if (ImGui::TreeNodeEx((void*)((uint32_t)entity | typeid(TransformComponent).hash_code()), ImGuiTreeNodeFlags_DefaultOpen, "Mesh"))
		{
			ImGui::Columns(3);
			ImGui::SetColumnWidth(0, 100);
			ImGui::SetColumnWidth(1, 300);
			ImGui::SetColumnWidth(2, 40);

			ImGui::Text("File Path");

			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);

			if (mc.MeshComp)
			{
				ImGui::InputText("##meshfilepath", (char*)mc.MeshComp->GetFilePath().c_str(), 256, ImGuiInputTextFlags_ReadOnly);
			}
			else
			{
				ImGui::InputText("##meshfilepath", (char*)"Null", 256, ImGuiInputTextFlags_ReadOnly);
			}

			ImGui::PopItemWidth();
			ImGui::NextColumn();

			if (ImGui::Button("...##openmesh"))
			{
				std::string file = Application::Get().OpenFile();

				if (!file.empty())
				{
					mc.MeshComp = Ref<Mesh>::Create(file);
				}
			}

			ImGui::NextColumn();
			ImGui::Columns(1);
			ImGui::TreePop();
		}

		ImGui::Separator();
	}

	if (entity.HasComponent<LightComponent>())
	{
		auto& lc = entity.GetComponent<LightComponent>();

		if (ImGui::TreeNodeEx((void*)((uint32_t)entity | typeid(LightComponent).hash_code()), ImGuiTreeNodeFlags_DefaultOpen, "Light"))
		{
			// Light Type
			const char* lightTypeStrings[] = { "Point" };
			const char* currentType = lightTypeStrings[(int)lc.LightType];

			if (ImGui::BeginCombo("Light Type", currentType))
			{
				for (int type = 0; type < 1; type++)
				{
					bool is_selected = (currentType == lightTypeStrings[type]);

					if (ImGui::Selectable(lightTypeStrings[type], is_selected))
					{
						currentType = lightTypeStrings[type];
						lc.LightType = (LightComponent::Type)type;
					}

					if (is_selected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}

			BeginPropertyGrid();

			// Point light parameters
			if (lc.LightType == LightComponent::Type::Point)
			{
				ImGui::Columns(3);

				BeginPropertyGrid();

				Property("Brightness", lc.Brightness, 0.0f, 100.0f);
				PropertyColour("Diffuse", lc.Diffuse);

				EndPropertyGrid();

				ImGui::NextColumn();
				ImGui::Columns(1);
			}

			EndPropertyGrid();

			ImGui::NextColumn();
			ImGui::Columns(1);

			ImGui::TreePop();
		}

		ImGui::Separator();
	}
}