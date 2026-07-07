#include "DescriptorSetLayout.h"

#include "Logging.h"

namespace CgEngine {
    void DescriptorSetLayoutSpecification::setUboBindingPoints(const std::vector<uint32_t>& bindingPoints) {
        uboBindingPoints.resize(bindingPoints.size());

        for (size_t i = 0; i < bindingPoints.size(); i++) {
            uboBindingPoints[i].bindingPoint = bindingPoints[i];
            uboBindingPoints[i].descriptorCount = 1;
        }
    }

    void DescriptorSetLayoutSpecification::setSsboBindingPoints(const std::vector<uint32_t>& bindingPoints) {
        ssboBindingPoints.resize(bindingPoints.size());

        for (size_t i = 0; i < bindingPoints.size(); i++) {
            ssboBindingPoints[i].bindingPoint = bindingPoints[i];
            ssboBindingPoints[i].descriptorCount = 1;
        }
    }

    void DescriptorSetLayoutSpecification::setTexture2DAndAttachmentBindingPoints(const std::vector<uint32_t>& bindingPoints) {
        texture2DAndAttachmentBindingPoints.resize(bindingPoints.size());

        for (size_t i = 0; i < bindingPoints.size(); i++) {
            texture2DAndAttachmentBindingPoints[i].bindingPoint = bindingPoints[i];
            texture2DAndAttachmentBindingPoints[i].descriptorCount = 1;
        }
    }

    void DescriptorSetLayoutSpecification::setImageBindingPoints(const std::vector<uint32_t>& bindingPoints) {
        imageBindingPoints.resize(bindingPoints.size());

        for (size_t i = 0; i < bindingPoints.size(); i++) {
            imageBindingPoints[i].bindingPoint = bindingPoints[i];
            imageBindingPoints[i].descriptorCount = 1;
        }
    }

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
}
