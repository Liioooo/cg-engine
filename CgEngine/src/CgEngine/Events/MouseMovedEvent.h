#pragma once

#include "Event.h"

namespace CgEngine {

    class MouseMovedEvent : public Event {
    public:
        MouseMovedEvent(const float x, const float y) : x(x), y(y) {}

        inline float getXPos() const {
            return x;
        }
        inline float getYPos() const {
            return y;
        }

        EVENT_TYPE_FN(MouseMoved);

    private:
        float x;
        float y;
    };

}
