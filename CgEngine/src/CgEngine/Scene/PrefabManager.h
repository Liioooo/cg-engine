#pragma once

#include "Entity.h"
#include "Scene.h"
#include "XMLFile.h"

namespace CgEngine {

    class PrefabManager {
        friend class SceneLoader;
    public:
        static Entity instantiatePrefab(Scene* scene, const std::string& prefabName, Entity parent, glm::vec3 position = {0.0f, 0.0f, 0.0f}, glm::vec3 rotation = {0.0f, 0.0f, 0.0f}, glm::vec3 scale = {1.0f, 1.0f, 1.0f}, const std::string& tag = "", const std::string& id = "");

    private:
        static const pugi::xml_node getPrefabDefinitionXMLNode(const std::string& prefabName);
        static Entity createPrefabEntity(Scene* scene, Entity parent, const pugi::xml_node& prefabDefinitionNode, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, const std::string& tag, const std::string& id);

        static void instantiatePrefabFromNode(Scene* scene, const pugi::xml_node& prefabNode, Entity parent);

        static inline XMLFile xmlPrefabFile;
    };

}
