#include "AABoundingBox.h"

namespace CgEngine {
    void AABoundingBox::addBoxCoordinates(glm::vec3 max, glm::vec3 min) {
        glm::vec3 corrMin = {
                glm::min(max.x, min.x),
                glm::min(max.y, min.y),
                glm::min(max.z, min.z)
        };
        glm::vec3 corrMax = {
                glm::max(max.x, min.x),
                glm::max(max.y, min.y),
                glm::max(max.z, min.z)
        };

        if (addedCoords) {
            this->min = {glm::min(this->min.x, corrMin.x), glm::min(this->min.y, corrMin.y), glm::min(this->min.z, corrMin.z)};
            this->max = {glm::max(this->max.x, corrMax.x), glm::max(this->max.y, corrMax.y), glm::max(this->max.z, corrMax.z)};
        } else {
            this->min = corrMin;
            this->max = corrMax;
            addedCoords = true;
        }
        verticesDirty = true;
    }

    void AABoundingBox::applyTransform(const glm::mat4& transform) {
        min = glm::vec3(transform * glm::vec4(min, 1.0f));
        max = glm::vec3(transform * glm::vec4(max, 1.0f));
        verticesDirty = true;
    }

    glm::mat4 AABoundingBox::getTransformForCubeMesh() const {
        glm::vec3 size = {
            glm::abs(max.x - min.x),
            glm::abs(max.y - min.y),
            glm::abs(max.z - min.z)
        };

        glm::vec3 midPoint = (min + max) / 2.0f;

        return glm::translate(glm::mat4(1.0f), midPoint) * glm::scale(glm::mat4(1.0f), size);
    }

    const glm::vec3& AABoundingBox::getVertex(uint32_t index) {
        if (verticesDirty) {
            vertices[0] = {min.x, min.y, min.z};
            vertices[1] = {max.x, min.y, min.z};
            vertices[2] = {min.x, max.y, min.z};
            vertices[3] = {min.x, min.y, max.z};

            vertices[4] = {max.x, max.y, min.z};
            vertices[5] = {max.x, min.y, max.z};
            vertices[6] = {min.x, max.y, max.z};
            vertices[7] = {max.x, max.y, max.z};

            verticesDirty = false;
        }
        return vertices[index];
    }
}