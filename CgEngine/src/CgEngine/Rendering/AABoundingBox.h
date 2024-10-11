#pragma once

namespace CgEngine {

    class AABoundingBox {
    public:
        AABoundingBox() = default;

        void addBoxCoordinates(glm::vec3 max, glm::vec3 min);

        glm::vec3 getCenterPoint() const;
        glm::vec3 getExtents() const;
        std::pair<glm::vec3, glm::vec3> getTransformedAdjustedCenterAndExtents(const glm::mat4& transform) const;

    private:
        glm::vec3 min = {0.0f, 0.0f, 0.0f};
        glm::vec3 max = {0.0f, 0.0f, 0.0f};

        bool addedCoords = false;
    };

}
