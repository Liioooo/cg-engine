#include "PropertyTestScript.h"

namespace RTR {

    void PropertyTestScript::onAttach() {
        CgEngine::ResRef<CgEngine::CustomComputeShader> s = getResource<CgEngine::CustomComputeShader>("custom-compute");
        CG_LOGGING_INFO("Shader: {0}", s->getProgramId());
    }

    void PropertyTestScript::update(CgEngine::TimeStep ts) {

    }
}
