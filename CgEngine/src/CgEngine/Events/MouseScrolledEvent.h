#pragma once

#include "Event.h"

namespace CgEngine {

    class MouseScrolledEvent : public Event {
    public:
        MouseScrolledEvent(const float xOffset, const float yOffset) : xOffset(xOffset), yOffset(yOffset) {}

        inline float getXOffsetPos() const {
            return xOffset;
        }
        inline float getYOffsetPos() const {
            return yOffset;
        }

        EVENT_TYPE_FN(MouseScrolled);

    private:
        float xOffset;
        float yOffset;
    };

}
