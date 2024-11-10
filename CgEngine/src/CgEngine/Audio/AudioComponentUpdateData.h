#pragma once

#include "AudioTransform.h"
#include "Uuid.h"

namespace CgEngine {

    struct AudioComponentUpdateData {
        Uuid uuid;

        float volume;
        float pitch;
        bool looping;
        AudioTransform transform;
        glm::vec3 velocity;
    };
}
