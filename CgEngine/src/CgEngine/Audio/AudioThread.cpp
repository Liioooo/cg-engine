#include "AudioThread.h"

namespace CgEngine {
    static std::queue<std::function<void()>> audioThreadTasksLocal;

    bool AudioThread::start() {
        if (threadActive) {
            return false;
        }

        threadActive = true;
        thread = new std::thread([] {
            while (threadActive) {
                onUpdate();
            }
            onShutdownCallback();
        });

        audioThreadId = thread->get_id();

        return true;
    }

    bool AudioThread::stop() {
        if (!threadActive) {
            return false;
        }
        threadActive = false;
        thread->join();
        return true;
    }

    bool AudioThread::isRunning() {
        return threadActive;
    }

    bool AudioThread::isAudioThread() {
        return std::this_thread::get_id() == audioThreadId;
    }

    void AudioThread::addTask(std::function<void()> task) {
        std::scoped_lock lock(audioThreadTasksLock);
        audioThreadTasks.emplace(std::move(task));
    }

    void AudioThread::runOnAudioThread(std::function<void()> task) {
        if (isAudioThread()) {
            task();
        } else {
            addTask(std::move(task));
        }
    }

    void AudioThread::onUpdate() {
        auto& tasks = audioThreadTasksLocal;
        {
            std::scoped_lock lock(audioThreadTasksLock);
            audioThreadTasksLocal = audioThreadTasks;
            audioThreadTasks = std::queue<std::function<void()>>();
        }
        if (!tasks.empty()) {
            for (int i = static_cast<int>(tasks.size() - 1); i >= 0; i--) {
                auto task = tasks.front();
                task();
                tasks.pop();
            }
        }

        onUpdateCallback();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
