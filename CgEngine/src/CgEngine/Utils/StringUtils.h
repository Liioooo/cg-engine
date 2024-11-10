#pragma once

namespace CgEngine::Utils::String {
    bool equalsIgnoreCase(std::string_view a, std::string_view b);
    std::string getExtension(const std::string& filename);
    std::vector<std::string> splitString(std::string_view string, char delimiter);
}


