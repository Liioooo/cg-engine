#include "XMLFile.h"
#include "FileSystem.h"
#include "Asserts.h"

namespace CgEngine {
    XMLFile* CgEngine::XMLFile::createResource(const std::string &name) {
        return new XMLFile(name);
    }

    XMLFile::XMLFile(const std::string &name) {
        CG_ASSERT(FileSystem::checkFileExists(name), "Scene file " + name + "does not exist!")
        xmlDocument.load_string(FileSystem::readFileToString(name).c_str());
    }

    const pugi::xml_document& XMLFile::getXMLDocument() {
        return xmlDocument;
    }

}
