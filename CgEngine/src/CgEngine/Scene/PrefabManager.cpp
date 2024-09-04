#include "PrefabManager.h"
#include "Application.h"
#include "FileSystem.h"
#include "SceneLoader.h"
#include "LoaderUtils.h"

namespace CgEngine {
    Entity PrefabManager::instantiatePrefab(Scene* scene, const std::string& prefabName, Entity parent, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, const std::string& tag, const std::string& id) {
        auto& prefabDefinitionNode = getPrefabDefinitionXMLNode(prefabName);
        return createPrefabEntity(scene, parent, prefabDefinitionNode, position, rotation, scale, tag, id);
    }

    const pugi::xml_node& PrefabManager::getPrefabDefinitionXMLNode(const std::string& prefabName) {
        const auto& xmlPrefabFile = Application::get().getResourceManager().getResource<XMLFile>(FileSystem::getAsGamePath("prefabs.xml"));
        const pugi::xml_document& xml = xmlPrefabFile->getXMLDocument();
        const auto& prefabNode =  xml.child("Prefabs").find_child_by_attribute("Prefab", "name", prefabName.c_str());

        CG_ASSERT(!prefabNode.empty(), "No Prefab " + prefabName + " defined.")

        return prefabNode;
    }

    Entity PrefabManager::createPrefabEntity(Scene* scene, Entity parent, const pugi::xml_node& prefabDefinitionNode, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, const std::string& tag, const std::string& id) {
        Entity entity = id.empty() ? scene->createEntity(parent) : scene->createEntity(parent, id);
        if (!tag.empty()) {
            scene->setEntityTag(entity, tag);
        }

        const auto& componentsNode = prefabDefinitionNode.child("Components");

        CG_ASSERT(!componentsNode.empty(), "Prefab " + std::string(prefabDefinitionNode.attribute("name").as_string()) + " doesn't have <Components>")
        CG_ASSERT(componentsNode.child("TransformComponent").empty(), "Prefabs cannot have a TransformComponent in its definition.")

        scene->attachComponent<TransformComponent>(entity, TransformComponentParams{position, rotation, scale});

        for (const auto &compNode: componentsNode.children()) {
            SceneLoader::createComponent(scene, entity, compNode);
        }

        for (const auto &child: prefabDefinitionNode.children("Entity")) {
            SceneLoader::createEntity(scene, entity, child);
        }

        for (const auto &child: prefabDefinitionNode.children("Prefab")) {
            instantiatePrefabFromNode(scene, child, entity);
        }

        return entity;
    }

    void PrefabManager::instantiatePrefabFromNode(Scene* scene, const pugi::xml_node& prefabNode, Entity parent) {
        glm::vec3 position = {0.0f, 0.0f, 0.0f};
        glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
        glm::vec3 scale = {1.0f, 1.0f, 1.0f};
        std::string tag, id;

        if (!prefabNode.attribute("position").empty()) position = LoaderUtils::stringTupleToVec3(prefabNode.attribute("position").as_string());
        if (!prefabNode.attribute("rotation").empty()) rotation = glm::radians(LoaderUtils::stringTupleToVec3(prefabNode.attribute("rotation").as_string()));
        if (!prefabNode.attribute("scale").empty()) scale = LoaderUtils::stringTupleToVec3(prefabNode.attribute("scale").as_string());
        if (!prefabNode.attribute("tag").empty()) tag = prefabNode.attribute("tag").as_string();
        if (!prefabNode.attribute("id").empty()) id = prefabNode.attribute("id").as_string();

        instantiatePrefab(scene, prefabNode.attribute("name").as_string(), parent, position, rotation, scale, tag, id);
    }
}
