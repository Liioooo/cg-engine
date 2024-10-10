#pragma once

namespace CgEngine {

    class AABoundingBox {
    public:
        AABoundingBox() = default;

        void addBoxCoordinates(glm::vec3 max, glm::vec3 min);
        void applyTransform(const glm::mat4& transform);

        glm::mat4 getTransformForCubeMesh() const;
        const glm::vec3& getVertex(uint32_t index);

    private:
        glm::vec3 min = {0.0f, 0.0f, 0.0f};
        glm::vec3 max = {0.0f, 0.0f, 0.0f};

        bool addedCoords = false;

        std::array<glm::vec3, 8> vertices{};
        bool verticesDirty = true;
    };

}
