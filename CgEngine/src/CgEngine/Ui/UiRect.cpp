#include "UiRect.h"
#include "GlobalObjectManager.h"
#include "FileSystem.h"

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
    }

    void UiRect::setLineColor(const glm::vec4& lineColor) {
        this->lineColor = lineColor;
    }

    void UiRect::setFillColor(const glm::vec4& fillColor) {
        this->fillColor = fillColor;
    }

    void UiRect::setTexture(CgEngine::Texture2D* texture) {
        this->texture = texture;
    }

    void UiRect::setTexture(const std::string& textureName) {
        auto& resourceManager = GlobalObjectManager::getInstance().getResourceManager();
        std::string texturePath = FileSystem::getAsGamePath(textureName);

        if (resourceManager.hasResource<Texture2D>(texturePath)) {
            setTexture(resourceManager.getResource<Texture2D>(texturePath));
        } else {
            auto* tex = new Texture2D(texturePath, false, TextureWrap::Repeat, MipMapFiltering::Bilinear);
            resourceManager.insertResource(texturePath, tex);
            setTexture(tex);
        }
    }

    float UiRect::getLineWidth() const {
        return lineWidth;
    }

    const glm::vec4& UiRect::getLineColor() const {
        return lineColor;
    }

    const glm::vec4& UiRect::getFillColor() const {
        return fillColor;
    }

    const Texture2D* UiRect::getTexture() const {
        return texture;
    }

    const glm::vec2& UiRect::getSize() const {
        return size;
    }

    const std::vector<glm::vec4>& UiRect::getVertices() const {
        return vertices;
    }

    bool UiRect::isPointInside(const glm::vec2& point) const {
        return point.x >= collisionPoints[0] && point.x <= collisionPoints[2] && point.y >= collisionPoints[1] && point.y <= collisionPoints[3];
    }

    void UiRect::updateElement(bool absolutePosDirty, bool viewportDirty, uint32_t viewportWidth, uint32_t viewportHeight) {
        if (dirty || absolutePosDirty || viewportDirty) {
            vertices.clear();

            switch (width.second) {
                case UIPosUnit::Pixel:
                    size.x = width.first;
                    break;
                case UIPosUnit::VWPercent:
                    size.x = width.first * static_cast<float>(viewportWidth);
                    break;
                case UIPosUnit::VHPercent:
                    size.x = width.first * static_cast<float>(viewportHeight);
                    break;
            }

            switch (height.second) {
                case UIPosUnit::Pixel:
                    size.y = height.first;
                    break;
                case UIPosUnit::VWPercent:
                    size.y = height.first * static_cast<float>(viewportWidth);
                    break;
                case UIPosUnit::VHPercent:
                    size.y = height.first * static_cast<float>(viewportHeight);
                    break;
            }

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
        }

        collisionPoints[0] = vertices[0].x;
        collisionPoints[1] = viewportHeight - vertices[2].y;
        collisionPoints[2] = vertices[2].x;
        collisionPoints[3] = viewportHeight - vertices[0].y;


        dirty = false;
    }
}