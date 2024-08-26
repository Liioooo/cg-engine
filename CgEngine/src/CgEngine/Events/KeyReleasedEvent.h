#pragma once

#include "Event.h"
#include "KeyCodes.h"

namespace CgEngine {

    class KeyReleasedEvent : public Event {
    public:
        KeyReleasedEvent(const KeyCode keyCode, const ModifierKey modifierKeys) : keyCode(keyCode), modifierKeys(modifierKeys) {};

        inline KeyCode getKeyCode() const {
            return keyCode;
        }
        inline ModifierKey getModifierKeys() const {
            return modifierKeys;
        }

        EVENT_TYPE_FN(KeyReleased);

    private:
        KeyCode keyCode;
        ModifierKey modifierKeys;
    };

}
