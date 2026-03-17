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

    CallbackConnection<std::function<void()>> UiElement::addClickListener(const std::function<void()>& listener) {
        return callbackConnectionManager.addCallback(listener);
    }

    void UiElement::receiveClickEvent() const {
        callbackConnectionManager.callCallbacks();
    }

    bool UiElement::update(uint32_t canvasWidth, uint32_t canvasHeight, bool canvasSizeDirty) {
        bool updated = dirty || canvasSizeDirty;

        if (dirty || canvasSizeDirty) {

            absolutePos = {0.0f, 0.0f};

            if (left.first >= 0) {
                switch (left.second) {
                    case UIPosUnit::Pixel:
                        absolutePos.x = left.first;
                        break;
                    case UIPosUnit::VWPercent:
                        absolutePos.x = static_cast<float>(canvasWidth) * left.first;
                        break;
                    case UIPosUnit::VHPercent:
                        absolutePos.x = static_cast<float>(canvasHeight) * left.first;
                        break;
                }
            } else if (right.first >= 0) {
                switch (right.second) {
                    case UIPosUnit::Pixel:
                        absolutePos.x = static_cast<float>(canvasWidth) - right.first;
                        break;
                    case UIPosUnit::VWPercent:
                        absolutePos.x = static_cast<float>(canvasWidth) - static_cast<float>(canvasWidth) * right.first;
                        break;
                    case UIPosUnit::VHPercent:
                        absolutePos.x = static_cast<float>(canvasWidth) - static_cast<float>(canvasHeight) * right.first;
                        break;
                }
            }

            if (top.first >= 0) {
                switch (top.second) {
                    case UIPosUnit::Pixel:
                        absolutePos.y = static_cast<float>(canvasHeight) - top.first;
                        break;
                    case UIPosUnit::VWPercent:
                        absolutePos.y = static_cast<float>(canvasHeight) - static_cast<float>(canvasWidth) * top.first;
                        break;
                    case UIPosUnit::VHPercent:
                        absolutePos.y = static_cast<float>(canvasHeight) - static_cast<float>(canvasHeight) * top.first;
                        break;
                }
            } else if (bottom.first >= 0) {
                switch (bottom.second) {
                    case UIPosUnit::Pixel:
                        absolutePos.y = bottom.first;
                        break;
                    case UIPosUnit::VWPercent:
                        absolutePos.y = static_cast<float>(canvasWidth) * bottom.first;
                        break;
                    case UIPosUnit::VHPercent:
                        absolutePos.y = static_cast<float>(canvasHeight) * bottom.first;
                        break;
                }
            }

            updateElement(canvasWidth, canvasHeight);
        }

        dirty = false;

        return updated;
    }
}
