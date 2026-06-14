#pragma once

#include "OpenGLDescriptorSetLayout.h"

#include "Asserts.h"

namespace CgEngine {
    OpenGLDescriptorSetLayout::OpenGLDescriptorSetLayout(const DescriptorSetLayoutSpecification &spec) {
        CG_ASSERT(validateBindingPoints({{spec.uboBindingPoints, "UBO"}, {spec.ssboBindingPoints, "SSBO"}, {spec.texture2DAndAttachmentBindingPoints, "Texture2D/Attachment"}, {spec.imageBindingPoints, "Image"}}), "DescriptorSetLayout: Binding points are overlapping!")
    }

    bool OpenGLDescriptorSetLayout::isReady() const {
        return true;
    }

}
