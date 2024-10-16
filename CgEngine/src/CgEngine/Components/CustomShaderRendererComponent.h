#pragma once

#include "Rendering/RenderPass.h"
#include "Rendering/CustomShader.h"
#include "Rendering/CustomValMaterial.h"
#include "Resources/Mesh.h"
#include "Resources/CustomMesh.h"
#include "Rendering/PBRMaterial.h"
#include "Component.h"
#include "Resources/ResRef.h"

namespace CgEngine {

    struct CustomShaderRendererComponentRenderPassOptions {
        bool useDirShadowMappingData = false;
        bool useEnvironmentMappingData = false;
        bool backfaceCulling = true;
        bool frontfaceCulling = false;
        bool useBlending = false;
        BlendingEquation blendingEquation = BlendingEquation::Add;
        BlendingFunction srcBlendingFunction = BlendingFunction::SrcAlpha;
        BlendingFunction destBlendingFunction = BlendingFunction::OneMinusSrcAlpha;
    };

    struct CustomShaderRendererComponentParams {
        std::string assetFile;
        std::string mesh;
        CustomMesh* customMesh = nullptr;
        Material* customMaterial = nullptr;
        std::string material;
        bool enableCulling = true;
        glm::vec3 boundingMin = glm::vec3(0.0f);
        glm::vec3 boundingMax = glm::vec3(0.0f);
        std::vector<std::string> meshNodes;
        uint32_t instanceCount = 1;
        std::string shader;
        CustomShaderRendererComponentRenderPassOptions renderPassOptions;

        void verifyParams() const;
    };

    class CustomShaderRendererComponent : public Component {
    public:
        using Component::Component;

        void onAttach(Scene& scene, CustomShaderRendererComponentParams& params);

        ResRef<MeshVertices> getMeshVertices();
        CustomMesh* getCustomMesh();
        Mesh* getRenderMesh();
        ResRef<PBRMaterial> getPBRMaterial();
        Material* getCustomMaterial();
        Material* getRenderMaterial();
        bool getCullingEnabled() const;
        void setCullingEnabled(bool value);
        const std::vector<uint32_t>& getMeshNodes();
        void setInstanceCount(uint32_t value);
        uint32_t getInstanceCount() const;
        ResRef<CustomShader> getShader();

        void setCustomMesh(CustomMesh* mesh);
        void setCustomMaterial(Material* material);
        CustomShaderRendererComponentRenderPassOptions& getRenderPassOptions();
        AABoundingBox& getBoundingBox();

    private:
        ResRef<MeshVertices> mesh;
        CustomMesh* customMesh = nullptr;
        ResRef<PBRMaterial> pbrMaterial;
        Material* customMaterial = nullptr;
        bool enableCulling;
        std::vector<uint32_t> meshNodes;
        uint32_t instanceCount;
        ResRef<CustomShader> shader;
        CustomShaderRendererComponentRenderPassOptions renderPassOptions;
        AABoundingBox boundingBox;
    };

}
