#pragma once
#include "Rendering/PushConstants.h"

namespace CgEngine {

    class OpenGLPushConstants : public PushConstants {
    public:
        OpenGLPushConstants(std::string prefix) : PushConstants(std::move(prefix)) {};
        ~OpenGLPushConstants() override = default;

        void upload(uint32_t programHandle) const;

    protected:
        void mapUniformInternal(size_t structSize, std::unique_ptr<IMemberAccessor> memberAccessor, const std::string& name) override;

    private:
        struct Setter {
            std::string name;
            std::unique_ptr<IMemberAccessor> accessor;
        };
        std::vector<Setter> setters;
    };

}
