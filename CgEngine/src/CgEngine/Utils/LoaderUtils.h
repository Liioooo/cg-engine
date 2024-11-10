#pragma once

namespace CgEngine::Utils::LoaderUtils {
    glm::vec3 stringTupleToVec3(const std::string& s);
    glm::vec3 hexStringToColor(const std::string& s);
    std::vector<std::string> getListFromString(const std::string& s);
}


