#include "LoadingScreenScript.h"

namespace RTR {
    void LoadingScreenScript::update(CgEngine::TimeStep ts) {
        updateCount++;

        if (updateCount == 2) {
            setActiveScene("scenes/start_scene.xml");
        }
    }
}
