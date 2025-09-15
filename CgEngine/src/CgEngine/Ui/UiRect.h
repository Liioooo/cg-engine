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
        void setLineColor(const glm::vec4& lineColor);
        void setFillColor(const glm::vec4& fillColor);
        void setTexture(ResRef<Texture2D> texture);
        void setTextureByName(const std::string& textureName);

        float getLineWidth() const;
        const glm::vec4& getLineColor() const;
        const glm::vec4& getFillColor() const;
        const ResRef<Texture2D> getTexture() const;

        const glm::vec2& getSize() const;

        const std::vector<glm::vec4>& getVertices() const override;

        bool isPointInside(const glm::vec2& point) const;

    protected:
        void updateElement(bool absolutePosDirty, bool viewportDirty, uint32_t viewportWidth, uint32_t viewportHeight) override;

    private:
        std::pair<float, UIPosUnit> width;
        std::pair<float, UIPosUnit> height;
        glm::vec2 size;

        float lineWidth{};
        glm::vec4 lineColor{};
        glm::vec4 fillColor{};

        ResRef<Texture2D> texture = nullptr;

        bool dirty = true;

        std::vector<glm::vec4> vertices;
        glm::vec4 collisionPoints;
    };

}
