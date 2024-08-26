#include "SkyboxComponent.h"
#include "Rendering/Renderer.h"

namespace CgEngine {
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
