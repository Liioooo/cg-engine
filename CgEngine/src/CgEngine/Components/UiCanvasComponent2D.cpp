#include <Asserts.h>
#include "UiCanvasComponent2D.h"
#include "imgui.h"
#include "Ui/UiCanvasFactory.h"
#include "CgEngineSharedUtils/UIPosUtils.h"

namespace CgEngine {
    void UiCanvasComponent2DParams::verifyParams() const {
        CG_ASSERT(!canvasName.empty(), "UiCanvasComponent2DParams: canvas is empty.")
    }

    void UiCanvasComponent2D::onAttach(Scene& scene, UiCanvasComponent2DParams& params) {
        canvas = UiCanvasFactory::createUiCanvas(params.canvasName);

        posX = params.posX;
        posY = params.posY;
        width = params.width;
        height = params.height;
        xAlignment = params.xAlignment;
        yAlignment = params.yAlignment;
        receiveInputEvents = params.receiveInputEvents;
        zIndex = params.zIndex;
    }

    void UiCanvasComponent2D::onDetach(Scene& scene) {
        delete canvas;
    }

    void UiCanvasComponent2D::onRenderImGui() {
        if (ImGui::CollapsingHeader("UiCanvasComponent")) {
        }
    }

    void UiCanvasComponent2D::setPositionX(float posX, UIPosUnit unit) {
        this->posX = {posX, unit};
    }

    void UiCanvasComponent2D::setPositionY(float posY, UIPosUnit unit) {
        this->posY = {posY, unit};
    }

    void UiCanvasComponent2D::setWidth(float width, UIPosUnit unit) {
        this->width = {width, unit};
    }

    void UiCanvasComponent2D::setHeight(float height, UIPosUnit unit) {
        this->height = {height, unit};
    }

    void UiCanvasComponent2D::setXAlignment(UIXAlignment xAlignment) {
        this->xAlignment = xAlignment;
    }

    void UiCanvasComponent2D::setYAlignment(UIYAlignment yAlignment) {
        this->yAlignment = yAlignment;
    }

    void UiCanvasComponent2D::setReceiveInputEvents(bool receiveInputEvents) {
        this->receiveInputEvents = receiveInputEvents;
    }

    uint32_t UiCanvasComponent2D::getZIndex() const {
        return zIndex;
    }

    void UiCanvasComponent2D::setZIndex(uint32_t zIndex) {
        this->zIndex = zIndex;
    }

    void UiCanvasComponent2D::update(uint32_t viewportWidth, uint32_t viewportHeight) {
        canvas->update(viewportWidth, viewportHeight);

        auto viewportWidthF = static_cast<float>(viewportWidth);
        auto viewportHeightF = static_cast<float>(viewportHeight);

        uint32_t pixelWidth = UIPosUtils::convertUIPosToPixels(width, viewportWidthF, viewportHeightF);
        uint32_t pixelHeight = UIPosUtils::convertUIPosToPixels(height, viewportWidthF, viewportHeightF);
        uint32_t pixelPosX = UIPosUtils::convertUIPosToPixels(posX, viewportWidthF, viewportHeightF);
        uint32_t pixelPosY = viewportHeight - UIPosUtils::convertUIPosToPixels(posY, viewportWidthF, viewportHeightF);

        switch (xAlignment) {
            case UIXAlignment::Left:
                pixelPosX += pixelWidth / 2;
                break;
            case UIXAlignment::Center:
                break;
            case UIXAlignment::Right:
                pixelPosX -= pixelWidth / 2;
                break;
        }

        switch (yAlignment) {
            case UIYAlignment::Top:
                pixelPosY -= pixelHeight / 2;
                break;
            case UIYAlignment::Center:
                break;
            case UIYAlignment::Bottom:
                pixelPosY += pixelHeight / 2;
                break;
        }

        finalTransform = glm::mat4(1.0f);
        finalTransform = glm::translate(finalTransform, {pixelPosX, pixelPosY, 0.0f});
        finalTransform = glm::scale(finalTransform, {static_cast<float>(pixelWidth) / 2.0f, static_cast<float>(pixelHeight) / 2.0f, 1.0f});
        finalTransformInverse = glm::inverse(finalTransform);
    }

    UiCanvas* UiCanvasComponent2D::getCanvas() const {
        return canvas;
    }

    glm::mat4 UiCanvasComponent2D::getFinalTransform() const {
        return finalTransform;
    }

    void UiCanvasComponent2D::onEvent(Event& event, uint32_t viewportWidth, uint32_t viewportHeight) {
        if (!receiveInputEvents || !event.hasMousePosition()) {
            return;
        }
        auto* mousePosEvent = dynamic_cast<IEventHasMousePosition*>(&event);
        glm::vec4 mousePos(mousePosEvent->getXPos(), viewportHeight - mousePosEvent->getYPos(), 0.0f, 1.0f);

        glm::vec4 localMousePos4 = finalTransformInverse * mousePos;
        glm::vec2 localMousePos(localMousePos4.x, -localMousePos4.y);

        if (localMousePos.x < -1.0f || localMousePos.x > 1.0f || localMousePos.y < -1.0f || localMousePos.y > 1.0f) {
            canvas->clearHoverState();
            return;
        }

        canvas->onEvent(event, localMousePos);
    }

}
