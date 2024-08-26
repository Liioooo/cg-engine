#pragma once

#include "pugixml.hpp"

namespace CgEngine {

    class XMLFile {
    public:
        static XMLFile* createResource(const std::string& name);

        explicit XMLFile(const std::string& name);

        const pugi::xml_document& getXMLDocument();

    private:
        pugi::xml_document xmlDocument;
    };

}
