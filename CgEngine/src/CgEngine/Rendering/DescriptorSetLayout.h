#pragma once

namespace CgEngine {

    enum class DescriptorSetLayoutUsage {
        Graphics, Compute, GraphicsAndCompute
    };

    struct DescriptorSetLayoutBinding {
        uint32_t bindingPoint = 0;;
        uint32_t descriptorCount = 1;

        DescriptorSetLayoutBinding() = default;
        DescriptorSetLayoutBinding(const uint32_t bindingPoint) : bindingPoint(bindingPoint) {};
    };

    struct DescriptorSetLayoutSpecification {
        DescriptorSetLayoutUsage usage;
        std::vector<DescriptorSetLayoutBinding> uboBindingPoints;
        std::vector<DescriptorSetLayoutBinding> ssboBindingPoints;
        std::vector<DescriptorSetLayoutBinding> texture2DAndAttachmentBindingPoints;
        std::vector<DescriptorSetLayoutBinding> imageBindingPoints;

        // These functions set only bindingPoint, descriptorCount is always assumed to be 1
        void setUboBindingPoints(const std::vector<uint32_t>& bindingPoints);
        void setSsboBindingPoints(const std::vector<uint32_t>& bindingPoints);
        void setTexture2DAndAttachmentBindingPoints(const std::vector<uint32_t>& bindingPoints);
        void setImageBindingPoints(const std::vector<uint32_t>& bindingPoints);
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
        static bool validateBindingPoints(std::vector<std::pair<std::vector<DescriptorSetLayoutBinding>, const char*>> bindingPoints);
    };

}
