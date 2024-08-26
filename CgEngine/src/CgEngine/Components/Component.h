#pragma once

#include "Scene/Entity.h"

namespace CgEngine {

    class Scene;

    class Component {
    public:
        explicit Component(Entity entity) : entity(entity) {};
        virtual ~Component() = default;

        virtual void onAttach(Scene& scene) {};
        virtual void onDetach(Scene& scene) {};

        inline Entity getEntity() const {
            return entity;
        }

    protected:
        Entity entity;
    };

}
