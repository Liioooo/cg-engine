#include "UiText.h"
#include "Application.h"

namespace CgEngine {
    void UiText::setColor(const glm::vec4& c) {
        color = c;
    }

    void UiText::setSize(float s, UIPosUnit unit) {
        size = {s, unit};
        dirty = true;
    }

    void UiText::setText(const std::string& t) {
        text = t;
        dirty = true;
    }

    void UiText::setFont(const std::string& name) {
        font = Application::get().getResourceManager().getResource<Font>(name);
        dirty = true;
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

    const std::vector<glm::vec4>& UiText::getVertices() const {
        return vertices;
    }

    uint32_t UiText::getNumIndices() const {
        return numIndices;
    }

    void UiText::updateElement(uint32_t canvasWidth, uint32_t canvasHeight) {
        vertices.clear();
        numIndices = 0;

        glm::vec2 bounding = glm::vec2(0.0f);
        float scaledSize = 0.0f;

        switch (size.second) {
            case UIPosUnit::Pixel:
                scaledSize = size.first;
                break;
            case UIPosUnit::VWPercent:
                scaledSize = size.first * static_cast<float>(canvasWidth);
                break;
            case UIPosUnit::VHPercent:
                scaledSize = size.first * static_cast<float>(canvasHeight);
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

            float x = penX + (kerning + cInfo.bitmapLeft) * fontScaledSize;
            float y = penY - (cInfo.bitmapHeight - cInfo.bitmapTop) * fontScaledSize;
            float w = cInfo.bitmapWidth * fontScaledSize;
            float h = cInfo.bitmapHeight * fontScaledSize;

            if (w > 0 && h > 0) {
                vertices.emplace_back(x, y, cInfo.textureCoord, cInfo.bitmapHeight / atlasHeight);
                vertices.emplace_back(x + w, y, cInfo.textureCoord + cInfo.bitmapWidth / atlasWidth, cInfo.bitmapHeight / atlasHeight);
                vertices.emplace_back(x + w, y + h, cInfo.textureCoord + cInfo.bitmapWidth / atlasWidth, 0.0f);
                vertices.emplace_back(x, y + h, cInfo.textureCoord, 0.0f);

                numIndices += 6;
            }

            bounding.y = glm::max(bounding.y, y + h);
            bounding.x = glm::max(bounding.x, x + w);

            penX += (cInfo.advanceX + kerning) * fontScaledSize;
            penY += cInfo.advanceY * fontScaledSize;

            lastGlyphIndex = cInfo.glyphIndex;
        }

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
        for (auto& item: vertices) {
            item += glm::vec4(offset, 0.0f, 0.0f);
        }
    }

    bool UiText::containsPoint(glm::vec2 point) const {
        return false;
    }
}
