#pragma once

#include "Rendering/DescriptorSet.h"

namespace CgEngine {

    class OpenGLDescriptorSet : public DescriptorSet {
    public:
        OpenGLDescriptorSet() = default;
        explicit OpenGLDescriptorSet(const DescriptorSetSpecification& spec);

        ~OpenGLDescriptorSet() override = default;

        OpenGLDescriptorSet(OpenGLDescriptorSet&& other) noexcept;
        OpenGLDescriptorSet& operator=(OpenGLDescriptorSet&& other) noexcept;

        OpenGLDescriptorSet(OpenGLDescriptorSet& other) = delete;
        OpenGLDescriptorSet& operator=(OpenGLDescriptorSet& other) = delete;

        bool isReady() const override;

        const DescriptorSetSpecification& getSpecification() const;
        void bind() const;


    private:
        DescriptorSetSpecification specification;
        bool ready = false;
    };

}
