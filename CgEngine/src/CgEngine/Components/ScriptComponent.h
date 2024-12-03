#pragma once

#include "Events/Event.h"
#include "Component.h"
#include "TimeStep.h"
#include "Scripting/ScriptParameterMap.h"

namespace CgEngine {

    class NativeScript;

    struct ScriptComponentParams {
        std::string scriptName;
        ScriptParameterMap parameterMap;

        void verifyParams() const;
    };

    class ScriptComponent : public Component {
    public:
        using Component::Component;
        using Params = ScriptComponentParams;

        void onAttach(Scene& scene, ScriptComponentParams& params);
        void onDetach(Scene& scene) override;
        void onRenderImGui() override;

        void fixedUpdate(TimeStep ts);
        void update(TimeStep ts);
        void lateUpdate(TimeStep ts);

        void onCollisionEnter(Entity other);
        void onCollisionExit(Entity other);

        void onTriggerEnter(Entity other);
        void onTriggerExit(Entity other);

        void onEvent(Event& event);

        template<typename S>
        S& getNativeScript() {
            return *dynamic_cast<S*>(script.get());
        }

    private:
        std::shared_ptr<NativeScript> script;
        ScriptParameterMap parameterMap;
    };

}


