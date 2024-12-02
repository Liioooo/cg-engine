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
    }

    void AABoundingBox::setCenterAndExtents(glm::vec3 center, glm::vec3 extents) {
        addedCoords = true;

        min = center - extents / 2.0f;
        max = center + extents / 2.0f;
    }

    bool AABoundingBox::hasCoords() const {
        return addedCoords;
    }

    glm::vec3 AABoundingBox::getCenterPoint() const {
        return (min + max) / 2.0f;
    }

    glm::vec3 AABoundingBox::getExtents() const {
        glm::vec3 center = getCenterPoint();
        return {max.x - center.x, max.y - center.y, max.z - center.z};
    }

    std::pair<glm::vec3, glm::vec3> AABoundingBox::getTransformedAdjustedCenterAndExtents(const glm::mat4& transform) const {
        glm::vec3 boxCenter = getCenterPoint();
        glm::vec3 adjustedCenter(transform * glm::vec4(boxCenter, 1.f));
        glm::vec3 extents = getExtents();

        const glm::vec3 right = transform[0] * extents.x;
        const glm::vec3 up = transform[1] * extents.y;
        const glm::vec3 forward = -transform[2] * extents.z;

        const float ix = glm::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, right)) +
                glm::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, up)) +
                glm::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, forward));

        const float iy = glm::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, right)) +
                glm::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, up)) +
                glm::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, forward));

        const float iz = glm::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, right)) +
                glm::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, up)) +
                glm::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, forward));

        return {adjustedCenter, {ix, iy, iz}};
    }
}
