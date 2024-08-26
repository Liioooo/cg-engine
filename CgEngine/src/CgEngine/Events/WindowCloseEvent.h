#pragma once

#include "Event.h"

namespace CgEngine {

    class WindowCloseEvent : public Event {
    public:
        WindowCloseEvent() = default;

        EVENT_TYPE_FN(WindowClose);
    };

}
