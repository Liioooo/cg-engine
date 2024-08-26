#pragma once

#include "Camera.h"
#include "AABoundingBox.h"

namespace CgEngine {

    struct FrustumPlane {
        glm::vec3 normal;
        float distance;

        FrustumPlane() = default;
        FrustumPlane(const glm::vec3& point, const glm::vec3& norm) : normal(glm::normalize(norm)), distance(glm::dot(normal, point)) {};

        float getSignedDistanceToPlane(const glm::vec3& point) const;
    };

    class CameraFrustum {
    public:
        CameraFrustum() = default;

        void updateCameraFrustum(const Camera& camera, const glm::vec3 cameraPos, const glm::vec3& cameraFront);
        bool testAABoundingBoxInFrustum(AABoundingBox& boundingBox, const glm::mat4& boxTransform) const;

    private:
        std::array<FrustumPlane, 6> frustumPlanes{};
    };

}
