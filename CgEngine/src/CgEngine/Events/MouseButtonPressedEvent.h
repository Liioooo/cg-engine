#pragma once

#include "Event.h"
#include "KeyCodes.h"

namespace CgEngine {

    class MouseButtonPressedEvent : public Event, public IEventHasMousePosition {
    public:
        MouseButtonPressedEvent(const MouseButton button, const float x, const float y) : button(button), IEventHasMousePosition(x, y) {}

        inline MouseButton getButton() const {
            return button;
        }

        EVENT_TYPE_FN(MouseButtonPressed);

    private:
        MouseButton button;
    };

}
