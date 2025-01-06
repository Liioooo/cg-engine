#include "StringUtils.h"

namespace CgEngine::Utils::String {
    bool equalsIgnoreCase(const std::string_view a, const std::string_view b) {
        if (a.size() != b.size()) {
            return false;
        }

        return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](const char a, const char b){
            return std::tolower(a) == std::tolower(b);
        });
    }

    std::string getExtension(const std::string& filename) {
        std::vector<std::string> parts = splitString(filename, '.');

        if (parts.size() > 1)
            return parts[parts.size() - 1];

        return "";
    }

    std::vector<std::string> splitString(const std::string_view string, const char delimiter) {
        size_t first = 0;

        std::vector<std::string> result;

        while (first <= string.size()) {
            const auto second = string.find_first_of(delimiter, first);

            if (first != second) {
                result.emplace_back(string.substr(first, second - first));
            }

            if (second == std::string_view::npos) {
                break;
            }

            first = second + 1;
        }
        return result;
    }

    bool startsWith(std::string_view string, const std::string& test) {
        return string.rfind(test, 0) == 0;
    }

    bool matches(std::string_view string, const std::string& pattern) {
        std::regex r(pattern);
        return std::regex_match(string.begin(), string.end(), r);
    }

    std::optional<int32_t> toInt(const std::string& s) {
        try {
            return std::stoi(s);
        } catch (...) {
            return std::nullopt;
        }
    }

    std::optional<float> toFloat(const std::string& s) {
        try {
            return std::stof(s);
        } catch (...) {
            return std::nullopt;
        }
    }

    bool toBool(const std::string& s) {
        char first = *s.c_str();
        return (first == '1' || first == 't' || first == 'T' || first == 'y' || first == 'Y');
    }

    std::string fromBool(bool value) {
        return value ? "true" : "false";
    }
}

