#pragma once

namespace CgEngine {

    class AudioThread {
    public:
        static bool start();
        static bool stop();
        static bool isRunning();
        static bool isAudioThread();
        static void addTask(std::function<void()> task);
        static void runOnAudioThread(std::function<void()> task);

        template<auto TFunction, class TClass>
        static void setOnUpdateCallback(TClass* object) {
            onUpdateCallback = std::bind(TFunction, object);
        }

        template<auto TFunction, class TClass>
        static void setOnShutdownCallback(TClass* object) {
            onShutdownCallback = std::bind(TFunction, object);
        }

    private:
        static inline std::thread* thread{nullptr};
        static inline std::atomic<bool> threadActive = false;
        static inline std::atomic<std::thread::id> audioThreadId{std::thread::id()};

        static inline std::queue<std::function<void()>> audioThreadTasks;
        static inline std::mutex audioThreadTasksLock;

        static inline std::function<void()> onUpdateCallback;
        static inline std::function<void()> onShutdownCallback;

        static void onUpdate();
    };

}
