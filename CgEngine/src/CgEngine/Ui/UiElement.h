#pragma once

namespace CgEngine {

    enum class UIElementType {
        Circle, Rect, Text
    };

    enum class UIPosUnit {
        Pixel, VWPercent, VHPercent
    };

    enum class UIXAlignment {
        Left, Center, Right
    };

    enum class UIYAlignment {
        Top, Center, Bottom
    };

    class UiElement {
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

        void update(uint32_t viewportWidth, uint32_t viewportHeight, bool viewportDirty);

        virtual const std::vector<glm::vec4>& getVertices() const = 0;

    protected:
        virtual void updateElement(bool absolutePosDirty, bool viewportDirty, uint32_t viewportWidth, uint32_t viewportHeight) = 0;

        UIXAlignment xAlignment;
        UIYAlignment yAlignment;
        glm::vec2 absolutePos = glm::vec2(0.0f, 0.0f);

    private:
        const UIElementType type;

        std::pair<float, UIPosUnit> top = {-1, UIPosUnit::Pixel};
        std::pair<float, UIPosUnit> right = {-1, UIPosUnit::Pixel};
        std::pair<float, UIPosUnit> bottom = {-1, UIPosUnit::Pixel};
        std::pair<float, UIPosUnit> left = {-1, UIPosUnit::Pixel};

        uint32_t zIndex = 0;
        bool dirty = true;
    };

}
