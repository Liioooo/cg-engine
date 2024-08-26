#pragma once

#include "Event.h"

namespace CgEngine {

    class WindowResizeEvent : public Event {
    public:
        WindowResizeEvent(const int width, const int height) : width(width), height(height) {}

        inline int getWidth() const {
            return width;
        }
        inline int getHeight() const {
            return height;
        }

        EVENT_TYPE_FN(WindowResize);

    private:
        int width;
        int height;
    };

}
