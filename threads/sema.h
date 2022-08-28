#pragma once

#include <mutex>
#include <condition_variable>
#include <set>

class DefaultCallback {
public:
    void operator()(int& value) {
        --value;
    }
};

class Semaphore {
public:
    Semaphore(int count) : count_(count) {
    }

    void Leave() {
        std::unique_lock<std::mutex> lock(mutex_);
        ++count_;
        cv_.notify_all();
    }

    template <class Func>
    void Enter(Func callback) {
        std::unique_lock<std::mutex> lock(mutex_);
        int my_id = new_index_++;
        while (count_ == 0 || next_index_ != my_id) {
            cv_.wait(lock);
        }
        ++next_index_;
        callback(count_);
    }

    void Enter() {
        DefaultCallback callback;
        Enter(callback);
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    int count_ = 0;
    int new_index_ = 0;
    int next_index_ = 0;
};
