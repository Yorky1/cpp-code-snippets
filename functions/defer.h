#pragma once

#include <utility>

template <typename Callback>
class CallbackStorage {
public:
    explicit CallbackStorage(Callback&& callback) {
        new (GetCallbackBuffer()) Callback(std::move(callback));
    }

    void* GetCallbackBuffer() {
        return static_cast<void*>(callback_buffer_);
    }

    Callback& GetCallback() {
        return *reinterpret_cast<Callback*>(GetCallbackBuffer());
    }

    ~CallbackStorage() {
        reinterpret_cast<CallbackStorage*>(callback_buffer_)->~Call
    }

private:
    alignas(Callback) char callback_buffer_[sizeof(Callback)];
};

template <typename Callback>
class Defer final {
public:
    Defer(Callback&& callback) : storage_(std::forward<Callback>(callback)) {
    }

    void operator()() && {
        if (canceled_) {
            return;
        }
        auto to_del = std::move(storage_.GetCallback());
        std::move(to_del)();
        canceled_ = true;
    }

    void Invoke() && {
        if (canceled_) {
            return;
        }
        auto to_del = std::move(storage_.GetCallback());
        std::move(to_del)();
        canceled_ = true;
    }

    void Cancel() && {
        auto to_del = std::move(storage_.GetCallback());
        canceled_ = true;
    }

    ~Defer() {
        if (!canceled_) {
            auto to_del = std::move(storage_.GetCallback());
            std::move(to_del)();
        }
    }

private:
    CallbackStorage<Callback> storage_;
    bool canceled_ = false;
};
