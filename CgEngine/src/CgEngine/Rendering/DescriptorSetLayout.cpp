#include "DescriptorSetLayout.h"

#include "Logging.h"

namespace CgEngine {
    bool DescriptorSetLayout::validateBindingPoints(std::vector<std::pair<std::vector<DescriptorSetLayoutBinding>, const char *>> bindingPoints) {
        std::unordered_map<uint32_t, const char*> usedBindings;

        for (const auto& [bindings, typeName] : bindingPoints) {
            for (auto binding : bindings) {
                auto it = usedBindings.find(binding.bindingPoint);
                if (it != usedBindings.end()) {
                    CG_LOGGING_ERROR("Binding point {} used by both {} and {}", binding.bindingPoint, it->second, typeName)
                    return false;
                }
                usedBindings[binding.bindingPoint] = typeName;
            }
        }
        return true;
    }

    std::unordered_map<uint32_t, DescriptorSetLayoutBindingUsage> DescriptorSetLayout::createBindingPointUsageMap(const DescriptorSetLayoutSpecification& spec) {
        std::unordered_map<uint32_t, DescriptorSetLayoutBindingUsage> out{};

        for (const auto& binding : spec.uboBindingPoints) {
           out[binding.bindingPoint] = binding.usage;
        }
        for (const auto& binding : spec.ssboBindingPoints) {
            out[binding.bindingPoint] = binding.usage;
        }
        for (const auto& binding : spec.texture2DAndAttachmentBindingPoints) {
            out[binding.bindingPoint] = binding.usage;
        }
        for (const auto& binding : spec.imageBindingPoints) {
            out[binding.bindingPoint] = binding.usage;
        }

        return out;
    }
}
