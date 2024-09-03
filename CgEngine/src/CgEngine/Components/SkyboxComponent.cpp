#include "SkyboxComponent.h"
#include "Rendering/Renderer.h"
#include "Asserts.h"

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

    const TextureCube* SkyboxComponent::getIrradianceMap() const {
        return irradianceMap;
    }

    const TextureCube* SkyboxComponent::getPrefilterMap() const {
        return prefilterMap;
    }

    float SkyboxComponent::getIntensity() const {
        return intensity;
    }

    float SkyboxComponent::getLod() const {
        return lod;
    }
}
