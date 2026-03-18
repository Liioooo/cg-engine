#include "UiCircle.h"
#include "CgEngineSharedUtils/UIPosUtils.h"

namespace CgEngine {
    void UiCircle::setDiameter(float diameter, UIPosUnit unit) {
        this->diameter = {diameter, unit};
        dirty = true;
    }

    void UiCircle::setLineWidth(float lineWidth) {
        this->lineWidth = lineWidth;
        if (!hasLineWidthHover) {
            this->lineWidthHover = lineWidth;
        }
        dirty = true;
    }

    void UiCircle::setLineWidthHover(float lineWidth) {
        this->lineWidthHover = lineWidth;
        hasLineWidthHover = true;
        dirty = true;
    }

    void UiCircle::setLineColor(const glm::vec4& lineColor) {
        this->lineColor = lineColor;
        if (!hasLineColorHover) {
            this->lineColorHover = lineColor;
        }
        dirty = true;
    }

    void UiCircle::setLineColorHover(const glm::vec4& lineColor) {
        this->lineColorHover = lineColor;
        hasLineColorHover = true;
        dirty = true;
    }

    void UiCircle::setFillColor(const glm::vec4& fillColor) {
        this->fillColor = fillColor;
        if (!hasFillColorHover) {
            this->fillColorHover = fillColor;
        }
        dirty = true;
    }

    void UiCircle::setFillColorHover(const glm::vec4& fillColor) {
        this->fillColorHover = fillColor;
        hasFillColorHover = true;
        dirty = true;
    }

    void UiCircle::setTexture(ResRef<Texture2D> texture) {
        this->texture = texture;
        dirty = true;
    }

    float UiCircle::getPixelDiameter() const {
        return scaledDiameter;
    }

    float UiCircle::getLineWidth() const {
        return lineWidth;
    }

    float UiCircle::getLineWidthHover() const {
        return lineWidthHover;
    }

    const glm::vec4& UiCircle::getLineColor() const {
        return lineColor;
    }

    const glm::vec4& UiCircle::getLineColorHover() const {
        return lineColorHover;
    }

    const glm::vec4& UiCircle::getFillColor() const {
        return fillColor;
    }

    const glm::vec4& UiCircle::getFillColorHover() const {
        return fillColorHover;
    }

    const ResRef<Texture2D> UiCircle::getTexture() const {
        return texture;
    }

    const std::vector<glm::vec4>& UiCircle::getVertices() const {
        return vertices;
    }

    uint32_t UiCircle::getNumIndices() const {
        return 6;
    }

    void UiCircle::updateElement(uint32_t canvasWidth, uint32_t canvasHeight) {
        vertices.clear();

        scaledDiameter = UIPosUtils::convertUIPosToPixels(diameter, static_cast<float>(canvasWidth), static_cast<float>(canvasHeight));

        vertices.emplace_back(0.0f, 0.0f, 0.0f, 0.0f);
        vertices.emplace_back(scaledDiameter, 0.0f, 1.0f, 0.0f);
        vertices.emplace_back(scaledDiameter, scaledDiameter, 1.0f, 1.0f);
        vertices.emplace_back(0.0f, scaledDiameter, 0.0f, 1.0f);

        glm::vec4 alignmentOffset = glm::vec4(0.0f);
        collisionCenter = {absolutePos.x, absolutePos.y};

        switch (xAlignment) {
            case UIXAlignment::Left:
                alignmentOffset.x = 0.0f;
                collisionCenter.x += scaledDiameter / 2.0f;
                break;
            case UIXAlignment::Right:
                alignmentOffset.x = -scaledDiameter;
                collisionCenter.x -= scaledDiameter / 2.0f;
                break;
            case UIXAlignment::Center:
                alignmentOffset.x = -scaledDiameter / 2;
                break;
        }

        switch (yAlignment) {
            case UIYAlignment::Top:
                alignmentOffset.y = -scaledDiameter;
                collisionCenter.y += scaledDiameter / 2.0f;
                break;
            case UIYAlignment::Bottom:
                alignmentOffset.y = 0.0f;
                collisionCenter.y -= scaledDiameter / 2.0f;
                break;
            case UIYAlignment::Center:
                alignmentOffset.y = -scaledDiameter / 2;
                break;
        }

        for (auto& vertex: vertices) {
            vertex += glm::vec4(absolutePos, 0.0f, 0.0f) + alignmentOffset;
        }
    }

    bool UiCircle::containsPoint(glm::vec2 point) const {
        return glm::distance(collisionCenter, point) <= (scaledDiameter / 2.0f);
    }
}
