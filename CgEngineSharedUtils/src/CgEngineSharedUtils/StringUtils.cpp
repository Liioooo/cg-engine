#include "StringUtils.h"
#include <charconv>
#include <regex>

namespace CgEngine::StringUtils {

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

    std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
        // https://stackoverflow.com/a/24315631/11587294
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
        }
        return str;
    }

    std::optional<int32_t> toInt(std::string_view s) {
        int32_t value;
        auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
        if (ec == std::errc()) {
            return value;
        } else {
            return std::nullopt;
        }
    }

    std::optional<float> toFloat(std::string_view s) {
        try {
            return std::stof(std::string(s));
        } catch (...) {
            return std::nullopt;
        }
    }

    bool toBool(std::string_view s) {
        char first = s.empty() ? '0' : s[0];
        return (first == '1' || first == 't' || first == 'T' || first == 'y' || first == 'Y');
    }

    std::string fromBool(bool value) {
        return value ? "true" : "false";
    }
}

