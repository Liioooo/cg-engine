#include "FPSCounter.h"

namespace RTR {
    void FPSCounter::onAttach() {
        createUI();
    }

    void FPSCounter::update(CgEngine::TimeStep ts) {
        frameTimes[currentIndex] = ts.getSeconds();
        currentIndex = (currentIndex + 1) % frameTimes.size();

        float sum = 0.0f;
        for (const auto& ft: frameTimes) {
            sum += ft;
        }

        sum /= static_cast<float>(frameTimes.size());
        fps->setText(std::to_string(static_cast<uint32_t>(1.0f / sum)));
    }

    void FPSCounter::createUI() {
        auto& gameCanvas = getComponent<CgEngine::UiCanvasComponent2D>();
        fps = gameCanvas.addUiText("fps");

        fps->setFont("SpaceMono-Bold.ttf");
        fps->setBottom(20.0f, CgEngine::UIPosUnit::Pixel);
        fps->setLeft(15.0f, CgEngine::UIPosUnit::Pixel);
        fps->setSize(20.0f, CgEngine::UIPosUnit::Pixel);
        fps->setXAlignment(CgEngine::UIXAlignment::Left);
        fps->setYAlignment(CgEngine::UIYAlignment::Bottom);
        fps->setColor({1.0f, 1.0f, 1.0f, 1.0f});
    }
}
