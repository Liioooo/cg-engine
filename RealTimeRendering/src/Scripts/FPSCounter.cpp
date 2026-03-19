#include "FPSCounter.h"

#include "Components/UiCanvasComponent2D.h"

namespace RTR {
    void FPSCounter::onEnable() {
        fps = getOwingEntity().getComponent<CgEngine::UiCanvasComponent2D>()->getCanvas()->getUIElement<CgEngine::UiText>("fps");
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
}
