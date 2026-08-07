#include "Texture2D.h"
#include "Asserts.h"
#include "FileSystem.h"
#include "GraphicsObjectsFactory.h"
#include "Helpers.h"
#include "stbi_image.h"

namespace CgEngine {

    Texture2D* Texture2D::createResource(const std::string& name) {
        return createResource(name, {});
    }

    Texture2D* Texture2D::createResource(const std::string& name, const Texture2DResourceSpecification& spec) {
        return GraphicsObjectsFactory::createTexture2D(name, spec.srgb, spec.wrap, spec.mipMapFiltering, spec.borderColor);
    }

    Texture2D::Texture2DLoadData Texture2D::loadTextureDataFromFile(const std::filesystem::path& path, bool srgb) {
        CG_ASSERT(FileSystem::checkFileExists(path), "Texture2D: " + path.string() + " does not exist!")

        Texture2DLoadData loadData{};

        int loadWidth, loadHeight, fileChannels;
        unsigned char* data = nullptr;

        std::string pathString = path.string();

        if (stbi_is_hdr(pathString.c_str())) {
            stbi_info(pathString.c_str(), &loadWidth, &loadHeight, &fileChannels);
            if (fileChannels == 3 || fileChannels == 4) {
                data = (unsigned char*)(stbi_loadf(pathString.c_str(), &loadWidth, &loadHeight, &fileChannels, STBI_rgb_alpha));
                loadData.format = TextureFormat::Float32A;
            } else if (fileChannels == 2) {
                data = (unsigned char*)(stbi_loadf(pathString.c_str(), &loadWidth, &loadHeight, &fileChannels, STBI_grey_alpha));
                loadData.format = TextureFormat::RedGreenFloat32;
            } else if (fileChannels == 1) {
                data = (unsigned char*)(stbi_loadf(pathString.c_str(), &loadWidth, &loadHeight, &fileChannels, STBI_grey));
                loadData.format = TextureFormat::RedFloat32;
            }
        } else {
            stbi_info(pathString.c_str(), &loadWidth, &loadHeight, &fileChannels);
            if (fileChannels == 3 || fileChannels == 4) {
                data = stbi_load(pathString.c_str(), &loadWidth, &loadHeight, &fileChannels, STBI_rgb_alpha);
                loadData.format = srgb ? TextureFormat::RGBA_SRGB : TextureFormat::RGBA;
            } else if (fileChannels == 2) {
                data = stbi_load(pathString.c_str(), &loadWidth, &loadHeight, &fileChannels, STBI_grey_alpha);
                loadData.format = TextureFormat::RG;
            } else if (fileChannels == 1) {
                data = stbi_load(pathString.c_str(), &loadWidth, &loadHeight, &fileChannels, STBI_grey);
                loadData.format = TextureFormat::R;
            }
        }

        if (!data) {
            CG_LOGGING_ERROR("Failed to load texture: {0}", pathString)
            return {};
        }

        loadData.data = data;
        loadData.width = loadWidth;
        loadData.height = loadHeight;

        return loadData;
    }

    Texture2D::Texture2DLoadData Texture2D::loadTextureDataFromMemory(const unsigned char *buffer, int bufferLen, bool srgb) {
        Texture2DLoadData loadData{};

        int loadWidth, loadHeight, fileChannels;
        unsigned char* data = nullptr;

        if (stbi_is_hdr_from_memory(buffer, bufferLen)) {
            stbi_info_from_memory(buffer, bufferLen, &loadWidth, &loadHeight, &fileChannels);
            if (fileChannels == 3 || fileChannels == 4) {
                data = (unsigned char*)(stbi_loadf_from_memory(buffer, bufferLen, &loadWidth, &loadHeight, &fileChannels, STBI_rgb_alpha));
                loadData.format = TextureFormat::Float32A;
            } else if (fileChannels == 2) {
                data = (unsigned char*)(stbi_loadf_from_memory(buffer, bufferLen, &loadWidth, &loadHeight, &fileChannels, STBI_grey_alpha));
                loadData.format = TextureFormat::RedGreenFloat32;
            } else if (fileChannels == 1) {
                data = (unsigned char*)(stbi_loadf_from_memory(buffer, bufferLen, &loadWidth, &loadHeight, &fileChannels, STBI_grey));
                loadData.format = TextureFormat::RedFloat32;
            }
        } else {
            stbi_info_from_memory(buffer, bufferLen, &loadWidth, &loadHeight, &fileChannels);
            if (fileChannels == 3 || fileChannels == 4) {
                data = stbi_load_from_memory(buffer, bufferLen, &loadWidth, &loadHeight, &fileChannels, STBI_rgb_alpha);
                loadData.format = srgb ? TextureFormat::RGBA_SRGB : TextureFormat::RGBA;
            } else if (fileChannels == 2) {
                data = stbi_load_from_memory(buffer, bufferLen, &loadWidth, &loadHeight, &fileChannels, STBI_grey_alpha);
                loadData.format = TextureFormat::RG;
            } else if (fileChannels == 1) {
                data = stbi_load_from_memory(buffer, bufferLen, &loadWidth, &loadHeight, &fileChannels, STBI_grey);
                loadData.format = TextureFormat::R;
            }
        }

        if (!data) {
            CG_LOGGING_ERROR("Failed to load texture: from buffer")
            return {};
        }

        loadData.data = data;
        loadData.width = loadWidth;
        loadData.height = loadHeight;

        return loadData;
    }

    Texture2DBuilder::Texture2DBuilder(TextureFormat format, uint32_t width, uint32_t height) : format(format), width(width), height(height), bytesPerPixel(Helpers::getBytesPerPixelForTextureFormat(format)), pitch(width * bytesPerPixel) {
        pixels.resize(pitch * height, 0);
    }

    void Texture2DBuilder::setPixel(int x, int y, const void *data) {
        CG_ASSERT(x >= 0 && x < width, "Pixel x coordinate out of bounds");
        CG_ASSERT(y >= 0 && y < height, "Pixel y coordinate out of bounds");

        uint8_t* dst = pixels.data() + (y * pitch) + (x * bytesPerPixel);
        memcpy(dst, data, bytesPerPixel);
    }

    void Texture2DBuilder::setSubRegion(int x, int y, int w, int h, const void* data) {
        setSubRegionWithPitch(x, y, w, h, data, static_cast<int>(w * bytesPerPixel));
    }

    void Texture2DBuilder::setSubRegionWithPitch(int x, int y, int w, int h, const void* data, int srcPitchBytes) {
        CG_ASSERT(x >= 0 && x + w <= width, "Sub-region x coordinates out of bounds");
        CG_ASSERT(y >= 0 && y + h <= height, "Sub-region y coordinates out of bounds");

        const uint8_t* src = static_cast<const uint8_t*>(data);
        uint8_t* dstBase = pixels.data();

        for (int row = 0; row < h; ++row) {
            uint8_t* dst = dstBase + ((y + row) * pitch) + (x * bytesPerPixel);
            const uint8_t* srcRow = src + row * srcPitchBytes;

            memcpy(dst, srcRow, w * bytesPerPixel);
        }
    }

    Texture2D* Texture2DBuilder::build(TextureWrap wrap, MipMapFiltering mipMapFiltering) const {
        return GraphicsObjectsFactory::createTexture2D(format, width, height, wrap, pixels.data(), mipMapFiltering);
    }
}
