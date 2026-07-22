#include <FileSystem.h>
#include <pugixml.hpp>
#include <Asserts.h>
#include <Application.h>
#include "CustomPipeline.h"
#include "GraphicsObjectsFactory.h"
#include "CgEngineSharedUtils/StringUtils.h"

namespace CgEngine {
    const pugi::xml_document& CustomPipelinesData::getPipelinesXMLFile() {
        if (!CustomPipelinesData::pipelinesXMLFile.isLoaded()) {
            CustomPipelinesData::pipelinesXMLFile.load(FileSystem::getAsGamePath("pipelines.xml"));
        }

        return CustomPipelinesData::pipelinesXMLFile.getXMLDocument();
    }

    DepthCompareOperator CustomPipelinesLoaderUtils::depthCompareOperatorFromString(std::string_view s) {
        if (s == "Never") {
            return DepthCompareOperator::Never;
        } else if (s == "Less") {
            return DepthCompareOperator::Less;
        } else if (s == "Equal") {
            return DepthCompareOperator::Equal;
        } else if (s == "LessOrEqual") {
            return DepthCompareOperator::LessOrEqual;
        } else if (s == "Greater") {
            return DepthCompareOperator::Greater;
        } else if (s == "NotEqual") {
            return DepthCompareOperator::NotEqual;
        } else if (s == "GreaterOrEqual") {
            return DepthCompareOperator::GreaterOrEqual;
        } else if (s == "Always") {
            return DepthCompareOperator::Always;
        } else {
            CG_ASSERT(false, "CustomGraphicsPipeline::depthCompareOperatorFromString: Unknown DepthCompareOperator string value.")
            return DepthCompareOperator::Less;
        }
    }

    BlendingEquation CustomPipelinesLoaderUtils::blendingEquationFromString(std::string_view s) {
        if (s == "Add") {
            return BlendingEquation::Add;
        } else if (s == "Subtract") {
            return BlendingEquation::Subtract;
        } else if (s == "ReverseSubtract") {
            return BlendingEquation::ReverseSubtract;
        } else if (s == "Min") {
            return BlendingEquation::Min;
        } else if (s == "Max") {
            return BlendingEquation::Max;
        } else {
            CG_ASSERT(false, "CustomGraphicsPipeline::blendingEquationFromString: Unknown BlendingEquation string value.")
            return BlendingEquation::Add;
        }
    }

    BlendingFunction CustomPipelinesLoaderUtils::blendingFunctionFromString(std::string_view s) {
        if (s == "Zero") {
            return BlendingFunction::Zero;
        } else if (s == "One") {
            return BlendingFunction::One;
        } else if (s == "SrcColor") {
            return BlendingFunction::SrcColor;
        } else if (s == "OneMinusSrcColor") {
            return BlendingFunction::OneMinusSrcColor;
        } else if (s == "DestColor") {
            return BlendingFunction::DestColor;
        } else if (s == "OneMinusDestColor") {
            return BlendingFunction::OneMinusDestColor;
        } else if (s == "SrcAlpha") {
            return BlendingFunction::SrcAlpha;
        } else if (s == "OneMinusSrcAlpha") {
            return BlendingFunction::OneMinusSrcAlpha;
        } else if (s == "DestAlpha") {
            return BlendingFunction::DestAlpha;
        } else if (s == "OneMinusDestAlpha") {
            return BlendingFunction::OneMinusDestAlpha;
        } else {
            CG_ASSERT(false, "CustomGraphicsPipeline::blendingFunctionFromString: Unknown BlendingFunction string value.")
            return BlendingFunction::One;
        }
    }

    DrawMode CustomPipelinesLoaderUtils::drawModeFromString(std::string_view s) {
        if (s == "Triangles") {
            return DrawMode::Triangles;
        } else if (s == "Lines") {
            return DrawMode::Lines;
        } else if (s == "Patches") {
            return DrawMode::Patches;
        } else {
            CG_ASSERT(false, "CustomGraphicsPipeline::drawModeFromString: Unknown DrawMode string value.")
            return DrawMode::Triangles;
        }
    }

    ShaderDataType CustomPipelinesLoaderUtils::shaderDataTypeFromString(std::string_view s) {
        if (s == "Float") {
            return ShaderDataType::Float;
        } else if (s == "Float2") {
            return ShaderDataType::Float2;
        } else if (s == "Float3") {
            return ShaderDataType::Float3;
        } else if (s == "Float4") {
            return ShaderDataType::Float4;
        }  else if (s == "Int") {
            return ShaderDataType::Int;
        } else if (s == "Int2") {
            return ShaderDataType::Int2;
        } else if (s == "Int3") {
            return ShaderDataType::Int3;
        } else if (s == "Int4") {
            return ShaderDataType::Int4;
        } else {
            CG_ASSERT(false, "CustomGraphicsPipeline::shaderDataTypeFromString: Unknown ShaderDataType string value.")
            return ShaderDataType::Float;
        }
    }

    DescriptorSetLayoutBindingUsage CustomPipelinesLoaderUtils::descriptorSetLayoutBindingUsageFromString(std::string_view s) {
        if (s == "Compute") {
            return DescriptorSetLayoutBindingUsage::Compute;
        } else if (s == "TCS") {
            return DescriptorSetLayoutBindingUsage::TCS;
        } else if (s == "TES") {
            return DescriptorSetLayoutBindingUsage::TES;
        } else if (s == "Geometry") {
            return DescriptorSetLayoutBindingUsage::Geometry;
        } else if (s == "Fragment") {
            return DescriptorSetLayoutBindingUsage::Fragment;
        } else if (s == "Vertex") {
            return DescriptorSetLayoutBindingUsage::Vertex;
        } else {
            CG_ASSERT(false, "CustomGraphicsPipeline::descriptorSetLayoutBindingUsageFromString: Unknown DescriptorSetLayoutBindingUsage string value.")
            return DescriptorSetLayoutBindingUsage::None;
        }
    }

    std::vector<DescriptorSetLayoutBinding> CustomPipelinesLoaderUtils::descriptorSetLayoutBindingsFromNode(const pugi::xml_node& bindingPointsNode) {
        std::vector<DescriptorSetLayoutBinding> bindings;

        for (const auto& bindingNode : bindingPointsNode.children("Binding")) {
            DescriptorSetLayoutBinding binding;
            binding.bindingPoint = bindingNode.attribute("bindingPoint").as_uint();
            binding.descriptorCount = bindingNode.attribute("descriptorCount").as_uint(1);

            for (const auto& usageToken : StringUtils::splitString(bindingNode.attribute("usage").as_string(), ',')) {
                binding.usage |= descriptorSetLayoutBindingUsageFromString(usageToken);
            }

            bindings.push_back(binding);
        }

        return bindings;
    }

    CustomGraphicsPipeline* CustomGraphicsPipeline::createResource(const std::string& name) {
        const pugi::xml_document& xml = CustomPipelinesData::getPipelinesXMLFile();
        const auto& pipelineNode =  xml.child("Pipelines").find_child_by_attribute("GraphicsPipeline", "name", name.c_str());

        CustomGraphicsPipelineSpecification spec{};

        std::string_view depthCompareOperator = pipelineNode.child_value("DepthCompareOperator");
        if (!depthCompareOperator.empty()) {
            spec.depthCompareOperator = CustomPipelinesLoaderUtils::depthCompareOperatorFromString(depthCompareOperator);
        }

        std::string_view depthTest = pipelineNode.child_value("DepthTest");
        if (!depthTest.empty()) {
            spec.depthTest = StringUtils::toBool(depthTest);
        }

        std::string_view depthWrite = pipelineNode.child_value("DepthWrite");
        if (!depthTest.empty()) {
            spec.depthWrite = StringUtils::toBool(depthWrite);
        }

        std::string_view wireframe = pipelineNode.child_value("Wireframe");
        if (!depthTest.empty()) {
            spec.wireframe = StringUtils::toBool(wireframe);
        }

        std::string_view backfaceCulling = pipelineNode.child_value("BackfaceCulling");
        if (!depthTest.empty()) {
            spec.backfaceCulling = StringUtils::toBool(backfaceCulling);
        }

        std::string_view frontfaceCulling = pipelineNode.child_value("FrontfaceCulling");
        if (!depthTest.empty()) {
            spec.frontfaceCulling = StringUtils::toBool(frontfaceCulling);
        }

        std::string_view useBlending = pipelineNode.child_value("UseBlending");
        if (!depthTest.empty()) {
            spec.useBlending = StringUtils::toBool(useBlending);
        }

        std::string_view blendingEquation = pipelineNode.child_value("BlendingEquation");
        if (!depthTest.empty()) {
            spec.blendingEquation = CustomPipelinesLoaderUtils::blendingEquationFromString(blendingEquation);
        }

        std::string_view srcBlendingFunction = pipelineNode.child_value("SrcBlendingFunction");
        if (!depthTest.empty()) {
            spec.srcBlendingFunction = CustomPipelinesLoaderUtils::blendingFunctionFromString(srcBlendingFunction);
        }

        std::string_view destBlendingFunction = pipelineNode.child_value("DestBlendingFunction");
        if (!depthTest.empty()) {
            spec.destBlendingFunction = CustomPipelinesLoaderUtils::blendingFunctionFromString(destBlendingFunction);
        }

        std::string_view drawMode = pipelineNode.child_value("DrawMode");
        if (!depthTest.empty()) {
            spec.drawMode = CustomPipelinesLoaderUtils::drawModeFromString(drawMode);
        }

        std::string_view tesselationPatchSize = pipelineNode.child_value("TesselationPatchSize");
        if (!depthTest.empty()) {
            spec.tesselationPatchSize = StringUtils::toInt(tesselationPatchSize).value_or(~0);
        }

        const auto& vertexBufferLayoutsNode = pipelineNode.child("VertexBufferLayouts");
        for (const auto &vbLayoutNode: vertexBufferLayoutsNode.children()) {
            std::vector<VertexBufferElement> vbElements;

            for (const auto& vbElementNode: vbLayoutNode.children()) {
                VertexBufferElement element(CustomPipelinesLoaderUtils::shaderDataTypeFromString(vbElementNode.attribute("data-type").as_string()), vbElementNode.attribute("normalized").as_bool(true));
                vbElements.push_back(element);
            }

            spec.vertexBufferLayouts.emplace_back(vbElements);
        }

        const auto& descriptorSetLayoutNode = pipelineNode.child("DescriptorSetLayout");
        spec.descriptorSetLayoutSpecification.uboBindingPoints = CustomPipelinesLoaderUtils::descriptorSetLayoutBindingsFromNode(descriptorSetLayoutNode.child("UboBindingPoints"));
        spec.descriptorSetLayoutSpecification.ssboBindingPoints = CustomPipelinesLoaderUtils::descriptorSetLayoutBindingsFromNode(descriptorSetLayoutNode.child("SsboBindingPoints"));
        spec.descriptorSetLayoutSpecification.texture2DAndAttachmentBindingPoints = CustomPipelinesLoaderUtils::descriptorSetLayoutBindingsFromNode(descriptorSetLayoutNode.child("Texture2DAndAttachmentBindingPoints"));
        spec.descriptorSetLayoutSpecification.imageBindingPoints = CustomPipelinesLoaderUtils::descriptorSetLayoutBindingsFromNode(descriptorSetLayoutNode.child("ImageBindingPoints"));

        const auto& shaderNode = pipelineNode.child("Shader");
        std::string vertexPath = shaderNode.child("Vertex").child_value();
        std::string fragmentPath = shaderNode.child("Fragment").child_value();
        std::string geometryPath = shaderNode.child("Geometry").child_value();
        std::string tcsPath = shaderNode.child("Tcs").child_value();
        std::string tesPath = shaderNode.child("Tes").child_value();

        spec.shaders.vertex = vertexPath;
        spec.shaders.fragment = fragmentPath;
        spec.shaders.geometry = geometryPath;
        spec.shaders.tcs = tcsPath;
        spec.shaders.tes = tesPath;

        return new CustomGraphicsPipeline(spec);
    }

    CustomGraphicsPipeline::CustomGraphicsPipeline(const CustomGraphicsPipelineSpecification& spec) {
        const auto& sceneRenderer = Application::get().getSceneRenderer();

        descriptorSetLayout = GraphicsObjectsFactory::createDescriptorSetLayout(spec.descriptorSetLayoutSpecification);
        std::vector<const DescriptorSetLayout*> descriptorSetLayouts(2);
        descriptorSetLayouts[0] = sceneRenderer.getCustomPipelineDescriptorSetLayout();
        descriptorSetLayouts[1] = descriptorSetLayout;

        GraphicsPipelineSpecification pipelineSpec{};
        pipelineSpec.depthCompareOperator = spec.depthCompareOperator;
        pipelineSpec.depthTest = spec.depthTest;
        pipelineSpec.depthWrite = spec.depthWrite;
        pipelineSpec.wireframe = spec.wireframe;
        pipelineSpec.backfaceCulling = spec.backfaceCulling;
        pipelineSpec.frontfaceCulling = spec.frontfaceCulling;
        pipelineSpec.useBlending = spec.useBlending;
        pipelineSpec.blendingEquation = spec.blendingEquation;
        pipelineSpec.srcBlendingFunction = spec.srcBlendingFunction;
        pipelineSpec.destBlendingFunction = spec.destBlendingFunction;
        pipelineSpec.tesselationPatchSize = spec.tesselationPatchSize;
        pipelineSpec.drawMode = spec.drawMode;
        pipelineSpec.customShaders = spec.shaders;
        pipelineSpec.engineShaderName = "";
        pipelineSpec.vertexInputLayout = spec.vertexBufferLayouts;
        pipelineSpec.descriptorSetLayouts = descriptorSetLayouts;

        PipelineAttachmentInfo pipelineAttachmentInfo = sceneRenderer.getGBufferAttachmentInfo();

        pipelineSpec.colorAttachments = pipelineAttachmentInfo.colorAttachments;
        pipelineSpec.hasDepthStencilAttachment = pipelineAttachmentInfo.hasDepthStencilAttachment;
        pipelineSpec.depthAttachmentFormat = pipelineAttachmentInfo.depthAttachmentFormat;

        pipelineSpec.usesPushConstants = true;
        pipelineSpec.pushConstantsSize = sizeof(SceneRenderer::CustomPipelineData);

        graphicsPipeline = GraphicsObjectsFactory::createGraphicsPipeline(pipelineSpec);
    }

    CustomGraphicsPipeline::~CustomGraphicsPipeline() {
        delete graphicsPipeline;
        delete descriptorSetLayout;
    }

    CustomGraphicsPipeline::CustomGraphicsPipeline(CustomGraphicsPipeline&& other) noexcept {
        graphicsPipeline = other.graphicsPipeline;
        other.graphicsPipeline = nullptr;
        descriptorSetLayout = other.descriptorSetLayout;
        other.descriptorSetLayout = nullptr;
    }

    CustomGraphicsPipeline& CustomGraphicsPipeline::operator=(CgEngine::CustomGraphicsPipeline&& other) noexcept {
        if (this != &other) {
            graphicsPipeline = other.graphicsPipeline;
            other.graphicsPipeline = nullptr;
            descriptorSetLayout = other.descriptorSetLayout;
            other.descriptorSetLayout = nullptr;
        }
        return *this;
    }

    const GraphicsPipeline* CustomGraphicsPipeline::getGraphicsPipeline() const {
        return graphicsPipeline;
    }

    const DescriptorSetLayout* CustomGraphicsPipeline::getDescriptorSetLayout() const {
        return descriptorSetLayout;
    }

    CustomComputePipeline* CustomComputePipeline::createResource(const std::string& name) {
        const pugi::xml_document& xml = CustomPipelinesData::getPipelinesXMLFile();
        const auto& pipelineNode = xml.child("Pipelines").find_child_by_attribute("ComputePipeline", "name", name.c_str());

        CustomComputePipelineSpecification spec{};
        spec.shader = pipelineNode.child_value("Shader");

        const auto& descriptorSetLayoutNode = pipelineNode.child("DescriptorSetLayout");
        spec.descriptorSetLayoutSpecification.uboBindingPoints = CustomPipelinesLoaderUtils::descriptorSetLayoutBindingsFromNode(descriptorSetLayoutNode.child("UboBindingPoints"));
        spec.descriptorSetLayoutSpecification.ssboBindingPoints = CustomPipelinesLoaderUtils::descriptorSetLayoutBindingsFromNode(descriptorSetLayoutNode.child("SsboBindingPoints"));
        spec.descriptorSetLayoutSpecification.texture2DAndAttachmentBindingPoints = CustomPipelinesLoaderUtils::descriptorSetLayoutBindingsFromNode(descriptorSetLayoutNode.child("Texture2DAndAttachmentBindingPoints"));
        spec.descriptorSetLayoutSpecification.imageBindingPoints = CustomPipelinesLoaderUtils::descriptorSetLayoutBindingsFromNode(descriptorSetLayoutNode.child("ImageBindingPoints"));

        return new CustomComputePipeline(spec);
    }

    CustomComputePipeline::CustomComputePipeline(const CustomComputePipelineSpecification& spec) {
        descriptorSetLayout = GraphicsObjectsFactory::createDescriptorSetLayout(spec.descriptorSetLayoutSpecification);

        ComputePipelineSpecification pipelineSpec{};
        pipelineSpec.descriptorSetLayouts = {descriptorSetLayout};
        pipelineSpec.customShader = spec.shader;

        computePipeline = GraphicsObjectsFactory::createComputePipeline(pipelineSpec);
    }

    CustomComputePipeline::~CustomComputePipeline() {
        delete computePipeline;
        delete descriptorSetLayout;
    }

    CustomComputePipeline::CustomComputePipeline(CustomComputePipeline&& other) noexcept {
        computePipeline = other.computePipeline;
        other.computePipeline = nullptr;
        descriptorSetLayout = other.descriptorSetLayout;
        other.descriptorSetLayout = nullptr;
    }

    CustomComputePipeline& CustomComputePipeline::operator=(CustomComputePipeline&& other) noexcept {
        if (this != &other) {
            computePipeline = other.computePipeline;
            other.computePipeline = nullptr;
            descriptorSetLayout = other.descriptorSetLayout;
            other.descriptorSetLayout = nullptr;
        }
        return *this;
    }

    const ComputePipeline* CustomComputePipeline::getComputePipeline() const {
        return computePipeline;
    }

    const DescriptorSetLayout* CustomComputePipeline::getDescriptorSetLayout() const {
        return descriptorSetLayout;
    }
}
