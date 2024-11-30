#pragma once

namespace CgEngine {

    enum class TextureFormat {
        R, RedFloat16, RedFloat32, RedGreenFloat16, RedGreenFloat32, RGB, RGBA, Float16A, Float32A, Float16, Float32, Depth
    };

    enum class TextureWrap {
        Repeat, Clamp, ClampBorder
    };

    enum class MipMapFiltering {
        Nearest, Bilinear, Trilinear
    };

    namespace TextureUtils {
        int getOpenGLTextureInternalFormat(TextureFormat format, bool compression);
        int getOpenGLTextureFormatForImageBind(TextureFormat format);
        int getOpenGLTextureType(TextureFormat format);
        int getOpenGLTextureFormat(TextureFormat format);
        int getTextureWrap(TextureWrap wrap);
        void setClampBorderColor(const glm::vec4& color, unsigned int textureType);
        void applyMipMapFiltering(MipMapFiltering mipMapFiltering, unsigned int textureType);
        uint32_t calculateMipCount(uint32_t width, uint32_t height);
        std::tuple<unsigned char*, int, int> loadImageData(const std::filesystem::path& path);
        void freeImageData(unsigned char* data);
    }

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
        Texture2D(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap, MipMapFiltering mipMapFiltering = MipMapFiltering::Trilinear, float anisotropicFiltering = 1.0f, bool compression = false);
        Texture2D(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap, const void* data, MipMapFiltering mipMapFiltering = MipMapFiltering::Trilinear, float anisotropicFiltering = 1.0f, bool compression = false);
        Texture2D(const std::filesystem::path& path, bool srgb, TextureWrap wrap = TextureWrap::Repeat, MipMapFiltering mipMapFiltering = MipMapFiltering::Trilinear, float anisotropicFiltering = 1.0f, bool compression = false);
        Texture2D(const unsigned char* buffer, int bufferLen, bool srgb, TextureWrap wrap = TextureWrap::Repeat, MipMapFiltering mipMapFiltering= MipMapFiltering::Trilinear, float anisotropicFiltering = 1.0f, bool compression = false);
        ~Texture2D();

        Texture2D(Texture2D&& other) noexcept;
        Texture2D& operator=(Texture2D&& other) noexcept;

        Texture2D(Texture2D& other) = delete;
        Texture2D& operator=(Texture2D& other) = delete;

        bool operator ==(const Texture2D& other) const {
            return id == other.id;
        }

        bool isReady() const;
        uint32_t getWidth() const;
        uint32_t getHeight() const;
        uint32_t getWidthForMip(uint32_t mip) const;
        uint32_t getHeightForMip(uint32_t mip) const;
        TextureFormat getFormat() const;
        uint32_t getRendererId() const;
        bool isCompressed() const;
        void bind(uint32_t slot) const;
        void generateMipMaps();
        void setClampBorderColor(const glm::vec4& color);
        void bufferSubData(int x, int y, int w, int h, const void* data);
        void setUnpackAlignment(int alignment);

    private:
        uint32_t id = ~0;
        uint32_t width;
        uint32_t height;
        TextureFormat format;
        bool compression;
    };

    class Texture2DArray {
    public:
        Texture2DArray() = default;
        Texture2DArray(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap, uint32_t count, MipMapFiltering mipMapFiltering = MipMapFiltering::Trilinear);
        ~Texture2DArray();

        Texture2DArray(Texture2DArray&& other) noexcept;
        Texture2DArray& operator=(Texture2DArray&& other) noexcept;

        Texture2DArray(Texture2DArray& other) = delete;
        Texture2DArray& operator=(Texture2DArray& other) = delete;

        bool operator ==(const Texture2DArray& other) const {
            return id == other.id;
        }

        bool isReady() const;
        uint32_t getWidth() const;
        uint32_t getHeight() const;
        uint32_t getCount() const;
        TextureFormat getFormat() const;
        uint32_t getRendererId() const;
        void bind(uint32_t slot) const;
        void generateMipMaps();
        void setClampBorderColor(const glm::vec4& color);

    private:
        uint32_t id = ~0;
        uint32_t width;
        uint32_t height;
        uint32_t count;
        TextureFormat format;
    };

    class TextureCube {
    public:
        static TextureCube* createResource(const std::string& name);

        TextureCube() = default;
        TextureCube(TextureFormat format, uint32_t width, uint32_t height, MipMapFiltering mipMapFiltering = MipMapFiltering::Bilinear, bool compression = false);
        TextureCube(TextureFormat format, uint32_t width, uint32_t height, const void* data, MipMapFiltering mipMapFiltering = MipMapFiltering::Bilinear, bool compression = false);
        ~TextureCube();

        TextureCube(TextureCube&& other) noexcept;
        TextureCube& operator=(TextureCube&& other) noexcept;

        TextureCube(TextureCube& other) = delete;
        TextureCube& operator=(TextureCube& other) = delete;

        bool operator ==(const TextureCube& other) const {
            return id == other.id;
        }

        bool isReady() const;
        uint32_t getWidth() const;
        uint32_t getHeight() const;
        TextureFormat getFormat() const;
        uint32_t getRendererId() const;
        bool isCompressed() const;
        void bind(uint32_t slot) const;
        void generateMipMaps();

    private:
        uint32_t id = ~0;
        uint32_t width;
        uint32_t height;
        TextureFormat format;
        bool compression;
    };

    class Texture2DView {
    public:
        Texture2DView() = default;
        Texture2DView(uint32_t originalTexture, bool originalTextureCompression, TextureFormat format, TextureWrap wrap, uint32_t minLevel, uint32_t numLevels, uint32_t minLayer, uint32_t numLayers, MipMapFiltering mipMapFiltering = MipMapFiltering::Trilinear);
        ~Texture2DView();

        Texture2DView(Texture2DView&& other) noexcept;
        Texture2DView& operator=(Texture2DView&& other) noexcept;

        Texture2DView(Texture2DView& other) = delete;
        Texture2DView& operator=(Texture2DView& other) = delete;

        bool isReady() const;
        TextureFormat getFormat() const;
        uint32_t getRendererId() const;
        void bind(uint32_t slot) const;

    private:
        uint32_t id = ~0;
        TextureFormat format;

    };

}
