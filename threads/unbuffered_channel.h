#pragma once

#include <utility>
#include <optional>
#include <condition_variable>
#include <mutex>

template <class T>
class UnbufferedChannel {
public:
    void Send(const T& value) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while (!closed_ && state_ != 0) {
                can_send_.wait(lock);
            }
            if (closed_) {
                throw std::runtime_error("");
            }
            ++state_;
            recv_ = value;
            has_value_.notify_one();
            while (!closed_ && state_ != 2) {
                value_getted_.wait(lock);
            }
            if (closed_ && state_ != 2) {
                throw std::runtime_error("");
            }
            state_ = 0;
            recv_.reset();
        }
        can_send_.notify_one();
    }

    std::optional<T> Recv() {
        std::optional<T> res;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while (!closed_ && state_ != 1) {
                has_value_.wait(lock);
            }
            if (closed_) {
                return std::nullopt;
            }
            res = recv_;
            ++state_;
        }
        value_getted_.notify_one();
        return res;
    }

    void Close() {
        closed_ = true;
        can_send_.notify_all();
        has_value_.notify_all();
        value_getted_.notify_all();
    }

private:
    std::mutex mutex_;
    std::condition_variable can_send_;
    std::condition_variable has_value_;
    std::condition_variable value_getted_;
    std::optional<T> recv_;
    int state_ = 0;
    std::atomic_bool closed_ = false;
};