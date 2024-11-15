#include "SkyboxComponent.h"
#include "Rendering/Renderer.h"
#include "Asserts.h"
#include "imgui.h"

namespace CgEngine {
    void SkyboxComponentParams::verifyParams() const {
        CG_ASSERT(!hdriPath.empty(), "SkyboxComponentParams: 'hdriPath' is required.")
    }

    void SkyboxComponent::onAttach(Scene &scene, SkyboxComponentParams &params) {
        auto maps = Renderer::createEnvironmentMap(params.hdriPath);
        irradianceMap = maps.first;
        prefilterMap = maps.second;
        intensity = params.intensity;
        lod = params.lod;
    }

    const ResRef<TextureCube> SkyboxComponent::getIrradianceMap() const {
        return irradianceMap;
    }

    const ResRef<TextureCube> SkyboxComponent::getPrefilterMap() const {
        return prefilterMap;
    }

    float SkyboxComponent::getIntensity() const {
        return intensity;
    }

    float SkyboxComponent::getLod() const {
        return lod;
    }

    void SkyboxComponent::onRenderImGui() {
        if (ImGui::CollapsingHeader("SkyboxComponent")) {
            ImGui::DragFloat("Intesity", &intensity, 0.001, 0.0f, 1.0f);
            ImGui::DragFloat("LOD", &lod, 0.01, 0.0f);
        }
    }
}
