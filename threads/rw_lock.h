#pragma once
#include <condition_variable>
#include <memory>
#include <mutex>

class RWLock {
public:
    template <class Func>
    void Read(Func func) {
        std::unique_lock<std::mutex> lock{global_};
        ++blocked_readers_;
        lock.unlock();
        try {
            func();
        } catch (...) {
            End(true);
            throw;
        }
        End(false);
    }

    template <class Func>
    void Write(Func func) {
        std::unique_lock lock(global_);
        while (blocked_readers_) {
            cv_.wait(lock);
        }
        func();
    }

private:
    std::mutex global_;
    std::condition_variable cv_;
    int blocked_readers_ = 0;
    void End(bool all) {
        std::lock_guard lock{global_};
        --blocked_readers_;
        cv_.notify_all();
    }
};