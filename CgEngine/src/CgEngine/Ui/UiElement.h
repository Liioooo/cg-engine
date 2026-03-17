#pragma once

#include "CgEngineSharedUtils/Enums.h"
#include "CallbackConnectionManager.h"

namespace CgEngine {

    class UiElement {
        friend class UiCanvas;
    public:
        explicit UiElement(UIElementType type);
        virtual ~UiElement() = default;

        inline UIElementType getType() const {
            return type;
        }

        void setTop(float top, UIPosUnit unit);
        void setRight(float right, UIPosUnit unit);
        void setBottom(float bottom, UIPosUnit unit);
        void setLeft(float left, UIPosUnit unit);

        void setXAlignment(UIXAlignment xAlignment);
        void setYAlignment(UIYAlignment yAlignment);

        uint32_t getZIndex() const;
        void setZIndex(uint32_t zIndex);

        CallbackConnection<std::function<void()>> addClickListener(const std::function<void()>& listener);
        void receiveClickEvent() const;

        virtual const std::vector<glm::vec4>& getVertices() const = 0;
        virtual uint32_t getNumIndices() const = 0;

    protected:
        virtual void updateElement(uint32_t canvasWidth, uint32_t canvasHeight) = 0;

        UIXAlignment xAlignment;
        UIYAlignment yAlignment;
        glm::vec2 absolutePos = glm::vec2(0.0f, 0.0f);
        bool dirty = true;

    private:
        const UIElementType type;

        std::pair<float, UIPosUnit> top = {-1.0f, UIPosUnit::Pixel};
        std::pair<float, UIPosUnit> right = {-1.0f, UIPosUnit::Pixel};
        std::pair<float, UIPosUnit> bottom = {-1.0f, UIPosUnit::Pixel};
        std::pair<float, UIPosUnit> left = {-1.0f, UIPosUnit::Pixel};

        uint32_t zIndex = 0;

        CallbackConnectionManager<std::function<void()>> callbackConnectionManager;

        bool update(uint32_t canvasWidth, uint32_t canvasHeight, bool canvasSizeDirty);
        virtual bool containsPoint(glm::vec2 point) const = 0;
    };

}
