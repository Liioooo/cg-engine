#include "FPSCounter.h"

namespace Game {
    void FPSCounter::onEnable() {
        canvasComponent = getOwingEntity().getComponent<CgEngine::UiCanvasComponent2D>();
    }

    void FPSCounter::update(CgEngine::TimeStep ts) {
        if (showing) {
            frameTimes[currentIndex] = ts.getSeconds();
            currentIndex = (currentIndex + 1) % frameTimes.size();

            float sum = 0.0f;
            for (const auto& ft: frameTimes) {
                sum += ft;
            }
            sum /= static_cast<float>(frameTimes.size());

            fps->setText(std::to_string(static_cast<uint32_t>(1.0f / sum)));
        }
    }

    void FPSCounter::onKeyPressed(CgEngine::KeyPressedEvent& event) {
        if (event.getKeyCode() == CgEngine::KeyCode::F4) {
            showing = !showing;
            if (showing) {
                createUI();
            } else {
                canvasComponent->getCanvas()->removeUIElement("fps");
                fps = nullptr;
            }
        }
    }

    void FPSCounter::createUI() {
        fps = canvasComponent->getCanvas()->addUiText("fps");

        fps->setFont("SpaceMono-Bold.ttf");
        fps->setBottom(20.0f, CgEngine::UIPosUnit::Pixel);
        fps->setLeft(15.0f, CgEngine::UIPosUnit::Pixel);
        fps->setSize(20.0f, CgEngine::UIPosUnit::Pixel);
        fps->setXAlignment(CgEngine::UIXAlignment::Left);
        fps->setYAlignment(CgEngine::UIYAlignment::Bottom);
        fps->setColor({1.0f, 1.0f, 1.0f, 1.0f});
    }
}
