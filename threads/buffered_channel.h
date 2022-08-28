#pragma once

#include <utility>
#include <optional>
#include <queue>
#include <mutex>
#include <condition_variable>

template <class T>
class BufferedChannel {
public:
    explicit BufferedChannel(int size) : size_(size) {
    }

    void Send(const T& value) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while (!closed_ && queue_.size() == size_) {
                can_send_.wait(lock);
            }
            if (closed_) {
                throw std::runtime_error("");
            }
            queue_.push(value);
        }
        can_recv_.notify_one();
    }

    std::optional<T> Recv() {
        std::optional<T> res;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while (!closed_ && queue_.empty()) {
                can_recv_.wait(lock);
            }
            if (queue_.empty()) {
                return std::nullopt;
            }
            res = queue_.front();
            queue_.pop();
        }
        can_send_.notify_one();
        return res;
    }

    void Close() {
        closed_ = true;
        can_send_.notify_all();
        can_recv_.notify_all();
    }

private:
    std::mutex mutex_;
    std::condition_variable can_send_;
    std::condition_variable can_recv_;
    std::queue<T> queue_;
    int new_index_send_ = 0;
    int next_index_send_ = 0;
    size_t size_ = 0;
    std::atomic_bool closed_ = false;
};
