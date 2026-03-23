#include "ImGuiSceneView.h"
#include "imgui.h"

namespace CgEngine {
    void ImGuiSceneView::renderSceneView(Scene& scene) {
        ImGui::SeparatorText("Entities");

        ImGuiWindowFlags entities_flags = ImGuiWindowFlags_HorizontalScrollbar;
        ImGui::BeginChild("entities", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y * 0.5f), ImGuiChildFlags_None, entities_flags);

        for (const auto &[entity, _] : scene.children) {
            if (scene.parents.find(entity) == scene.parents.end()) {
                std::optional<std::string> id = scene.getIdForEntity(entity);
                std::string tag = scene.getEntityTag(entity);

                renderEntityNode(scene, entity, scene.getChildren(entity).empty(), id, tag);
            }
        }

        ImGui::EndChild();

        if (selectedEntity != NoEntity && scene.hasEntity(selectedEntity)) {
            ImGui::SeparatorText("Entity Components");

            ImGuiWindowFlags components_flags = ImGuiWindowFlags_HorizontalScrollbar;
            ImGui::BeginChild("components", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None, components_flags);
            scene.componentManager.renderImGuiForEntity(selectedEntity);
            ImGui::EndChild();
        }
    }

    void ImGuiSceneView::renderEntityNode(Scene& scene, Entity entity, bool isLeave, const std::optional<std::string>& id, const std::string& tag) {
        ImGuiTreeNodeFlags flags = 0;
        if (isLeave) {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }
        if (selectedEntity == entity) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        if (!id.has_value() && tag.empty()) {
            if (ImGui::TreeNodeEx((void*)(intptr_t)(entity), flags, "%i", entity)) {
                if (ImGui::IsItemClicked()) {
                    selectedEntity = entity;
                }

                renderEntityChildNodes(scene, entity);
                ImGui::TreePop();
            }
        } else if (id.has_value() && tag.empty()) {
            if (ImGui::TreeNodeEx((void*)(intptr_t)(entity), flags, "%i (ID: %s)", entity, id.value().c_str())) {
                if (ImGui::IsItemClicked()) {
                    selectedEntity = entity;
                }

                renderEntityChildNodes(scene, entity);
                ImGui::TreePop();
            }
        } else if (!id.has_value() && !tag.empty()) {
            if (ImGui::TreeNodeEx((void*)(intptr_t)(entity), flags, "%i (Tag: %s)", entity, tag.c_str())) {
                if (ImGui::IsItemClicked()) {
                    selectedEntity = entity;
                }

                renderEntityChildNodes(scene, entity);
                ImGui::TreePop();
            }
        } else if (id.has_value() && !tag.empty()) {
            if (ImGui::TreeNodeEx((void*)(intptr_t)(entity), flags, "%i (ID: %s, Tag: %s)", entity, id.value().c_str(), tag.c_str())) {
                if (ImGui::IsItemClicked()) {
                    selectedEntity = entity;
                }

                renderEntityChildNodes(scene, entity);
                ImGui::TreePop();
            }
        }
    }

    void ImGuiSceneView::renderEntityChildNodes(Scene& scene, Entity entity) {
        for (const auto &child : scene.children[entity]) {
            std::optional<std::string> id = scene.getIdForEntity(child);
            std::string tag = scene.getEntityTag(child);

            renderEntityNode(scene, child, scene.getChildren(child).empty(), id, tag);
        }
    }
}
