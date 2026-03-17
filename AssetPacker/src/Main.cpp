#include "AssetPackerLogging.h"
#include "AssetPacker.h"

int main(int argc, char **argv) {
    AssetPacker::AssetPackerLogging::init();

    AP_LOGGING_INFO("Starting Asset Packer");

    AssetPacker::AssetPacker assetPacker(argc, argv);
    assetPacker.run();

    AP_LOGGING_INFO("Asset Packer finished successfully");
}
