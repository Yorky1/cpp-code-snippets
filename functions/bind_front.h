#pragma once

#include <functional>
#include <utility>
#include <tuple>

template <class F, class... Args>
constexpr auto BindFront(F&& f, Args&&... args) {
    return [ f = std::forward<F>(f), ... args = std::forward<decltype(args)>(
                                         args) ]<class... Args2>(Args2 && ... args2) mutable {
        return f(std::forward<decltype(args)>(args)..., std::forward<Args2>(args2)...);
    };
}