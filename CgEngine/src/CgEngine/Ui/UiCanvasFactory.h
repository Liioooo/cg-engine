#pragma once

#include <XMLFile.h>
#include <FileSystem.h>
#include "UiCanvas.h"

namespace CgEngine {

    class UiCanvasFactory {
    public:
        static UiCanvas* createUiCanvas(const std::string& uiCanvasName) {
            if (!xmlFile.isLoaded()) {
                xmlFile.load(FileSystem::getAsGamePath("ui-canvases.xml"));
            }

            const pugi::xml_document& xml = xmlFile.getXMLDocument();
            const auto& canvases = xml.child("UiCanvases");
            const auto& canvasNode = canvases.find_child_by_attribute("Canvas", "name", uiCanvasName.c_str());
            return new UiCanvas(canvasNode);
        }

    private:
        static inline XMLFile xmlFile;
    };
}
