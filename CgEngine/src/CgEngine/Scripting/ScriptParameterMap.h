#pragma once

namespace CgEngine {

    class ScriptParameterMap {
    public:
        void insert(const std::string& key, const std::string& value);

        std::optional<std::string> getAsString(const std::string& key) const;
        std::optional<int32_t> getAsInt(const std::string& key) const;
        std::optional<float> getAsFloat(const std::string& key) const;
        std::optional<bool> getAsBool(const std::string& key) const;
        std::optional<glm::vec3> getAsVec3(const std::string& key) const;
        std::optional<std::vector<std::string>> getAsList(const std::string& key) const;
        std::optional<glm::vec3> getAsColor(const std::string& key) const;

    private:
        std::unordered_map<std::string, std::string> map;
    };

}
