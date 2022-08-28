#pragma once

#include <boost/context/continuation.hpp>
#include <functional>
#include <utility>

namespace ctx = boost::context;

class Coroutine {
public:
    explicit Coroutine(std::function<void()> f) {
        current_coroutine_ = this;
        source_ = ctx::callcc([f, this](ctx::continuation&& sink) {
            sink_ = std::move(sink);
            f();
            current_coroutine_ = nullptr;
            return std::move(sink_);
        });
    }

    void resume() {
        if (!source_) {
            throw std::runtime_error("no source");
        }
        current_coroutine_ = this;
        source_ = source_.resume();
    }

    void suspend() {
        current_coroutine_ = nullptr;
        sink_ = sink_.resume();
    }

    static inline thread_local Coroutine* current_coroutine_ = nullptr;

private:
    ctx::continuation source_;
    ctx::continuation sink_;
};

void suspend() {
    if (!Coroutine::current_coroutine_) {
        throw std::runtime_error("no current_coroutine");
    }
    Coroutine::current_coroutine_->suspend();
}

template <typename T>
class Yield {
public:
    template <typename U>
    void yield(U&& u) {
        yield_result_ = u;
        was_yield_ = true;
        suspend();
    }
    T yield_result_;
    bool was_yield_ = false;
};

template <typename T>
class Generator : public Yield<T> {
public:
    Generator(std::function<void(Yield<T>&)> f) : coroutine_([f, this] {suspend();f(*this);}){
    }

    const T* get() {
        coroutine_.resume();
        if (!Yield<T>::was_yield_) {
            end_func_ = true;
        }
        Yield<T>::was_yield_ = false;
        return end_func_ ? nullptr : &(Yield<T>::yield_result_);
    }

private:
    bool end_func_ = false;
    Coroutine coroutine_;
};
