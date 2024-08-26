#include "CameraFrustum.h"
#include "Logging.h"

namespace CgEngine {
    float FrustumPlane::getSignedDistanceToPlane(const glm::vec3& point) const {
        return glm::dot(normal, point) - distance;
    }

    void CameraFrustum::updateCameraFrustum(const Camera& camera, const glm::vec3 cameraPos, const glm::vec3& cameraFront) {
        if (camera.getProjectionType() == CameraProjectionType::Perspective) {
            float farH = camera.getPerspectiveFar() * glm::tan(glm::radians(camera.getPerspectiveFov()) * 0.5f);
            float farW = farH * camera.getAspectRatio();

            glm::vec3 Z = cameraFront;
            glm::vec3 X = glm::normalize(glm::cross(glm::vec3({0.0f, 1.0f, 0.0f}), Z));
            glm::vec3 Y = glm::cross(Z, X);

            glm::vec3 ZNear = Z * camera.getPerspectiveNear();
            glm::vec3 ZFar = Z * camera.getPerspectiveFar();

            // NEAR
            frustumPlanes[0] = {cameraPos + ZNear, Z};

            // FAR
            frustumPlanes[1] = {cameraPos + ZFar, -Z};

            // TOP
            frustumPlanes[2] = {cameraPos, glm::cross(X, ZFar + Y * farH)};

            // BOTTOM
            frustumPlanes[3] = {cameraPos, glm::cross(ZFar - Y * farH, X)};

            // LEFT
            frustumPlanes[4] = {cameraPos, glm::cross(Y, ZFar - X * farW)};

            // RIGHT
            frustumPlanes[5] = {cameraPos, glm::cross(ZFar + X * farW, Y)};
        } else {

        }
    }

    bool CameraFrustum::testAABoundingBoxInFrustum(AABoundingBox& boundingBox, const glm::mat4& boxTransform) const {
        uint32_t in, out;

        for (uint32_t i = 0; i < 6; i++) {
            in = 0;
            out = 0;

            for (uint32_t j = 0; j < 8 && (in == 0 || out == 0); j++) {
                if (frustumPlanes[i].getSignedDistanceToPlane(glm::vec3(boxTransform * glm::vec4(boundingBox.getVertex(j), 1.0f))) < 0) {
                    out++;
                } else {
                    in++;
                }
            }

            if (in == 0) {
                return false;
            } else if (out != 0) {
                return true;
            }
        }

        return true;
    }
}