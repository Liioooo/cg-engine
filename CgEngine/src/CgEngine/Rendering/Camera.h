#pragma once

namespace CgEngine {

    enum class CameraProjectionType {
        Perspective, Orthographic
    };

    class Camera {
    public:
        CameraProjectionType getProjectionType() const;
        void setProjectionType(CameraProjectionType projectionType);
        float getPerspectiveFov() const;
        void setPerspectiveFov(float perspectiveFov);
        float getPerspectiveNear() const;
        void setPerspectiveNear(float perspectiveNear);
        float getPerspectiveFar() const;
        void setPerspectiveFar(float perspectiveFar);
        float getOrthographicSize() const;
        void setOrthographicSize(float orthographicSize);
        float getOrthographicNear() const;
        void setOrthographicNear(float orthographicNear);
        float getOrthographicFar() const;
        void setOrthographicFar(float orthographicFar);

        float getExposure() const;
        void setExposure(float exposure);

        float getBloomIntensity() const;
        void setBloomIntensity(float intensity);

        float getBloomThreshold() const;
        void setBloomThreshold(float threshold);

        float getAspectRatio() const;

        const glm::mat4& getProjectionMatrix() const;

        void setViewportSize(int width, int height);

    private:
        CameraProjectionType projectionType = CameraProjectionType::Perspective;

        float perspectiveFov = glm::radians(60.0f);
        float perspectiveNear = 0.1f;
        float perspectiveFar = 100.0f;

        float orthographicSize = 10.0f;
        float orthographicNear = -1.0f;
        float orthographicFar = 1.0f;

        float exposure = 1.0f;

        float bloomIntensity = 1.0f;
        float bloomThreshold = 0.0f;

        float aspectRatio = 1;

        glm::mat4 projectionMatrix;

        void calculateProjection();
    };

}
