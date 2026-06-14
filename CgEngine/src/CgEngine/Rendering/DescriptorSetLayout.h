#pragma once

namespace CgEngine {

    enum class DescriptorSetLayoutUsage {
        Graphics, Compute, GraphicsAndCompute
    };

    struct DescriptorSetLayoutSpecification {
        DescriptorSetLayoutUsage usage;
        std::vector<uint32_t> uboBindingPoints;
        std::vector<uint32_t> ssboBindingPoints;
        std::vector<uint32_t> texture2DAndAttachmentBindingPoints;
        std::vector<uint32_t> imageBindingPoints;
    };

    class DescriptorSetLayout {
    public:
        DescriptorSetLayout() = default;

        virtual ~DescriptorSetLayout() = default;

        DescriptorSetLayout(DescriptorSetLayout&& other) noexcept = default;
        DescriptorSetLayout& operator=(DescriptorSetLayout&& other) noexcept = default;

        DescriptorSetLayout(DescriptorSetLayout& other) = delete;
        DescriptorSetLayout& operator=(DescriptorSetLayout& other) = delete;

        virtual bool isReady() const = 0;

    protected:
        static bool validateBindingPoints(std::vector<std::pair<std::vector<uint32_t>, const char*>> bindingPoints);
    };

}
