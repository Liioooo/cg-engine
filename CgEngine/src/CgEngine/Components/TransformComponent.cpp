#include "TransformComponent.h"
#include "Scene/Scene.h"
#include "imgui.h"
#include "ImGui/ImGuiWidgets.h"
#include "CgEngineSharedUtils/LoaderUtils.h"

namespace CgEngine {
    void TransformComponentParams::verifyParams() const {}

    void TransformComponent::onAttach(Scene& scene, TransformComponentParams &params) {
        localPosition = params.position;
        localRotationVec = params.rotation;
        localRotationQuat = glm::quat(params.rotation);
        localScale = params.scale;
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

    void TransformComponent::setLocalModalMatrix(const glm::mat4& mat) {
        localModelMatrix = mat;
        localModalMatrixDirty = true;
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
                decomposeModel(modelMatrix, globalPosition, globalScale, globalRotationVec, globalRotationQuat);
                localRotationVec = globalRotationVec;
                localRotationQuat = globalRotationQuat;
            } else {
                localModelMatrix = calculateModelMatrix(localPosition, localRotationVec, localScale);
            }
            modelMatrix = localModelMatrix;
            isDirty = false;
            physicsDirty = false;
            localModalMatrixDirty = false;
            return true;
        } else if (localModalMatrixDirty) {
            modelMatrix = localModelMatrix;
            decomposeModel(modelMatrix, globalPosition, globalScale, globalRotationVec, globalRotationQuat);
            decomposeModel(localModelMatrix, localPosition, localScale, localRotationVec, localRotationQuat);
            physicsDirty = false;
            localModalMatrixDirty = false;
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
        if (isDirty || parentDirty || localModalMatrixDirty) {
            if (isDirty && !localModalMatrixDirty) {
                if (yawPitchRoll.x != 0 || yawPitchRoll.y != 0 || yawPitchRoll.z != 0) {
                    glm::vec3 direction = glm::normalize(glm::quat({yawPitchRoll.y, yawPitchRoll.x, yawPitchRoll.z}) * glm::vec3(0, 0, -1));
                    localModelMatrix = glm::inverse(glm::lookAt(localPosition, localPosition + direction, {0.0f, 1.0f, 0.0f}));
                } else {
                    localModelMatrix = calculateModelMatrix(localPosition, localRotationVec, localScale);
                }
            }
            if (localModalMatrixDirty) {
                decomposeModel(localModelMatrix, localPosition, localScale, localRotationVec, localRotationQuat);
            }
            modelMatrix = parentModelMatrix * localModelMatrix;
            decomposeModel(modelMatrix, globalPosition, globalScale, globalRotationVec, globalRotationQuat);
            isDirty = false;
            physicsDirty = false;
            localModalMatrixDirty = false;
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
        return glm::translate(glm::mat4(1.0f), pos) * glm::toMat4(rot) * glm::scale(glm::mat4(1.0f), scale);
    }

    void TransformComponent::decomposeModel(glm::mat4 model, glm::vec3& posTarget, glm::vec3& scaleTarget, glm::vec3& rotVecTarget, glm::quat& rotQuatTarget) {
        posTarget = glm::vec3(model[3]);
        model[3] = glm::vec4(0.0f, 0.0f, 0.0f, model[3].w);

        glm::vec3 col[3];

        col[0] = glm::vec3(model[0]);
        col[1] = glm::vec3(model[1]);
        col[2] = glm::vec3(model[2]);

        scaleTarget.x = glm::length(col[0]);
        col[0] = glm::normalize(col[0]);

        scaleTarget.y = glm::length(col[1]);
        col[1] = glm::normalize(col[1]);

        scaleTarget.z = glm::length(col[2]);
        col[2] = glm::normalize(col[2]);

        rotQuatTarget = glm::quat(glm::mat3(col[0], col[1], col[2]));
        rotVecTarget = glm::eulerAngles(rotQuatTarget);
    }

    void TransformComponent::onRenderImGui() {
        if (ImGui::CollapsingHeader("TransformComponent")) {
            ImGui::BeginDisabled();
            ImGui::DragFloat3("Global Position", glm::value_ptr(globalPosition));
            ImGui::DragFloat3("Global Rotation", glm::value_ptr(globalRotationVec));
            ImGui::DragFloat3("Global Scale", glm::value_ptr(globalScale));
            ImGui::EndDisabled();

            bool changed = false;
            changed |= ImGui::DragFloat3("Local Position", glm::value_ptr(localPosition), 0.1f);
            changed |= ImGui::DragFloat3("Local Rotation", glm::value_ptr(localRotationVec), 0.01f, 0, glm::two_pi<float>());
            changed |= ImGui::DragFloat3("Local Scale", glm::value_ptr(localScale), 0.05f);
            changed |= ImGui::DragFloat3("Yaw/Pitch/Roll", glm::value_ptr(yawPitchRoll), 0.05f);

            if (changed) {
                localRotationQuat = glm::quat(localRotationVec);
            }

            isDirty = changed;

            ImGuiWidgets::copyCurrentConfig("TransformComponent", [this](auto& map) {
                map["position"] = LoaderUtils::vec3ToStringTuple(localPosition);
                map["rotation"] = LoaderUtils::vec3ToStringTuple(localRotationVec);
                map["rotation-rads"] = "true";
                map["scale"] = LoaderUtils::vec3ToStringTuple(localScale);
            });
        }
    }
}
