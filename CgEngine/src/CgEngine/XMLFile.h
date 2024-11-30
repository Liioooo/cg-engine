#pragma once

#include "pugixml.hpp"

namespace CgEngine {

    class XMLFile {
    public:
        XMLFile() = default;

        void load(const std::filesystem::path& name);
        bool isLoaded();

        const pugi::xml_document& getXMLDocument();

    private:
        pugi::xml_document xmlDocument;
        bool loaded = false;
    };

}
