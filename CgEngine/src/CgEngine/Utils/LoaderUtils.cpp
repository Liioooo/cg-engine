#include "LoaderUtils.h"

namespace CgEngine::Utils::LoaderUtils {
    glm::vec3 stringTupleToVec3(const std::string& s) {
        size_t p0 = 0;
        size_t p1 = s.find(' ');
        float x = std::stof(s.substr(p0, p1));
        p0 = p1 + 1;
        p1 = s.find(' ', p0);
        float y = std::stof(s.substr(p0, p1));
        float z = std::stof(s.substr(p1 + 1));

        return {x, y, z};
    }

    glm::vec3 hexStringToColor(const std::string& s) {
        uint64_t color = std::stoul(s.substr(1), nullptr, 16);
        float r = ((color >> 16) & 0xFF) / 255.0f;
        float g = ((color >> 8) & 0xFF) / 255.0f;
        float b = (color & 0xFF) / 255.0f;
        return {r, g, b};
    }

    std::vector<std::string> getListFromString(const std::string& s) {
        std::vector<std::string> result{};
        std::stringstream ss(s);
        std::string item;

        while (std::getline(ss, item, ',')) {
            result.emplace_back(item);
        }

        return result;
    }
}
