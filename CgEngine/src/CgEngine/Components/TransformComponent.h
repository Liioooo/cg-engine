#pragma once

#include "Component.h"

namespace CgEngine {

    struct TransformComponentParams {
        glm::vec3 position = {0.0f, 0.0f, 0.0f};
        glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
        glm::vec3 scale {1.0f, 1.0f, 1.0f};

        void verifyParams() const;
    };

    class TransformComponent : public Component {
    public:
        using Component::Component;

        void onAttach(Scene& scene, TransformComponentParams& params);
        void onRenderImGui() override;

        const glm::vec3& getLocalPosition() const;
        const glm::vec3& getLocalRotationVec() const;
        const glm::quat & getLocalRotationQuat() const;
        const glm::vec3& getLocalScale() const;

        const glm::vec3& getGlobalPosition() const;
        const glm::vec3& getGlobalRotationVec() const;
        const glm::quat& getGlobalRotationQuat() const;
        const glm::vec3& getGlobalScale() const;

        const glm::mat4& getModelMatrix() const;

        void setLocalPosition(glm::vec3 position);
        void setLocalRotationVec(glm::vec3 rotation);
        void setLocalRotationQuat(glm::quat rotation);
        void setLocalScale(glm::vec3 scale);
        void setYawPitchRoll(float yaw, float pitch, float roll);
        void setLocalModalMatrix(const glm::mat4& mat);

        void _physicsUpdate(glm::vec3 pos, glm::quat rot);
        bool _calculateTopLevelTransforms();
        bool _calculateChildTransformsWithParent(const glm::mat4& parentModelMatrix, bool parentDirty);

    private:
        glm::vec3 localPosition;
        glm::vec3 localRotationVec;
        glm::quat localRotationQuat;
        glm::vec3 localScale;
        glm::vec3 yawPitchRoll{0, 0, 0};

        glm::vec3 globalPosition;
        glm::vec3 globalRotationVec;
        glm::quat globalRotationQuat;
        glm::vec3 globalScale;

        glm::mat4 localModelMatrix;
        glm::mat4 modelMatrix;

        bool isDirty = true;
        bool physicsDirty = false;
        bool localModalMatrixDirty = false;

        glm::mat4 calculateModelMatrix(glm::vec3& pos, glm::vec3& rot, glm::vec3& scale);
        glm::mat4 calculateModelMatrix(glm::vec3& pos, glm::quat& rot, glm::vec3& scale);
        void decomposeModel(glm::mat4 model, glm::vec3& posTarget, glm::vec3& scaleTarget, glm::vec3& rotVecTarget, glm::quat& rotQuatTarget);
    };

}
