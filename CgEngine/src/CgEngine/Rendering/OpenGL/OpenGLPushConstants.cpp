#pragma once
#include "OpenGLPushConstants.h"
#include "OpenGLHelpers.h"

namespace CgEngine {

    void OpenGLPushConstants::upload(uint32_t programHandle) const {
        for (const auto& setter : setters) {
            setter.accessor->setUniform(data, OpenGLHelpers::getShaderUniformLocation(programHandle, setter.name));
        }
    }

    void OpenGLPushConstants::mapUniformInternal(size_t structSize, std::unique_ptr<IMemberAccessor> memberAccessor, const std::string& name) {
        setters.push_back({name, std::move(memberAccessor)});
    }

}
