# pragma once

namespace CgEngine {

    enum class EventType {
        Undefined,
        WindowClose,
        WindowResize,
        KeyPressed,
        KeyReleased,
        MouseButtonPressed,
        MouseButtonReleased,
        MouseMoved,
        MouseScrolled
    };

    class Event {
        friend class EventDispatcher;
    public:
        virtual ~Event() = default;

        virtual EventType getEventType() const = 0;
        virtual std::string getEventName() const = 0;
        void stopPropagation() {
            handled = true;
        }
        inline bool wasHandled() const {
            return handled;
        }

    private:
        bool handled = false;
    };

    class EventDispatcher {
    public:
        explicit EventDispatcher(Event& event) : event(event) {};

        template<typename E>
        void dispatch(const std::function<void(E& event)>& handler) {
            if (event.getEventType() == E::getStaticEventType()) {
                handler(static_cast<E&>(event));
            }
        }

    private:
        Event& event;
    };

}

#define EVENT_TYPE_FN(eventType) static EventType getStaticEventType() { return EventType::eventType; } \
                                 virtual EventType getEventType() const override { return EventType::eventType; } \
                                 virtual std::string getEventName() const override { return #eventType; }

#define EVENT_BIND_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
