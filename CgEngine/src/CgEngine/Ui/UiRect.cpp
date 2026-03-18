#include "UiRect.h"
#include "FileSystem.h"
#include "Application.h"
#include "CgEngineSharedUtils/UIPosUtils.h"

namespace CgEngine {
    void UiRect::setWidth(float width, UIPosUnit unit) {
        this->width = {width, unit};
        dirty = true;
    }

    void UiRect::setHeight(float height, UIPosUnit unit) {
        this->height = {height, unit};
        dirty = true;
    }

    void UiRect::setLineWidth(float lineWidth) {
        this->lineWidth = lineWidth;
        if (!hasLineWidthHover) {
            this->lineWidthHover = lineWidth;
        }
        dirty = true;
    }

    void UiRect::setLineWidthHover(float lineWidth) {
        this->lineWidthHover = lineWidth;
        hasLineWidthHover = true;
        dirty = true;
    }

    void UiRect::setLineColor(const glm::vec4& lineColor) {
        this->lineColor = lineColor;
        if (!hasLineColorHover) {
            this->lineColorHover = lineColor;
        }
        dirty = true;
    }

    void UiRect::setLineColorHover(const glm::vec4& lineColor) {
        this->lineColorHover = lineColor;
        hasLineColorHover = true;
        dirty = true;
    }

    void UiRect::setFillColor(const glm::vec4& fillColor) {
        this->fillColor = fillColor;
        if (!hasFillColorHover) {
            this->fillColorHover = fillColor;
        }
        dirty = true;
    }

    void UiRect::setFillColorHover(const glm::vec4& fillColor) {
        this->fillColorHover = fillColor;
        hasFillColorHover = true;
        dirty = true;
    }

    void UiRect::setTexture(ResRef<Texture2D> texture) {
        this->texture = texture;
        dirty = true;
    }

    void UiRect::setTextureByName(const std::string& textureName) {
        auto& resourceManager = Application::get().getResourceManager();
        std::string texturePath = FileSystem::getAsGamePath(textureName).string();

        Texture2DResourceSpecification spec{};
        spec.srgb = true;
        spec.wrap = TextureWrap::Repeat;
        spec.mipMapFiltering = MipMapFiltering::Bilinear;

        setTexture(resourceManager.getResource<Texture2D>(texturePath, spec));
    }

    float UiRect::getLineWidth() const {
        return lineWidth;
    }

    float UiRect::getLineWidthHover() const {
        return lineWidthHover;
    }

    const glm::vec4& UiRect::getLineColor() const {
        return lineColor;
    }

    const glm::vec4& UiRect::getLineColorHover() const {
        return lineColorHover;
    }

    const glm::vec4& UiRect::getFillColor() const {
        return fillColor;
    }

    const glm::vec4& UiRect::getFillColorHover() const {
        return fillColorHover;
    }

    const ResRef<Texture2D> UiRect::getTexture() const {
        return texture;
    }

    const glm::vec2& UiRect::getSize() const {
        return size;
    }

    const std::vector<glm::vec4>& UiRect::getVertices() const {
        return vertices;
    }

    uint32_t UiRect::getNumIndices() const {
        return 6;
    }

    void UiRect::updateElement(uint32_t canvasWidth, uint32_t canvasHeight) {
        vertices.clear();

       size.x = UIPosUtils::convertUIPosToPixels(width, canvasWidth, canvasHeight);
       size.y = UIPosUtils::convertUIPosToPixels(height, canvasWidth, canvasHeight);

        vertices.emplace_back(0.0f, 0.0f, 0.0f, 1.0f);
        vertices.emplace_back(size.x, 0.0f, 1.0f, 1.0f);
        vertices.emplace_back(size.x, size.y, 1.0f, 0.0f);
        vertices.emplace_back(0.0f, size.y, 0.0f, 0.0f);

        for (auto& vertex: vertices) {
            switch (xAlignment) {
                case UIXAlignment::Left:
                    vertex.x += absolutePos.x;
                    break;
                case UIXAlignment::Right:
                    vertex.x += absolutePos.x - size.x;
                    break;
                case UIXAlignment::Center:
                    vertex.x += absolutePos.x - size.x / 2;
                    break;
            }

            switch (yAlignment) {
                case UIYAlignment::Top:
                    vertex.y += absolutePos.y - size.y;
                    break;
                case UIYAlignment::Bottom:
                    vertex.y += absolutePos.y;
                    break;
                case UIYAlignment::Center:
                    vertex.y += absolutePos.y - size.y / 2;
                    break;
            }
        }

        collisionPoints[0] = vertices[0].x;
        collisionPoints[1] = canvasHeight - vertices[2].y;
        collisionPoints[2] = vertices[2].x;
        collisionPoints[3] = canvasHeight - vertices[0].y;
    }

    bool UiRect::containsPoint(glm::vec2 point) const {
        return point.x >= collisionPoints[0] && point.x <= collisionPoints[2] && point.y >= collisionPoints[1] && point.y <= collisionPoints[3];
    }
}
