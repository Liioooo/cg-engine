#pragma once

#include "UiElement.h"
#include "Resources/Font.h"
#include "Resources/ResRef.h"

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

        const std::vector<glm::vec4>& getVertices() const override;
        uint32_t getNumIndices() const override;

    protected:
        void updateElement(uint32_t canvasWidth, uint32_t canvasHeight) override;

    private:
        glm::vec4 color{};
        std::string text;

        std::pair<float, UIPosUnit> size;

        ResRef<Font> font;
        bool useKerning = true;

        std::vector<glm::vec4> vertices;

        uint32_t numIndices = 0;

        bool containsPoint(glm::vec2 point) const override;
    };

}
