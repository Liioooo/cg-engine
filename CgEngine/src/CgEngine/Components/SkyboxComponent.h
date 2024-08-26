#pragma once

#include "Rendering/Texture.h"
#include "Component.h"

namespace CgEngine {

    struct SkyboxComponentParams {
        std::string hdriPath;
        float intensity;
        float lod;
    };

    class SkyboxComponent : public Component{
    public:
        using Component::Component;

        void onAttach(Scene& scene, SkyboxComponentParams& params);

        const TextureCube* getIrradianceMap() const;
        const TextureCube* getPrefilterMap() const;
        float getIntensity() const;
        float getLod() const;

    private:
        TextureCube* irradianceMap;
        TextureCube* prefilterMap;
        float intensity;
        float lod;
    };

}
