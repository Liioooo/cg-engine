#include "Texture2D.h"
#include "Asserts.h"
#include "GraphicsObjectsFactory.h"

namespace CgEngine {

    Texture2D* Texture2D::createResource(const std::string& name) {
        return createResource(name, {});
    }

    Texture2D* Texture2D::createResource(const std::string& name, const Texture2DResourceSpecification& spec) {
        return GraphicsObjectsFactory::createTexture2D(name, spec.srgb, spec.wrap, spec.mipMapFiltering);
    }

    Texture2DBuilder::Texture2DBuilder(TextureFormat format, uint32_t width, uint32_t height) : format(format), width(width), height(height) {
        uint32_t bpp = getBytesPerPixel(format);
        uint32_t stride = (width * bpp + 3) & ~3;
        pixels.resize(stride * height, 0);
    }

    void Texture2DBuilder::setPixel(int x, int y, const void *data) {
        CG_ASSERT(x >= 0 && x < width, "Pixel x coordinate out of bounds");
        CG_ASSERT(y >= 0 && y < height, "Pixel y coordinate out of bounds");

        const uint32_t bpp = getBytesPerPixel(format);
        uint32_t stride = (width * bpp + 3) & ~3;

        uint8_t* dst = pixels.data() + (y * stride) + (x * bpp);
        memcpy(dst, data, bpp);
    }

    void Texture2DBuilder::setSubRegion(int x, int y, int w, int h, const void* data) {
        CG_ASSERT(x >= 0 && x + w <= width, "Sub-region x coordinates out of bounds");
        CG_ASSERT(y >= 0 && y + h <= height, "Sub-region y coordinates out of bounds");

        const uint32_t bpp = getBytesPerPixel(format);
        const uint32_t stride = (width * bpp + 3) & ~3;

        const uint8_t* src = static_cast<const uint8_t*>(data);
        uint8_t* dstBase = pixels.data();

        for (int row = 0; row < h; ++row) {
            uint8_t* dst = dstBase + ((y + row) * stride) + (x * bpp);
            const uint8_t* srcRow = src + row * w * bpp;

            memcpy(dst, srcRow, w * bpp);
        }
    }

    void Texture2DBuilder::setSubRegionWithPitch(int x, int y, int w, int h, const void* data, int srcPitchBytes) {
        CG_ASSERT(x >= 0 && x + w <= width, "Sub-region x coordinates out of bounds");
        CG_ASSERT(y >= 0 && y + h <= height, "Sub-region y coordinates out of bounds");

        const uint32_t bpp = getBytesPerPixel(format);
        const uint32_t stride = (width * bpp + 3) & ~3;

        const uint8_t* src = static_cast<const uint8_t*>(data);
        uint8_t* dstBase = pixels.data();

        for (int row = 0; row < h; ++row) {
            uint8_t* dst = dstBase + ((y + row) * stride) + (x * bpp);
            const uint8_t* srcRow = src + row * srcPitchBytes;

            memcpy(dst, srcRow, w * bpp);
        }
    }

    Texture2D* Texture2DBuilder::build(TextureWrap wrap, MipMapFiltering mipMapFiltering) const {
        return GraphicsObjectsFactory::createTexture2D(format, width, height, wrap, pixels.data(), mipMapFiltering);
    }

    uint32_t Texture2DBuilder::getBytesPerPixel(TextureFormat format) const {
        switch (format) {
            case TextureFormat::R:                  return 1;
            case TextureFormat::RG:                 return 2;
            case TextureFormat::RGBA:               return 4;
            case TextureFormat::RGBA_SRGB:          return 4;

            case TextureFormat::RedFloat16:         return 2;
            case TextureFormat::RedFloat32:         return 4;

            case TextureFormat::RedGreenFloat16:    return 4; // 2 * 16-bit
            case TextureFormat::RedGreenFloat32:    return 8; // 2 * 32-bit

            case TextureFormat::Float16A:           return 8;  // RGBA16F
            case TextureFormat::Float32A:           return 16; // RGBA32F
        }

        return 0;
    }
}
