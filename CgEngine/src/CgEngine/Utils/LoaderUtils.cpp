#include <charconv>
#include "LoaderUtils.h"
#include "StringUtils.h"

namespace CgEngine::Utils::LoaderUtils {
    glm::vec3 stringTupleToVec3(std::string_view s) {
        size_t p0 = 0;
        size_t p1 = s.find(' ');
        float x = Utils::String::toFloat(s.substr(p0, p1)).value_or(0.0f);
        p0 = p1 + 1;
        p1 = s.find(' ', p0);
        float y = Utils::String::toFloat(s.substr(p0, p1)).value_or(0.0f);
        float z = Utils::String::toFloat(s.substr(p1 + 1)).value_or(0.0f);

        return {x, y, z};
    }

    glm::vec4 stringTupleToVec4(std::string_view s) {
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

    glm::vec3 hexStringToColor(std::string_view s) {
        if (s.empty()) return {0.0f, 0.0f, 0.0f};
        if (s.front() == '#')
            s.remove_prefix(1);

        uint32_t color;
        auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), color, 16);
        if (ec != std::errc()) return {0.0f, 0.0f, 0.0f};;

        float r = ((color >> 16) & 0xFF) / 255.0f;
        float g = ((color >> 8)  & 0xFF) / 255.0f;
        float b = ( color        & 0xFF) / 255.0f;

        return glm::vec3{r, g, b};
    }

    std::vector<std::string> getListFromString(std::string_view s) {
        std::vector<std::string> result;
        size_t start = 0;

        while (true) {
            size_t end = s.find(',', start);
            if (end == std::string_view::npos) {
                result.emplace_back(s.substr(start)); // last segment
                break;
            }
            result.emplace_back(s.substr(start, end - start));
            start = end + 1;
        }

        return result;
    }

    std::vector<uint32_t> getUint32ListFromString(std::string_view s) {
        std::vector<uint32_t> result;
        size_t start = 0;

        while (true) {
            size_t end = s.find(',', start);
            if (end == std::string_view::npos) {
                result.emplace_back(Utils::String::toInt(s.substr(start)).value_or(0)); // last segment
                break;
            }
            result.emplace_back(Utils::String::toInt(s.substr(start, end - start)).value_or(0));
            start = end + 1;
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

    std::string vec3ToStringTuple(const glm::vec3& vec) {
        std::stringstream stream;
        stream << vec.x << ' ' << vec.y << ' ' << vec.z;
        return stream.str();
    }

    std::string vec4ToStringTuple(const glm::vec4& vec) {
        std::stringstream stream;
        stream << vec.x << ' ' << vec.y << ' ' << vec.z << ' ' << vec.w;
        return stream.str();
    }
}
