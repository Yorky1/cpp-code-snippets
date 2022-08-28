#pragma once

#include <boost/context/continuation.hpp>
#include <functional>
#include <utility>

namespace ctx=boost::context;

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
