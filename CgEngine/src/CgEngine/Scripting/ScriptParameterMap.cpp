#include "ScriptParameterMap.h"
#include "Utils/StringUtils.h"
#include "Utils/LoaderUtils.h"

namespace CgEngine {
    void ScriptParameterMap::insert(const std::string& key, const std::string& value) {
        map[key] = value;
    }

    std::optional<std::string> ScriptParameterMap::getAsString(const std::string& key) const {
        if (map.find(key) != map.end()) {
            return map.at(key);
        }
        return std::nullopt;
    }

    std::optional<int32_t> ScriptParameterMap::getAsInt(const std::string& key) const {
        if (map.find(key) != map.end()) {
            return Utils::String::toInt(map.at(key));
        }
        return std::nullopt;
    }

    std::optional<float> ScriptParameterMap::getAsFloat(const std::string& key) const {
        if (map.find(key) != map.end()) {
            return Utils::String::toFloat(map.at(key));
        }
        return std::nullopt;
    }

    std::optional<bool> ScriptParameterMap::getAsBool(const std::string& key) const {
        if (map.find(key) != map.end()) {
            return Utils::String::toBool(map.at(key));
        }
        return std::nullopt;
    }

    std::optional<glm::vec3> ScriptParameterMap::getAsVec3(const std::string& key) const {
        if (map.find(key) != map.end()) {
            return Utils::LoaderUtils::stringTupleToVec3(map.at(key));
        }
        return std::nullopt;
    }

    std::optional<std::vector<std::string>> ScriptParameterMap::getAsList(const std::string& key) const {
        if (map.find(key) != map.end()) {
            return Utils::LoaderUtils::getListFromString(map.at(key));
        }
        return std::nullopt;
    }

    std::optional<glm::vec3> ScriptParameterMap::getAsColor(const std::string& key) const {
        if (map.find(key) != map.end()) {
            return Utils::LoaderUtils::hexStringToColor(map.at(key));
        }
        return std::nullopt;
    }
}
