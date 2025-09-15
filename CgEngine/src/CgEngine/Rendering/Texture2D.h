#pragma once

#include "Enums.h"

namespace CgEngine {

    struct Texture2DResourceSpecification {
        bool srgb = false;
        TextureWrap wrap = TextureWrap::Repeat;
        MipMapFiltering mipMapFiltering = MipMapFiltering::Trilinear;
        float anisotropicFiltering = 1.0f;
        bool compression = false;
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
        virtual bool isCompressed() const = 0;
        virtual void bufferSubData(int x, int y, int w, int h, const void* data, int alignment) = 0;
    };

}
