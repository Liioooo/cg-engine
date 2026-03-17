#pragma once

#include <random>
#include "string"
#include "string_view"
#include "vector"
#include "INIReader.h"
#include "glm/glm.hpp"
#include "CollectedAssets.h"
#include "filesystem"
#include "CgEngineSharedUtils/Enums.h"
#include "pugixml.hpp"

namespace AssetPacker {

    struct AssetPackerConfig {
        std::string configFilePath;
        std::string outputDirectory;
    };

    class AssetPacker {
    public:
        AssetPacker(int argc, char **argv);
        void run();

    private:
        AssetPackerConfig config;
        std::unique_ptr<INIReader> iniReader;

        CollectedAssets collectedAssets;

        std::tuple<CgEngine::MipMapFiltering, CgEngine::TextureWrap, bool> getTextureSettingsFromXMLNode(const pugi::xml_node& xmlNode);
        std::string convertTextureParamsToString(const std::tuple<CgEngine::MipMapFiltering, CgEngine::TextureWrap, bool>& params);
        void packMaterialsFromXML(const std::filesystem::path& materialsXMLPath);
        CgEngine::Assets::AssetID packTextureFromFilePathAndParams(const std::string& filePath, const std::tuple<CgEngine::MipMapFiltering, CgEngine::TextureWrap, bool>& params);
        CgEngine::Assets::AssetID packImageFromFilePath(const std::string& filePath);
        void packPhysicsMaterialsFromXML(const std::filesystem::path& physicsMaterialsXMLPath);
    };

}
