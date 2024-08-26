#pragma once

namespace CgEngine {

    enum class TextureFormat {
        R, RGB, RGBA, Float16A, Float32A, Float16, Float32, Depth
    };

    enum class TextureWrap {
        Repeat, Clamp, ClampBorder
    };

    enum class MipMapFiltering {
        Nearest, Bilinear, Trilinear
    };

    namespace TextureUtils {
        int getOpenGLTextureInternalFormat(TextureFormat format);
        int getOpenGLTextureFormatForImageBind(TextureFormat format);
        int getOpenGLTextureType(TextureFormat format);
        int getOpenGLTextureFormat(TextureFormat format);
        int getTextureWrap(TextureWrap wrap);
        void setClampBorderColor(const glm::vec4& color, unsigned int textureType);
        void applyMipMapFiltering(MipMapFiltering mipMapFiltering, unsigned int textureType);
        uint32_t calculateMipCount(uint32_t width, uint32_t height);
        std::tuple<unsigned char*, int, int> loadImageData(const std::string& path);
        void freImageData(unsigned char* data);
    }

    class Texture2D {
    public:
        static Texture2D* createResource(const std::string& name);

        Texture2D(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap, MipMapFiltering mipMapFiltering = MipMapFiltering::Trilinear, float anisotropicFiltering = 1.0f);
        Texture2D(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap, const void* data, MipMapFiltering mipMapFiltering = MipMapFiltering::Trilinear, float anisotropicFiltering = 1.0f);
        Texture2D(const std::string& path, bool srgb, TextureWrap wrap = TextureWrap::Repeat, MipMapFiltering mipMapFiltering = MipMapFiltering::Trilinear, float anisotropicFiltering = 1.0f);
        ~Texture2D();

        bool operator ==(const Texture2D& other) const {
            return id == other.id;
        }

        uint32_t getWidth() const;
        uint32_t getHeight() const;
        uint32_t getWidthForMip(uint32_t mip) const;
        uint32_t getHeightForMip(uint32_t mip) const;
        TextureFormat getFormat() const;
        uint32_t getRendererId() const;
        void bind(uint32_t slot) const;
        void generateMipMaps();
        void setClampBorderColor(const glm::vec4& color);
        void bufferSubData(int x, int y, int w, int h, const void* data);
        void setUnpackAlignment(int alignment);
        bool isLoaded() const;

    private:
        uint32_t id;
        uint32_t width;
        uint32_t height;
        TextureFormat format;
        bool loaded = false;
    };

    class Texture2DArray {
    public:
        Texture2DArray(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap, uint32_t count, MipMapFiltering mipMapFiltering = MipMapFiltering::Trilinear);
        ~Texture2DArray();

        bool operator ==(const Texture2DArray& other) const {
            return id == other.id;
        }

        uint32_t getWidth() const;
        uint32_t getHeight() const;
        uint32_t getCount() const;
        TextureFormat getFormat() const;
        uint32_t getRendererId() const;
        void bind(uint32_t slot) const;
        void generateMipMaps();
        void setClampBorderColor(const glm::vec4& color);

    private:
        uint32_t id;
        uint32_t width;
        uint32_t height;
        uint32_t count;
        TextureFormat format;
    };

    class TextureCube {
    public:
        static TextureCube* createResource(const std::string& name);

        TextureCube(TextureFormat format, uint32_t width, uint32_t height, MipMapFiltering mipMapFiltering = MipMapFiltering::Bilinear);
        TextureCube(TextureFormat format, uint32_t width, uint32_t height, const void* data, MipMapFiltering mipMapFiltering = MipMapFiltering::Bilinear);
        ~TextureCube();

        bool operator ==(const TextureCube& other) const {
            return id == other.id;
        }

        uint32_t getWidth() const;
        uint32_t getHeight() const;
        TextureFormat getFormat() const;
        uint32_t getRendererId() const;
        void bind(uint32_t slot) const;
        void generateMipMaps();

    private:
        uint32_t id;
        uint32_t width;
        uint32_t height;
        TextureFormat format;
    };

}
