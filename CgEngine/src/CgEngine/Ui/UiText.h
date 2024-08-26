#pragma once

#include "UiElement.h"
#include "Resources/Font.h"

namespace CgEngine {

    class UiText : public UiElement {
    public:
        explicit UiText() : UiElement(UIElementType::Text) {};
        ~UiText() override = default;

        void setColor(const glm::vec4& c);
        void setSize(float s, UIPosUnit unit);
        void setText(const std::string& t);
        void setFont(const std::string& name);
        void setUseKerning(bool use);

        const glm::vec4& getColor() const;
        const Texture2D* getFontAtlas() const;

        uint32_t getNumIndices() const;

        const std::vector<glm::vec4>& getVertices() const override;

    protected:
        void updateElement(bool absolutePosDirty, bool viewportDirty, uint32_t viewportWidth, uint32_t viewportHeight) override;

    private:
        glm::vec4 color;
        std::string text;

        std::pair<float, UIPosUnit> size;
        float scaledSize;

        Font* font;
        bool useKerning = true;

        bool textDirty = true;

        std::vector<glm::vec4> rawVertices;
        std::vector<glm::vec4> vertices;

        glm::vec2 bounding;

        uint32_t numIndices = 0;
    };

}
