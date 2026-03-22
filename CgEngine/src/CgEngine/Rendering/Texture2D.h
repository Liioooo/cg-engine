#pragma once

#include "CgEngineSharedUtils/Enums.h"

namespace CgEngine {

    struct Texture2DResourceSpecification {
        bool srgb = false;
        TextureWrap wrap = TextureWrap::Repeat;
        MipMapFiltering mipMapFiltering = MipMapFiltering::Anisotropic;
    };

    class Texture2D {
    public:
        static Texture2D* createResource(const std::string& name);
        static Texture2D* createResource(const std::string& name, const Texture2DResourceSpecification& spec);

        Texture2D() = default;

        virtual ~Texture2D() = default;

        Texture2D(Texture2D&& other) noexcept = default;
        Texture2D& operator=(Texture2D&& other) noexcept = default;

        Texture2D(Texture2D& other) = delete;
        Texture2D& operator=(Texture2D& other) = delete;

        virtual uint32_t getWidth() const = 0;
        virtual uint32_t getHeight() const = 0;
        virtual TextureFormat getFormat() const = 0;
    };

    class Texture2DBuilder {
    public:
        Texture2DBuilder(TextureFormat format, uint32_t width, uint32_t height);

        void setPixel(int x, int y, const void* data);
        void setSubRegion(int x, int y, int w, int h, const void* data);
        void setSubRegionWithPitch(int x, int y, int w, int h, const void* data, int srcPitchBytes);

        Texture2D* build(TextureWrap wrap, MipMapFiltering mipMapFiltering) const;

    private:
        TextureFormat format;
        uint32_t width;
        uint32_t height;

        std::vector<unsigned char> pixels;

        uint32_t getBytesPerPixel(TextureFormat format) const;
    };

}
