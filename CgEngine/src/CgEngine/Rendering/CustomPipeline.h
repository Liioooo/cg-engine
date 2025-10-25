#pragma once

#include "XMLFile.h"
#include "GraphicsPipeline.h"
#include "ComputePipeline.h"

namespace CgEngine {

    namespace CustomPipelinesData {
        static inline XMLFile pipelinesXMLFile;
        const pugi::xml_document& getPipelinesXMLFile();
    }

    namespace CustomPipelinesLoaderUtils {
        DepthCompareOperator depthCompareOperatorFromString(std::string_view s);
        BlendingEquation blendingEquationFromString(std::string_view s);
        BlendingFunction blendingFunctionFromString(std::string_view s);
        DrawMode drawModeFromString(std::string_view s);
        ShaderDataType shaderDataTypeFromString(std::string_view s);
    }

    struct CustomGraphicsPipelineSpecification {
        DepthCompareOperator depthCompareOperator = DepthCompareOperator::Less;
        bool depthTest = true;
        bool depthWrite = true;
        bool wireframe = false;
        bool backfaceCulling = true;
        bool frontfaceCulling = false;
        bool useBlending = false;
        BlendingEquation blendingEquation = BlendingEquation::Add;
        BlendingFunction srcBlendingFunction = BlendingFunction::SrcAlpha;
        BlendingFunction destBlendingFunction = BlendingFunction::OneMinusSrcAlpha;
        int tesselationPatchSize = ~0;
        DrawMode drawMode = DrawMode::Triangles;
        GraphicsPipelineCustomShaders shaders = {};
        std::vector<VertexBufferLayout> vertexBufferLayouts = {};
        DescriptorSetLayoutSpecification descriptorSetLayoutSpecification = {};
    };

    class CustomGraphicsPipeline {
    public:
        static CustomGraphicsPipeline* createResource(const std::string& name);

        explicit CustomGraphicsPipeline(const CustomGraphicsPipelineSpecification& spec);
        ~CustomGraphicsPipeline();

        CustomGraphicsPipeline(CustomGraphicsPipeline&& other) noexcept;
        CustomGraphicsPipeline& operator=(CustomGraphicsPipeline&& other) noexcept;

        CustomGraphicsPipeline(CustomGraphicsPipeline& other) = delete;
        CustomGraphicsPipeline& operator=(CustomGraphicsPipeline& other) = delete;

        const GraphicsPipeline* getGraphicsPipeline() const;
        const DescriptorSetLayout* getDescriptorSetLayout() const;

    private:
        GraphicsPipeline* graphicsPipeline = nullptr;
        DescriptorSetLayout* descriptorSetLayout = nullptr;
    };

    struct CustomComputePipelineSpecification {
        std::string shader;
        DescriptorSetLayoutSpecification descriptorSetLayoutSpecification = {};
    };

    class CustomComputePipeline {
    public:
        static CustomComputePipeline* createResource(const std::string& name);

        explicit CustomComputePipeline(const CustomComputePipelineSpecification& spec);
        ~CustomComputePipeline();

        CustomComputePipeline(CustomComputePipeline&& other) noexcept;
        CustomComputePipeline& operator=(CustomComputePipeline&& other) noexcept;

        CustomComputePipeline(CustomComputePipeline& other) = delete;
        CustomComputePipeline& operator=(CustomComputePipeline& other) = delete;

        const ComputePipeline* getComputePipeline() const;
        const DescriptorSetLayout* getDescriptorSetLayout() const;

    private:
        ComputePipeline* computePipeline = nullptr;
        DescriptorSetLayout* descriptorSetLayout = nullptr;

    };
}
