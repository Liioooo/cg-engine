#pragma once

#include "EnumFlags.h"

namespace CgEngine {

    enum class DescriptorSetLayoutBindingUsage : uint32_t {
        None = 0,
        Compute = 1 << 0,
        TCS = 1 << 1,
        TES = 1 << 2,
        Geometry = 1 << 3,
        Fragment = 1 << 4,
        Vertex = 1 << 5,
        AllGraphics = TCS | TES | Geometry | Fragment | Vertex,
        All = Compute | TCS | TES | Geometry | Fragment | Vertex
    };

    CG_ENUM_FLAGS(DescriptorSetLayoutBindingUsage)

    struct DescriptorSetLayoutBinding {
        uint32_t bindingPoint = 0;
        DescriptorSetLayoutBindingUsage usage = DescriptorSetLayoutBindingUsage::None;
        uint32_t descriptorCount = 1;

        DescriptorSetLayoutBinding() = default;
        DescriptorSetLayoutBinding(const uint32_t bindingPoint, DescriptorSetLayoutBindingUsage usage) : bindingPoint(bindingPoint), usage(usage) {};
    };

    struct DescriptorSetLayoutSpecification {
        std::vector<DescriptorSetLayoutBinding> uboBindingPoints;
        std::vector<DescriptorSetLayoutBinding> ssboBindingPoints;
        std::vector<DescriptorSetLayoutBinding> texture2DAndAttachmentBindingPoints;
        std::vector<DescriptorSetLayoutBinding> imageBindingPoints;
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
