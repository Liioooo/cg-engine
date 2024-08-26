#include "UiText.h"
#include "GlobalObjectManager.h"

namespace CgEngine {
    void UiText::setColor(const glm::vec4& c) {
        color = c;
    }

    void UiText::setSize(float s, UIPosUnit unit) {
        size = {s, unit};
        textDirty = true;
    }

    void UiText::setText(const std::string& t) {
        text = t;
        textDirty = true;
    }

    void UiText::setFont(const std::string& name) {
        font = GlobalObjectManager::getInstance().getResourceManager().getResource<Font>(name);
        textDirty = true;
    }

    void UiText::setUseKerning(bool use) {
        useKerning = use;
    }

    const glm::vec4& UiText::getColor() const {
        return color;
    }

    const Texture2D* UiText::getFontAtlas() const {
        return font->getFontAtlas();
    }

    uint32_t UiText::getNumIndices() const {
        return numIndices;
    }

    const std::vector<glm::vec4>& UiText::getVertices() const {
        return vertices;
    }

    void UiText::updateElement(bool absolutePosDirty, bool viewportDirty, uint32_t viewportWidth, uint32_t viewportHeight) {
        if (textDirty || viewportDirty) {
            rawVertices.clear();
            numIndices = 0;
            bounding = glm::vec2(0.0f);

            switch (size.second) {
                case UIPosUnit::Pixel:
                    scaledSize = size.first;
                    break;
                case UIPosUnit::VWPercent:
                    scaledSize = size.first * static_cast<float>(viewportWidth);
                    break;
                case UIPosUnit::VHPercent:
                    scaledSize = size.first * static_cast<float>(viewportHeight);
                    break;
            }

            float penX = 0.0f;
            float penY = 0.0f;

            float atlasWidth = static_cast<float>(font->getFontAtlas()->getWidth());
            float atlasHeight = static_cast<float>(font->getFontAtlas()->getHeight());

            float fontScaledSize = scaledSize / Font::fontPixelSize;

            uint32_t lastGlyphIndex = 0;

            for (const char& c: text) {
                auto& cInfo = font->getCharacterInfo(c);

                float kerning = 0.0f;

                if (useKerning && lastGlyphIndex != 0) {
                    kerning = font->getKerning(lastGlyphIndex, cInfo.glyphIndex);
                }

                float x =  penX + (kerning + cInfo.bitmapLeft) * fontScaledSize;
                float y =  penY - (cInfo.bitmapHeight - cInfo.bitmapTop) * fontScaledSize;
                float w = cInfo.bitmapWidth * fontScaledSize;
                float h = cInfo.bitmapHeight * fontScaledSize;

                if (w > 0 && h > 0) {
                    rawVertices.emplace_back(x, y, cInfo.textureCoord, cInfo.bitmapHeight / atlasHeight);
                    rawVertices.emplace_back(x + w, y, cInfo.textureCoord + cInfo.bitmapWidth / atlasWidth, cInfo.bitmapHeight / atlasHeight);
                    rawVertices.emplace_back(x + w, y + h, cInfo.textureCoord + cInfo.bitmapWidth / atlasWidth, 0.0f);
                    rawVertices.emplace_back(x, y + h, cInfo.textureCoord, 0.0f);

                    numIndices += 6;
                }

                bounding.y = glm::max(bounding.y, y + h);
                bounding.x = glm::max(bounding.x, x + w);

                penX += (cInfo.advanceX + kerning) * fontScaledSize;
                penY += cInfo.advanceY * fontScaledSize;

                lastGlyphIndex = cInfo.glyphIndex;
            }
        }

        if (textDirty || absolutePosDirty || viewportDirty) {
            vertices.clear();

            auto alignmentOffset = glm::vec2(0.0f);

            switch (xAlignment) {
                case UIXAlignment::Left:
                    alignmentOffset.x = 0.0f;
                    break;
                case UIXAlignment::Right:
                    alignmentOffset.x = -bounding.x;
                    break;
                case UIXAlignment::Center:
                    alignmentOffset.x = -bounding.x / 2;
                    break;
            }

            switch (yAlignment) {
                case UIYAlignment::Top:
                    alignmentOffset.y = -bounding.y;
                    break;
                case UIYAlignment::Bottom:
                    alignmentOffset.y = 0.0f;
                    break;
                case UIYAlignment::Center:
                    alignmentOffset.y = -bounding.y / 2;
                    break;
            }

            auto offset = absolutePos + alignmentOffset;
            for (const auto& item: rawVertices) {
                vertices.push_back(item + glm::vec4(offset, 0.0f, 0.0f));
            }
        }

        textDirty = false;
    }
}