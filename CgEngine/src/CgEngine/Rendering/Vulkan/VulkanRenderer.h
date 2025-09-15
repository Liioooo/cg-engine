#pragma once

#include "Rendering/RendererBackendBase.h"

namespace CgEngine {

    class VulkanRenderer : public RendererBackendBase {
        void init(Window& window) override;
        void shutdown() override;
    };

}
