#pragma once

#include "UiElement.h"
#include "Rendering/Texture.h"

namespace CgEngine {

    class UiCircle : public UiElement {
    public:
        explicit UiCircle() : UiElement(UIElementType::Circle) {};
        ~UiCircle() override = default;

        void setWidth(float width, UIPosUnit unit);
        float getWidth() const;

        void setLineWidth(float lineWidth);
        void setLineColor(const glm::vec4& lineColor);
        void setFillColor(const glm::vec4& fillColor);
        void setTexture(Texture2D* texture);

        float getLineWidth() const;
        const glm::vec4& getLineColor() const;
        const glm::vec4& getFillColor() const;
        const Texture2D* getTexture() const;

        const std::vector<glm::vec4>& getVertices() const override;

    protected:
        void updateElement(bool absolutePosDirty, bool viewportDirty, uint32_t viewportWidth, uint32_t viewportHeight) override;

    private:
        std::pair<float, UIPosUnit> width;
        float scaledWidth;

        float lineWidth{};
        glm::vec4 lineColor{};
        glm::vec4 fillColor{};

        Texture2D* texture = nullptr;

        bool dirty = true;

        std::vector<glm::vec4> vertices;
    };

}