#pragma once

namespace CgEngine {

    class TimeStep {
    public:
        TimeStep() = default;
        TimeStep(float step) : step(step) {}

        inline float getSeconds() const {
            return step;
        }

        inline float getMillis() const {
            return step * 1000.0f;
        }

    private:
        float step = 0.0f;
    };

}
