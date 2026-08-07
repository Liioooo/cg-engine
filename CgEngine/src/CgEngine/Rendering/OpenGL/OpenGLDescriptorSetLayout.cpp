#pragma once

#include "OpenGLDescriptorSetLayout.h"

#include "Asserts.h"

namespace CgEngine {
    OpenGLDescriptorSetLayout::OpenGLDescriptorSetLayout(const DescriptorSetLayoutSpecification &spec) : specification(spec), ready(true) {
        CG_ASSERT(validateBindingPoints({{spec.uboBindingPoints, "UBO"}, {spec.immutableSsboBindingPoints, "Immutable SSBO"}, {spec.ssboBindingPoints, "SSBO"}, {spec.vertexBufferSsboBindingPoints, "VertexBuffer SSBO"}, {spec.texture2DAndAttachmentBindingPoints, "Texture2D/Attachment"}, {spec.imageBindingPoints, "Image"}}), "DescriptorSetLayout: Binding points are overlapping!")

        bindingPointUsageMap = std::move(createBindingPointUsageMap(specification));
    }

    OpenGLDescriptorSetLayout::OpenGLDescriptorSetLayout(OpenGLDescriptorSetLayout &&other) noexcept : DescriptorSetLayout(std::move(other)), ready(other.ready) {
        specification = std::move(other.specification);
        other.ready = false;
    }

    OpenGLDescriptorSetLayout & OpenGLDescriptorSetLayout::operator=(OpenGLDescriptorSetLayout &&other) noexcept {
        if (this != &other) {
            DescriptorSetLayout::operator=(std::move(other));
            specification = std::move(other.specification);
            other.specification = DescriptorSetLayoutSpecification();
            ready = other.ready;
            other.ready = false;
        }
        return *this;
    }

    DescriptorSetLayoutBindingUsage OpenGLDescriptorSetLayout::getDescriptorSetLayoutBindingUsageForBindingPoint(uint32_t bindingPoint) const {
        return bindingPointUsageMap.at(bindingPoint);
    }

    bool OpenGLDescriptorSetLayout::isReady() const {
        return ready;
    }

    const DescriptorSetLayoutSpecification& OpenGLDescriptorSetLayout::getSpecification() const {
        CG_ASSERT(ready, "DescriptorSetLayout is not ready!")
        return specification;
    }
}
