#pragma once

#include "Rendering/Texture.h"
#include "Component.h"
#include "Resources/ResRef.h"

namespace CgEngine {

    struct SkyboxComponentParams {
        std::string hdriPath;
        float intensity = 1.0f;
        float lod = 1.0f;

        void verifyParams() const;
    };

    class SkyboxComponent : public Component{
    public:
        using Component::Component;

        void onAttach(Scene& scene, SkyboxComponentParams& params);

        const ResRef<TextureCube> getIrradianceMap() const;
        const ResRef<TextureCube> getPrefilterMap() const;
        float getIntensity() const;
        float getLod() const;

    private:
        ResRef<TextureCube> irradianceMap;
        ResRef<TextureCube> prefilterMap;
        float intensity;
        float lod;
    };

}
