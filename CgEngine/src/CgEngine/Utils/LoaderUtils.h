#pragma once

namespace CgEngine::Utils::LoaderUtils {
    glm::vec3 stringTupleToVec3(std::string_view s);
    glm::vec4 stringTupleToVec4(std::string_view s);
    glm::vec3 hexStringToColor(std::string_view s);
    std::vector<std::string> getListFromString(std::string_view s);
    std::vector<uint32_t> getUint32ListFromString(std::string_view s);

    std::string vec3ColorToHexString(const glm::vec3& color);
    std::string vec3ToStringTuple(const glm::vec3& vec);
    std::string vec4ToStringTuple(const glm::vec4& vec);
}


