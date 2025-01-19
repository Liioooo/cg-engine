#pragma once

namespace CgEngine::Utils::String {
    bool equalsIgnoreCase(std::string_view a, std::string_view b);
    std::string getExtension(const std::string& filename);
    std::vector<std::string> splitString(std::string_view string, char delimiter);
    bool startsWith(std::string_view string, const std::string& test);
    bool matches(std::string_view string, const std::string& pattern);
    std::string replaceAll(std::string str, const std::string& from, const std::string& to);

    std::optional<int32_t> toInt(const std::string& s);
    std::optional<float> toFloat(const std::string& s);
    bool toBool(const std::string& s);

    std::string fromBool(bool value);
}


