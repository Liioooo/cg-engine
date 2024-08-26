#include "TransformComponent.h"
#include "Scene/Scene.h"

namespace CgEngine {
    void TransformComponent::onAttach(Scene& scene, TransformComponentParams &params) {
        localPosition = params.position;
        localRotationVec = params.rotation;
        localRotationQuat = glm::quat(params.rotation);
        localScale = params.scale;

        if (scene.hasParent(entity)) {
            _calculateChildTransformsWithParent(scene.getComponent<TransformComponent>(scene.getParent(entity)).modelMatrix, true);
        } else {
            _calculateTopLevelTransforms();
        }
    }

    const glm::vec3& TransformComponent::getLocalPosition() const {
        return localPosition;
    }

    const glm::vec3& TransformComponent::getLocalRotationVec() const {
        return localRotationVec;
    }

    const glm::quat & TransformComponent::getLocalRotationQuat() const {
        return localRotationQuat;
    }

    const glm::vec3& TransformComponent::getLocalScale() const {
        return localScale;
    }

    const glm::vec3& TransformComponent::getGlobalPosition() const {
        return globalPosition;
    }

    const glm::vec3& TransformComponent::getGlobalRotationVec() const {
        return globalRotationVec;
    }

    const glm::quat& TransformComponent::getGlobalRotationQuat() const {
        return globalRotationQuat;
    }

    const glm::vec3& TransformComponent::getGlobalScale() const {
        return globalScale;
    }

    const glm::mat4& TransformComponent::getModelMatrix() const {
        return modelMatrix;
    }

    void TransformComponent::setLocalPosition(glm::vec3 position) {
        isDirty = true;
        localPosition = position;
    }

    void TransformComponent::setLocalRotationVec(glm::vec3 rotation) {
        isDirty = true;
        localRotationQuat = glm::quat(rotation);
        localRotationVec = rotation;
    }

    void TransformComponent::setLocalRotationQuat(glm::quat rotation) {
        isDirty = true;
        localRotationQuat = rotation;
        localRotationVec = glm::eulerAngles(rotation);
    }

    void TransformComponent::setLocalScale(glm::vec3 scale) {
        isDirty = true;
        localScale = scale;
    }

    void TransformComponent::setYawPitchRoll(float yaw, float pitch, float roll) {
        isDirty = true;
        yawPitchRoll = {yaw, pitch, roll};
    }

    bool TransformComponent::_calculateTopLevelTransforms() {
        if (isDirty) {
            globalPosition = localPosition;
            globalRotationVec = localRotationVec;
            globalRotationQuat = localRotationQuat;
            globalScale = localScale;

            if (yawPitchRoll.x != 0 || yawPitchRoll.y != 0 || yawPitchRoll.z != 0) {
                glm::vec3 direction = glm::normalize(glm::quat({yawPitchRoll.y, yawPitchRoll.x, yawPitchRoll.z}) * glm::vec3(0, 0, -1));
                localModelMatrix = glm::inverse(glm::lookAt(localPosition, localPosition + direction, {0.0f, 1.0f, 0.0f}));
            } else {
                localModelMatrix = calculateModelMatrix(localPosition, localRotationVec, localScale);
            }
            modelMatrix = localModelMatrix;
            isDirty = false;
            physicsDirty = false;
            return true;
        } else if (physicsDirty) {
            modelMatrix = calculateModelMatrix(globalPosition, globalRotationQuat, globalScale);
            physicsDirty = false;
            return true;
        }
        return false;
    }

    void TransformComponent::_physicsUpdate(glm::vec3 pos, glm::quat rot) {
        globalRotationQuat = rot;
        globalRotationVec = glm::eulerAngles(rot);
        globalPosition = pos;

        physicsDirty = true;
    }

    bool TransformComponent::_calculateChildTransformsWithParent(const glm::mat4& parentModelMatrix, bool parentDirty) {
        if (isDirty || parentDirty) {
            if (isDirty) {
                if (yawPitchRoll.x != 0 || yawPitchRoll.y != 0 || yawPitchRoll.z != 0) {
                    glm::vec3 direction = glm::normalize(glm::quat({yawPitchRoll.y, yawPitchRoll.x, yawPitchRoll.z}) * glm::vec3(0, 0, -1));
                    localModelMatrix = glm::inverse(glm::lookAt(localPosition, localPosition + direction, {0.0f, 1.0f, 0.0f}));
                } else {
                    localModelMatrix = calculateModelMatrix(localPosition, localRotationVec, localScale);
                }
            }
            modelMatrix = parentModelMatrix * localModelMatrix;
            decomposeModelToGlobals();
            isDirty = false;
            physicsDirty = false;
            return true;
        } else if (physicsDirty) {
            modelMatrix = calculateModelMatrix(globalPosition, globalRotationQuat, globalScale);
            physicsDirty = false;
            return true;
        }
        return false;
    }

    glm::mat4 TransformComponent::calculateModelMatrix(glm::vec3& pos, glm::vec3& rot, glm::vec3& scale) {
        auto m = glm::mat4(1.0f);
        m = glm::translate(m, pos);
        m = glm::rotate(m, rot.x, glm::vec3(1, 0, 0));
        m = glm::rotate(m, rot.y, glm::vec3(0, 1, 0));
        m = glm::rotate(m, rot.z, glm::vec3(0, 0, 1));
        m = glm::scale(m, scale);
        return m;
    }

    glm::mat4 TransformComponent::calculateModelMatrix(glm::vec3& pos, glm::quat& rot, glm::vec3& scale) {
        auto m = glm::mat4(1.0f);
        m = glm::translate(m, pos);
        m *= glm::toMat4(rot);
        m = glm::scale(m, scale);
        return m;
    }

    void TransformComponent::decomposeModelToGlobals() {
        glm::mat4 model = modelMatrix;

        globalPosition = glm::vec3(model[3]);
        model[3] = glm::vec4(0.0f, 0.0f, 0.0f, model[3].w);

        glm::vec3 col[3];

        col[0] = glm::vec3(model[0]);
        col[1] = glm::vec3(model[1]);
        col[2] = glm::vec3(model[2]);

        globalScale.x = glm::length(col[0]);
        col[0] = glm::normalize(col[0]);

        globalScale.y = glm::length(col[1]);
        col[1] = glm::normalize(col[1]);

        globalScale.z = glm::length(col[2]);
        col[2] = glm::normalize(col[2]);

        float trace = col[0].x + col[1].y + col[2].z;
        if (trace > 0.0f) {
            float rot = glm::sqrt(trace + 1.0f);
            globalRotationQuat.w = 0.5f * rot;
            rot = 0.5f / rot;
            globalRotationQuat.x = rot * (col[1].z - col[2].y);
            globalRotationQuat.y = rot * (col[2].x - col[0].z);
            globalRotationQuat.z = rot * (col[0].y - col[1].x);
        } else {
            int i, j, k = 0;
            int next[3] = {1, 2, 0};
            i = 0;
            if (col[1].y > col[0].x) i = 1;
            if (col[2].z > col[i][i]) i = 2;
            j = next[i];
            k = next[j];

            float rot = glm::sqrt(col[i][i] - col[j][j] - col[k][k] + 1.0f);

            globalRotationQuat[i] = 0.5f * rot;
            rot = 0.5f / rot;
            globalRotationQuat[j] = rot * (col[i][j] + col[j][i]);
            globalRotationQuat[k] = rot * (col[i][k] + col[k][i]);
            globalRotationQuat.w = rot * (col[j][k] - col[k][j]);
        }

        globalRotationVec = glm::eulerAngles(globalRotationQuat);
    }
}
