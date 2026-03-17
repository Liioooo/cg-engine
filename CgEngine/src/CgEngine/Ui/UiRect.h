#pragma once

#include "UiElement.h"
#include "Rendering/Texture2D.h"
#include "Resources/ResRef.h"

namespace CgEngine {

    class UiRect : public UiElement {
    public:
        explicit UiRect() : UiElement(UIElementType::Rect) {};
        ~UiRect() override = default;

        void setWidth(float width, UIPosUnit unit);
        void setHeight(float height, UIPosUnit unit);

        void setLineWidth(float lineWidth);
        void setLineWidthHover(float lineWidth);
        void setLineColor(const glm::vec4& lineColor);
        void setLineColorHover(const glm::vec4& lineColor);
        void setFillColor(const glm::vec4& fillColor);
        void setFillColorHover(const glm::vec4& fillColor);
        void setTexture(ResRef<Texture2D> texture);
        void setTextureByName(const std::string& textureName);

        float getLineWidth() const;
        float getLineWidthHover() const;
        const glm::vec4& getLineColor() const;
        const glm::vec4& getLineColorHover() const;
        const glm::vec4& getFillColor() const;
        const glm::vec4& getFillColorHover() const;
        const ResRef<Texture2D> getTexture() const;

        const glm::vec2& getSize() const;

        const std::vector<glm::vec4>& getVertices() const override;
        uint32_t getNumIndices() const;

    protected:
        void updateElement(uint32_t canvasWidth, uint32_t canvasHeight) override;

    private:
        std::pair<float, UIPosUnit> width;
        std::pair<float, UIPosUnit> height;
        glm::vec2 size;

        float lineWidth = 0.0f;
        float lineWidthHover = 0.0f;
        glm::vec4 lineColor = {0.0f, 0.0f, 0.0f, 1.0f};
        glm::vec4 lineColorHover = {0.0f, 0.0f, 0.0f, 1.0f};
        glm::vec4 fillColor = {0.0f, 0.0f, 0.0f, 1.0f};
        glm::vec4 fillColorHover = {0.0f, 0.0f, 0.0f, 1.0f};

        bool hasLineWidthHover = false;
        bool hasLineColorHover = false;
        bool hasFillColorHover = false;

        ResRef<Texture2D> texture = nullptr;

        std::vector<glm::vec4> vertices;
        glm::vec4 collisionPoints;

        bool containsPoint(glm::vec2 point) const override;
    };

}
