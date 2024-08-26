#pragma once

#include "NativeScript.h"

namespace CgEngine {

    class ScriptManager {
    public:
        template<typename S>
        void registerNativeScript(const std::string& name) {
            scriptMap.insert({name, S::instantiateScript});
        }

        std::shared_ptr<NativeScript> getScriptInstance(const std::string& name);

    private:
        std::unordered_map<std::string, std::shared_ptr<NativeScript>(*)()> scriptMap;

    };

}
