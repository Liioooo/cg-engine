#include "CameraComponent.h"
#include "Scene/Scene.h"
#include "imgui.h"
#include "ImGui/ImGuiWidgets.h"

namespace CgEngine {
    void CameraComponentParams::verifyParams() const {}

    void CameraComponent::onAttach(Scene& scene, CameraComponentParams &params) {
        camera = Camera();

        isPrimaryCam = params.isPrimary;
        camera.setViewportSize(scene.getViewportWidth(), scene.getViewportHeight());
        camera.setExposure(params.exposure);
        camera.setBloomIntensity(params.bloomIntensity);
        camera.setBloomThreshold(params.bloomThreshold);

        camera.setHbaoBias(params.hbaoBias);
        camera.setHbaoIntensity(params.hbaoIntensity);
        camera.setHbaoRadius(params.hbaoRadius);
        camera.setHbaoSharpness(params.hbaoSharpness);

        if (params.projection == "perspective") {
            camera.setProjectionType(CameraProjectionType::Perspective);
            camera.setPerspectiveFar(params.cfar);
            camera.setPerspectiveNear(params.cnear);
            camera.setPerspectiveFov(params.cfov);
        } else {
            camera.setProjectionType(CameraProjectionType::Orthographic);
            camera.setOrthographicFar(params.cfar);
            camera.setOrthographicNear(params.cnear);
            camera.setOrthographicSize(params.orthoSize);
        }
    }

    Camera& CameraComponent::getCamera() {
        return camera;
    }

    void CameraComponent::setPrimary(bool primary) {
        isPrimaryCam = primary;
    }

    bool CameraComponent::isPrimary() const {
        return isPrimaryCam;
    }

    void CameraComponent::onRenderImGui() {
        if (ImGui::CollapsingHeader("CameraComponent")) {
            ImGui::Checkbox("Is Primary", &isPrimaryCam);
            ImGuiWidgets::dragFloat("Exposure", &camera, &Camera::getExposure, &Camera::setExposure, 0.01f, 0.0f, 1.0f);
            ImGuiWidgets::dragFloat("Bloom Intensity", &camera, &Camera::getBloomIntensity, &Camera::setBloomIntensity, 0.01f, 0.0f, 1.0f);
            ImGuiWidgets::dragFloat("Bloom Threshold", &camera, &Camera::getBloomThreshold, &Camera::setBloomThreshold, 0.01f, 0.0f, 1.0f);
            ImGuiWidgets::dragFloat("HBAO Radius", &camera, &Camera::getHbaoRadius, &Camera::setHbaoRadius, 0.01f, 0.0f, 2.0f);
            ImGuiWidgets::dragFloat("HBAO Intensity", &camera, &Camera::getHbaoIntensity, &Camera::setHbaoIntensity, 0.01f, 0.0f, 2.0f);
            ImGuiWidgets::dragFloat("HBAO Bias", &camera, &Camera::getHbaoBias, &Camera::setHbaoBias, 0.01f, 0.0f, 1.0f);
            ImGuiWidgets::dragFloat("HBAO Sharpness", &camera, &Camera::getHbaoSharpness, &Camera::setHbaoSharpness, 0.01f, 0.0f, 2.0f);

            const char* type = camera.getProjectionType() == CameraProjectionType::Perspective ? "Perspective" : "Orthographic";

            if (ImGui::BeginCombo("Type", type)) {
                if (ImGui::Selectable("Perspective", camera.getProjectionType() == CameraProjectionType::Perspective)) {
                    camera.setProjectionType(CameraProjectionType::Perspective);
                }
                if (ImGui::Selectable("Orthographic", camera.getProjectionType() == CameraProjectionType::Orthographic)) {
                    camera.setProjectionType(CameraProjectionType::Orthographic);
                }
                ImGui::EndCombo();
            }

            ImGuiWidgets::inputFloat("Perspective Near", &camera, &Camera::getPerspectiveNear, &Camera::setPerspectiveNear);
            ImGuiWidgets::inputFloat("Perspective Far", &camera, &Camera::getPerspectiveFar, &Camera::setPerspectiveFar);
            ImGuiWidgets::inputFloat("Orthographic Near", &camera, &Camera::getOrthographicNear, &Camera::setOrthographicNear);
            ImGuiWidgets::inputFloat("Orthographic Far", &camera, &Camera::getOrthographicFar, &Camera::setOrthographicFar);
            ImGuiWidgets::inputFloat("Orthographic Size", &camera, &Camera::getOrthographicSize, &Camera::setOrthographicSize);
        }
    }
}
