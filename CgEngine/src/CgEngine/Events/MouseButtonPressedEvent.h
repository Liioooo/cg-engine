#pragma once

#include "Event.h"
#include "KeyCodes.h"

namespace CgEngine {

    class MouseButtonPressedEvent : public Event {
    public:
        MouseButtonPressedEvent(const MouseButton button, const float x, const float y) : button(button), x(x), y(y) {}

        inline MouseButton getButton() const {
            return button;
        }

        inline float getXPos() const {
            return x;
        }
        inline float getYPos() const {
            return y;
        }

        EVENT_TYPE_FN(MouseButtonPressed);

    private:
        MouseButton button;
        float x;
        float y;
    };

}
