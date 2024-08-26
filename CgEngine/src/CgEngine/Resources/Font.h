#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include "Rendering/Texture.h"

namespace CgEngine {

    struct FontCharacterInfo {
        float advanceX;
        float advanceY;
        float bitmapWidth;
        float bitmapHeight;
        float bitmapLeft;
        float bitmapTop;
        unsigned int glyphIndex;
        float textureCoord;
    };

    class Font {
    public:
        static Font* createResource(const std::string& name);

        static const uint32_t fontPixelSize = 256;

        Font(const std::string& name);
        ~Font();

        float getKerning(uint32_t leftGlyph, uint32_t rightGlyph) const;
        const FontCharacterInfo& getCharacterInfo(char c) const;
        const Texture2D* getFontAtlas() const;

    private:
        static FT_Library ftLibrary;
        static FT_Library getFTLibrary();

        FT_Face ftFace;
        std::array<FontCharacterInfo, 128> fontCharacterInfos;
        Texture2D* fontAtlas;
    };

}
