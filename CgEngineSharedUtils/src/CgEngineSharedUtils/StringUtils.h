#pragma once

#include "string"
#include "vector"
#include "optional"

namespace CgEngine::StringUtils {
    bool equalsIgnoreCase(std::string_view a, std::string_view b);
    std::string getExtension(const std::string& filename);
    std::vector<std::string> splitString(std::string_view string, char delimiter);
    bool startsWith(std::string_view string, const std::string& test);
    bool matches(std::string_view string, const std::string& pattern);
    std::string replaceAll(std::string str, const std::string& from, const std::string& to);

    std::optional<int32_t> toInt(std::string_view s);
    std::optional<float> toFloat(std::string_view s);
    bool toBool(std::string_view s);

    std::string fromBool(bool value);
}


