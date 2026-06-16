#pragma once
#include "CgEngineSharedUtils/Enums.h"

namespace CgEngine {

    struct PipelineAttachmentInfo {
        std::vector<AttachmentType> colorAttachments;
        bool hasDepthStencilAttachment = false;
        DepthStencilAttachmentFormat depthAttachmentFormat;
    };

}
