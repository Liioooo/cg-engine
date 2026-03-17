#pragma once

#include "Event.h"

namespace CgEngine {

    class MouseMovedEvent : public Event, public IEventHasMousePosition {
    public:
        MouseMovedEvent(const float x, const float y) : IEventHasMousePosition(x, y) {}

        EVENT_TYPE_FN(MouseMoved);
    };

}
