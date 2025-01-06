#pragma once

#include <Resources/AudioFile.h>
#include "Component.h"
#include "Physics/PhysicsShape.h"
#include "Resources/ResRef.h"
#include "Uuid.h"

namespace CgEngine {

    struct LodDistanceComponentParams {
        std::vector<float> lodDistances;

        void verifyParams() const;
    };

    class LodDistanceComponent : public Component {
    public:
        using Component::Component;
        using Params = LodDistanceComponentParams;

        void onAttach(Scene& scene, LodDistanceComponentParams& params);
        void onRenderImGui() override;

        const std::vector<float>& getLodDistances() const;

    private:
        std::vector<float> lodDistances;
    };

}
