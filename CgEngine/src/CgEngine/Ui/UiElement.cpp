#include "UiElement.h"
#include "Logging.h"

namespace CgEngine {
    UiElement::UiElement(UIElementType type) : type(type) {}

    void UiElement::setTop(float top, UIPosUnit unit) {
        this->top = {top, unit};
        dirty = true;
    }

    void UiElement::setRight(float right, UIPosUnit unit) {
        this->right = {right, unit};
        dirty = true;
    }

    void UiElement::setBottom(float bottom, UIPosUnit unit) {
        this->bottom = {bottom, unit};
        dirty = true;
    }

    void UiElement::setLeft(float left, UIPosUnit unit) {
        this->left = {left, unit};
        dirty = true;
    }

    void UiElement::setXAlignment(UIXAlignment xAlignment) {
        this->xAlignment = xAlignment;
    }

    void UiElement::setYAlignment(UIYAlignment yAlignment) {
        this->yAlignment = yAlignment;
    }

    uint32_t UiElement::getZIndex() const {
        return zIndex;
    }

    void UiElement::setZIndex(uint32_t zIndex) {
        this->zIndex = zIndex;
    }

    void UiElement::update(uint32_t viewportWidth, uint32_t viewportHeight, bool viewportDirty) {
        if (dirty || viewportDirty) {

            absolutePos = {0.0f, 0.0f};

            if (left.first >= 0) {
                switch (left.second) {
                    case UIPosUnit::Pixel:
                        absolutePos.x = left.first;
                        break;
                    case UIPosUnit::VWPercent:
                        absolutePos.x = static_cast<float>(viewportWidth) * left.first;
                        break;
                    case UIPosUnit::VHPercent:
                        absolutePos.x = static_cast<float>(viewportHeight) * left.first;
                        break;
                }
            } else if (right.first >= 0) {
                switch (right.second) {
                    case UIPosUnit::Pixel:
                        absolutePos.x = static_cast<float>(viewportWidth) - right.first;
                        break;
                    case UIPosUnit::VWPercent:
                        absolutePos.x = static_cast<float>(viewportWidth) - static_cast<float>(viewportWidth) * right.first;
                        break;
                    case UIPosUnit::VHPercent:
                        absolutePos.x = static_cast<float>(viewportWidth) - static_cast<float>(viewportHeight) * right.first;
                        break;
                }
            }

            if (top.first >= 0) {
                switch (top.second) {
                    case UIPosUnit::Pixel:
                        absolutePos.y = static_cast<float>(viewportHeight) - top.first;
                        break;
                    case UIPosUnit::VWPercent:
                        absolutePos.y = static_cast<float>(viewportHeight) - static_cast<float>(viewportWidth) * top.first;
                        break;
                    case UIPosUnit::VHPercent:
                        absolutePos.y = static_cast<float>(viewportHeight) - static_cast<float>(viewportHeight) * top.first;
                        break;
                }
            } else if (bottom.first >= 0) {
                switch (bottom.second) {
                    case UIPosUnit::Pixel:
                        absolutePos.y = bottom.first;
                        break;
                    case UIPosUnit::VWPercent:
                        absolutePos.y = static_cast<float>(viewportWidth) * bottom.first;
                        break;
                    case UIPosUnit::VHPercent:
                        absolutePos.y = static_cast<float>(viewportHeight) * bottom.first;
                        break;
                }
            }
        }

        updateElement(dirty, viewportDirty, viewportWidth, viewportHeight);

        dirty = false;
    }
}