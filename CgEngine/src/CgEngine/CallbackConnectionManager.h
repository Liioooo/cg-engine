#pragma once

namespace CgEngine {

    template <typename Fn>
    struct CallbackConnectionContainer {
        std::unordered_map<size_t, Fn> listeners;
        size_t nextId = 0;

        template <typename F>
        size_t addCallback(F&& callback) {
            size_t id = ++nextId;
            listeners[id] = listeners[id] = std::forward<F>(callback);
            return id;
        }
    };

    template <typename Fn>
    struct CallbackConnection {
        std::weak_ptr<CallbackConnectionContainer<Fn>> container;
        size_t id = 0;

        CallbackConnection(std::weak_ptr<CallbackConnectionContainer<Fn>> container, size_t id) : container(std::move(container)), id(id) {}

        ~CallbackConnection() {
            if (auto cont = container.lock()) {
                if (id != 0) {
                    cont->listeners.erase(id);
                }
            }
        }

        CallbackConnection(CallbackConnection&& other) noexcept : container(std::move(other.container)), id(other.id) {
            other.id = 0;
        }

        CallbackConnection& operator=(CallbackConnection&& other) noexcept {
            if (this != &other) {
                container = std::move(other.container);
                id = other.id;
                other.id = 0;
            }
            return *this;
        }

        CallbackConnection(const CallbackConnection&) = delete;
        CallbackConnection& operator=(const CallbackConnection&) = delete;

        void disconnect() {
            if (auto cont = container.lock()) {
                cont->listeners.erase(id);
            }
            id = 0;
        }
    };

    template <typename Fn>
    class CallbackConnectionManager {
    public:
        CallbackConnectionManager() = default;

        CallbackConnectionManager(const CallbackConnectionManager&) = delete;
        CallbackConnectionManager& operator=(const CallbackConnectionManager&) = delete;

        template <typename F>
        CallbackConnection<Fn> addCallback(F&& callback) {
            size_t id = callbackConnectionContainer->addCallback(std::forward<F>(callback));
            return CallbackConnection<Fn>{callbackConnectionContainer, id};
        }

        template <typename... Args>
        void callCallbacks(Args&&... args) const {
            for (const auto& [_, callback]: callbackConnectionContainer->listeners) {
                callback(std::forward<Args>(args)...);
            }
        }

    private:
        std::shared_ptr<CallbackConnectionContainer<Fn>> callbackConnectionContainer = std::make_shared<CallbackConnectionContainer<Fn>>();
    };

}
