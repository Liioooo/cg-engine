#include "Font.h"
#include "Asserts.h"
#include "FileSystem.h"
#include "Rendering/GraphicsObjectsFactory.h"

namespace CgEngine {
    Font* Font::createResource(const std::string& name) {
        return new Font(name);
    }

    Font::Font(const std::string& name) {
        auto fontPath = FileSystem::getAsGamePath(name);

        CG_ASSERT(FileSystem::checkFileExists(fontPath), "Font Path " + fontPath.string() + " does not exist")

        auto error = FT_New_Face(getFTLibrary(), fontPath.string().c_str(), 0, &ftFace);
        if (error) {
            CG_LOGGING_ERROR(FT_Error_String(error));
        }
        CG_ASSERT(!error, "Could not open font")

        FT_Set_Pixel_Sizes(ftFace, 0, fontPixelSize);

        FT_GlyphSlot glyphSlot = ftFace->glyph;

        uint32_t atlasWidth = 0;
        uint32_t atlasHeight = 0;

        for (int i = 32; i < 128; i++) {
            if(FT_Load_Char(ftFace, i, FT_LOAD_RENDER)) {
                CG_LOGGING_WARNING("Loading character {0} failed!", i);
                continue;
            }

            atlasWidth += glyphSlot->bitmap.width + 2;
            atlasHeight = glm::max(atlasHeight, glyphSlot->bitmap.rows);
        }

        fontAtlas = GraphicsObjectsFactory::createTexture2D(TextureFormat::R, atlasWidth, atlasHeight, TextureWrap::Clamp, MipMapFiltering::Bilinear);

        int currentAtlasOffset = 0;

        for (int i = 32; i < 128; i++) {
            if(FT_Load_Char(ftFace, i, FT_LOAD_RENDER)) {
                continue;
            }

            fontCharacterInfos[i].advanceX = static_cast<float>(glyphSlot->advance.x >> 6);
            fontCharacterInfos[i].advanceY = static_cast<float>(glyphSlot->advance.y >> 6);
            fontCharacterInfos[i].bitmapWidth = static_cast<float>(glyphSlot->bitmap.width);
            fontCharacterInfos[i].bitmapHeight = static_cast<float>(glyphSlot->bitmap.rows);
            fontCharacterInfos[i].bitmapLeft = static_cast<float>(glyphSlot->bitmap_left);
            fontCharacterInfos[i].bitmapTop = static_cast<float>(glyphSlot->bitmap_top);
            fontCharacterInfos[i].textureCoord = static_cast<float>(currentAtlasOffset) / static_cast<float>(atlasWidth);
            fontCharacterInfos[i].glyphIndex = FT_Get_Char_Index(ftFace, i);

            fontAtlas->bufferSubData(currentAtlasOffset, 0, glyphSlot->bitmap.width, glyphSlot->bitmap.rows, glyphSlot->bitmap.buffer, 1);

            currentAtlasOffset += static_cast<int>(glyphSlot->bitmap.width) + 2;
        }
    }

    Font::~Font() {
        delete fontAtlas;
        FT_Done_Face(ftFace);
    }

    float Font::getKerning(uint32_t leftGlyph, uint32_t rightGlyph) const {
        FT_Vector delta;
        FT_Get_Kerning(ftFace, leftGlyph, rightGlyph, FT_KERNING_DEFAULT, &delta);
        return static_cast<float>(delta.x >> 6);
    }

    const FontCharacterInfo& Font::getCharacterInfo(char c) const {
        return fontCharacterInfos[c];
    }

    const Texture2D* Font::getFontAtlas() const {
        return fontAtlas;
    }

    FT_Library Font::getFTLibrary() {
        if (ftLibrary != nullptr) {
            return ftLibrary;
        }

        if(FT_Init_FreeType(&ftLibrary)) {
            CG_LOGGING_ERROR("Could not init freetype library");
        }

        return ftLibrary;
    }
}
