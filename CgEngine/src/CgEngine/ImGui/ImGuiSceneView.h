#pragma once

#include "Scene/Scene.h"

namespace CgEngine {

    class ImGuiSceneView {
    public:
        static void renderSceneView(Scene& scene);
        static void renderEntityNode(Scene& scene, Entity entity, bool isLeave, const std::optional<std::string>& id, const std::string& tag);
        static void renderEntityChildNodes(Scene& scene, Entity entity);

    private:
        static inline Entity selectedEntity = NoEntity;
    };

}
