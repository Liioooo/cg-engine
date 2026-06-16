#pragma once

#include "Rendering/DescriptorSet.h"

namespace CgEngine {

    class OpenGLDescriptorSet : public DescriptorSet {
    public:
        OpenGLDescriptorSet() = default;
        explicit OpenGLDescriptorSet(const DescriptorSetSpecification& spec);
        explicit OpenGLDescriptorSet(const DescriptorSetLayout* layout);

        ~OpenGLDescriptorSet() override = default;

        OpenGLDescriptorSet(OpenGLDescriptorSet&& other) noexcept;
        OpenGLDescriptorSet& operator=(OpenGLDescriptorSet&& other) noexcept;

        OpenGLDescriptorSet(OpenGLDescriptorSet& other) = delete;
        OpenGLDescriptorSet& operator=(OpenGLDescriptorSet& other) = delete;

        bool isReady() const override;
        void recreate() override;
        void reconfigure(const DescriptorSetSpecification& spec) override;

        const DescriptorSetSpecification& getSpecification() const;
        void bind() const;


    private:
        DescriptorSetSpecification specification{};
        bool ready = false;

        bool validateSpecification(const DescriptorSetSpecification& spec) const;
    };

}
