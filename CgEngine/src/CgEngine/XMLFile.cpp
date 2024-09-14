#include "XMLFile.h"
#include "FileSystem.h"
#include "Asserts.h"

namespace CgEngine {
    const pugi::xml_document& XMLFile::getXMLDocument() {
        return xmlDocument;
    }

    void XMLFile::load(const std::string& name) {
        CG_ASSERT(FileSystem::checkFileExists(name), "XMLFile cannot be loaded: " + name + " does not exist.")
        xmlDocument.load_string(FileSystem::readFileToString(name).c_str());
        loaded = true;
    }

    bool XMLFile::isLoaded() {
        return loaded;
    }

}
