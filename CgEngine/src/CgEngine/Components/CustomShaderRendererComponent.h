#pragma once

#include "Resources/MeshVertices.h"
#include "Resources/CustomMesh.h"
#include "Component.h"
#include "Resources/ResRef.h"
#include "Rendering/CustomPipeline.h"

namespace CgEngine {

    struct CustomShaderRendererComponentParams {
        std::string assetFile;
        std::string mesh;
        CustomMesh* customMesh = nullptr;
        bool enableCulling = true;
        glm::vec3 boundingMin = glm::vec3(0.0f);
        glm::vec3 boundingMax = glm::vec3(0.0f);
        std::vector<std::string> meshNodes;
        uint32_t instanceCount = 1;
        std::string pipeline;
        DescriptorSet* descriptorSet = nullptr;

        void verifyParams() const;
    };

    class CustomShaderRendererComponent : public Component {
    public:
        using Component::Component;
        using Params = CustomShaderRendererComponentParams;

        void onAttach(Scene& scene, CustomShaderRendererComponentParams& params);
        void onRenderImGui() override;

        ResRef<MeshVertices> getMeshVertices();
        CustomMesh* getCustomMesh();
        Mesh* getRenderMesh();
        bool getCullingEnabled() const;
        void setCullingEnabled(bool value);
        const std::vector<uint32_t>& getMeshNodes();
        void setInstanceCount(uint32_t value);
        uint32_t getInstanceCount() const;

        void setCustomMesh(CustomMesh* mesh);
        const AABoundingBox* getBoundingBox();
        void addBoundingBoxCoordinates(glm::vec3 max, glm::vec3 min);
        void setBoundingBoxCenterAndExtents(glm::vec3 center, glm::vec3 extents);

        bool isActive() const;
        void setActive(bool a);

        ResRef<CustomGraphicsPipeline> getPipeline();

        void setDescriptorSet(DescriptorSet* set);
        const DescriptorSet* getDescriptorSet() const;

    private:
        ResRef<MeshVertices> mesh;
        CustomMesh* customMesh = nullptr;
        bool enableCulling;
        std::vector<uint32_t> meshNodes;
        uint32_t instanceCount;
        AABoundingBox boundingBox;
        bool active = true;
        ResRef<CustomGraphicsPipeline> pipeline;
        DescriptorSet* descriptorSet = nullptr;
    };

}
