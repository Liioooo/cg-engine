#pragma once

#include "Events/Event.h"
#include "Component.h"
#include "TimeStep.h"
#include "Scripting/ScriptParameterMap.h"
#include "Scene/EntityHandle.h"

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
        void onEnable(Scene& scene) override;
        void onDetach(Scene& scene) override;
        void onRenderImGui() override;

        void fixedUpdate(TimeStep ts);
        void update(TimeStep ts);
        void lateUpdate(TimeStep ts);

        void onCollisionEnter(EntityHandle other);
        void onCollisionExit(EntityHandle other);

        void onTriggerEnter(EntityHandle other);
        void onTriggerExit(EntityHandle other);

        void onEvent(Event& event);

        template<typename S>
        S& getNativeScript() {
            return *dynamic_cast<S*>(script.get());
        }

        const ScriptParameterMap& getParameterMap() const;

    private:
        std::shared_ptr<NativeScript> script;
        ScriptParameterMap parameterMap;
    };

}


