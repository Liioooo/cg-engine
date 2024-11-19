#pragma once

namespace CgEngine::Utils::LoaderUtils {
    glm::vec3 stringTupleToVec3(const std::string& s);
    glm::vec4 stringTupleToVec4(const std::string& s);
    glm::vec3 hexStringToColor(const std::string& s);
    std::vector<std::string> getListFromString(const std::string& s);

    std::string vec3ColorToHexString(const glm::vec3& color);
    std::string vec3ToStringTuple(const glm::vec3& vec);
    std::string vec4ToStringTuple(const glm::vec4& vec);
}


