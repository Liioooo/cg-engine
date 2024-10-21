#pragma once

#include "pugixml.hpp"

namespace CgEngine {

    class XMLFile {
    public:
        explicit XMLFile() = default;

        void load(const std::string& name);
        bool isLoaded();

        const pugi::xml_document& getXMLDocument();

    private:
        pugi::xml_document xmlDocument;
        bool loaded = false;
    };

}
