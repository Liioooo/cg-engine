#include "DescriptorSetLayout.h"

#include "Logging.h"

namespace CgEngine {
    bool DescriptorSetLayout::validateBindingPoints(std::vector<std::pair<std::vector<uint32_t>, const char *>> bindingPoints) {
        std::unordered_map<uint32_t, const char*> usedBindings;

        for (const auto& [bindings, typeName] : bindingPoints) {
            for (uint32_t binding : bindings) {
                auto it = usedBindings.find(binding);
                if (it != usedBindings.end()) {
                    CG_LOGGING_ERROR("Binding point {} used by both {} and {}", binding, it->second, typeName)
                    return false;
                }
                usedBindings[binding] = typeName;
            }
        }
        return true;
    }
}
