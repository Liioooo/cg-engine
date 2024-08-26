#include "ScriptManager.h"
#include "Asserts.h"

namespace CgEngine {
    std::shared_ptr<NativeScript> ScriptManager::getScriptInstance(const std::string& name) {
        CG_ASSERT(scriptMap.find(name) != scriptMap.end(), "Script not registered: " + name)

        return scriptMap.at(name)();
    }
}
