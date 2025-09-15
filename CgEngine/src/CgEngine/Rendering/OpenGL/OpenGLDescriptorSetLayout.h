#pragma once

#include "Rendering/DescriptorSetLayout.h"

namespace CgEngine {

    class OpenGLDescriptorSetLayout : public DescriptorSetLayout {
    public:
        OpenGLDescriptorSetLayout() = default;
        explicit OpenGLDescriptorSetLayout(const DescriptorSetLayoutSpecification& spec) {}

        ~OpenGLDescriptorSetLayout() override = default;

        OpenGLDescriptorSetLayout(OpenGLDescriptorSetLayout&& other) noexcept = default;
        OpenGLDescriptorSetLayout& operator=(OpenGLDescriptorSetLayout&& other) noexcept = default;

        OpenGLDescriptorSetLayout(OpenGLDescriptorSetLayout& other) = delete;
        OpenGLDescriptorSetLayout& operator=(OpenGLDescriptorSetLayout& other) = delete;

        bool isReady() const override;
    };

}
