#pragma once

#include "Rendering/DescriptorSetLayout.h"

namespace CgEngine {

    class OpenGLDescriptorSetLayout : public DescriptorSetLayout {
    public:
        OpenGLDescriptorSetLayout() = default;
        explicit OpenGLDescriptorSetLayout(const DescriptorSetLayoutSpecification& spec);

        ~OpenGLDescriptorSetLayout() override = default;

        OpenGLDescriptorSetLayout(OpenGLDescriptorSetLayout&& other) noexcept;
        OpenGLDescriptorSetLayout& operator=(OpenGLDescriptorSetLayout&& other) noexcept;

        OpenGLDescriptorSetLayout(OpenGLDescriptorSetLayout& other) = delete;
        OpenGLDescriptorSetLayout& operator=(OpenGLDescriptorSetLayout& other) = delete;

        DescriptorSetLayoutBindingUsage getDescriptorSetLayoutBindingUsageForBindingPoint(uint32_t bindingPoint) const override;

        bool isReady() const override;

        const DescriptorSetLayoutSpecification& getSpecification() const;

    private:
        bool ready = false;
        DescriptorSetLayoutSpecification specification{};
        std::unordered_map<uint32_t, DescriptorSetLayoutBindingUsage> bindingPointUsageMap{};
    };

}
