#pragma once

namespace CgEngine {

    struct AudioTransform {
        glm::vec3 position{ 0.0f, 0.0f, 0.0f };
        glm::vec3 orientation{ 0.0f, 0.0f, -1.0f };
        glm::vec3 up{ 0.0f, 1.0f, 0.0f };

        AudioTransform() = default;
        AudioTransform(glm::quat rotation, glm::vec3 translation) : position(translation), orientation(rotation * glm::vec3(0.0f, 0.0f, -1.0f)), up(rotation * glm::vec3(0.0f, 1.0f, 0.0f)) {}

        bool operator==(const AudioTransform& other) const {
            return position == other.position && orientation == other.orientation && up == other.up;
        }

        bool operator!=(const AudioTransform& other) const {
            return !(*this == other);
        }
    };

}
