#pragma once
#include "CgEngineSharedUtils/Enums.h"

namespace CgEngine {

    class TextureCube {
    public:
        static TextureCube* createResource(const std::string& name);

        TextureCube() = default;

        virtual ~TextureCube() = default;

        TextureCube(TextureCube&& other) noexcept = default;
        TextureCube& operator=(TextureCube&& other) noexcept = default;

        TextureCube(TextureCube& other) = delete;
        TextureCube& operator=(TextureCube& other) = delete;

        virtual bool isReady() const = 0;
        virtual uint32_t getWidth() const = 0;
        virtual uint32_t getHeight() const = 0;
        virtual TextureFormat getFormat() const = 0;

        virtual void generateMipMaps() = 0;
    };

}
