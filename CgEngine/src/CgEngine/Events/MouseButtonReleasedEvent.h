#pragma once

#include "Event.h"
#include "KeyCodes.h"

namespace CgEngine {

    class MouseButtonReleasedEvent : public Event, public IEventHasMousePosition {
    public:
        MouseButtonReleasedEvent(const MouseButton button, const float x, const float y) : button(button), IEventHasMousePosition(x, y) {}

        inline MouseButton getButton() const {
            return button;
        }

        EVENT_TYPE_FN(MouseButtonReleased);

    private:
        MouseButton button;
    };

}
