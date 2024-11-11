#include "PropertyTestScript.h"

namespace RTR {

    void PropertyTestScript::onAttach() {
        CgEngine::ResRef<CgEngine::CustomComputeShader> s = getResource<CgEngine::CustomComputeShader>("custom-compute");
        CG_LOGGING_INFO("Shader: {0}", s->getProgramId());
        CG_LOGGING_INFO("Prop Test {0}", getParameterMap().getAsFloat("test").value())
        auto vec = getParameterMap().getAsVec3("vec").value();
        CG_LOGGING_INFO("Prop Vec {0}, {1}, {2}", vec.x, vec.y, vec.z)


    }

    void PropertyTestScript::update(CgEngine::TimeStep ts) {

    }
}
