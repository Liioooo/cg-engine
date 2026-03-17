#pragma once

#include "Component.h"
#include "Ui/UiCanvas.h"
#include "Events/Event.h"

namespace CgEngine {

    struct UiCanvasComponent2DParams {
        std::string canvasName;
        std::pair<float, UIPosUnit> posX = {0.5f, UIPosUnit::VWPercent};
        std::pair<float, UIPosUnit> posY = {0.5f, UIPosUnit::VHPercent};
        std::pair<float, UIPosUnit> width = {1.0f, UIPosUnit::VWPercent};
        std::pair<float, UIPosUnit> height = {1.0f, UIPosUnit::VHPercent};
        UIXAlignment xAlignment = UIXAlignment::Center;
        UIYAlignment yAlignment = UIYAlignment::Center;
        bool receiveInputEvents = true;
        uint32_t zIndex = 0;

        void verifyParams() const;
    };

    class UiCanvasComponent2D : public Component {
    public:
        using Component::Component;
        using Params = UiCanvasComponent2DParams;

        void onAttach(Scene& scene, UiCanvasComponent2DParams& params);
        void onDetach(Scene& scene) override;
        void onRenderImGui() override;

        void setPositionX(float posX, UIPosUnit unit);
        void setPositionY(float posY, UIPosUnit unit);

        void setWidth(float width, UIPosUnit unit);
        void setHeight(float height, UIPosUnit unit);

        void setXAlignment(UIXAlignment xAlignment);
        void setYAlignment(UIYAlignment yAlignment);

        void setReceiveInputEvents(bool receiveInputEvents);

        void setZIndex(uint32_t zIndex);
        uint32_t getZIndex() const;

        void update(uint32_t viewportWidth, uint32_t viewportHeight);
        UiCanvas* getCanvas() const;
        glm::mat4 getFinalTransform() const;

        void onEvent(Event& event, uint32_t viewportWidth, uint32_t viewportHeight);

    private:
        UiCanvas* canvas = nullptr;

        std::pair<float, UIPosUnit> posX;
        std::pair<float, UIPosUnit> posY;
        std::pair<float, UIPosUnit> width;
        std::pair<float, UIPosUnit> height;
        UIXAlignment xAlignment;
        UIYAlignment yAlignment;

        bool receiveInputEvents = true;
        uint32_t zIndex = 0;

        glm::mat4 finalTransform = glm::mat4(1.0f);
        glm::mat4 finalTransformInverse = glm::mat4(1.0f);
    };

}
