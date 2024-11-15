#include "LoaderUtils.h"
#include "StringUtils.h"

namespace CgEngine::Utils::LoaderUtils {
    glm::vec3 stringTupleToVec3(const std::string& s) {
        size_t p0 = 0;
        size_t p1 = s.find(' ');
        float x = Utils::String::toFloat(s.substr(p0, p1)).value_or(0.0f);
        p0 = p1 + 1;
        p1 = s.find(' ', p0);
        float y = Utils::String::toFloat(s.substr(p0, p1)).value_or(0.0f);
        float z = Utils::String::toFloat(s.substr(p1 + 1)).value_or(0.0f);

        return {x, y, z};
    }

    glm::vec4 stringTupleToVec4(const std::string& s) {
        size_t p0 = 0;
        size_t p1 = s.find(' ');
        float x = Utils::String::toFloat(s.substr(p0, p1)).value_or(0.0f);
        p0 = p1 + 1;
        p1 = s.find(' ', p0);
        float y = Utils::String::toFloat(s.substr(p0, p1)).value_or(0.0f);
        p1++;
        p0 = s.find(' ', p1);
        float z = Utils::String::toFloat(s.substr(p1, p0)).value_or(0.0f);
        float w = Utils::String::toFloat(s.substr(p0 + 1)).value_or(0.0f);

        return {x, y, z, w};
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

    std::string vec3ColorToHexString(const glm::vec3& color) {
        int r = static_cast<int>(glm::round(color.r * 255.0f));
        int g = static_cast<int>(glm::round(color.g * 255.0f));
        int b = static_cast<int>(glm::round(color.b * 255.0f));

        std::stringstream stream;
        stream << std::hex << r << g << b;
        return "#" + stream.str();
    }
}
