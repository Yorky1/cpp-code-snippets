#pragma once

#include "sw_fwd.h"  // Forward declaration
#include <type_traits>
#include <utility>
#include <cstddef>  // std::nullptr_t

class ControlBlockBase {
public:
    size_t shared_ref_count = 0;
    size_t weak_ref_count = 0;
    virtual ~ControlBlockBase() = default;
};

template <typename T>
class ControlBlockPointer : public ControlBlockBase {
public:
    ControlBlockPointer(T *ptr) : ptr_(ptr) {
    }

    ~ControlBlockPointer() override {
        if (shared_ref_count == 0) {
            delete ptr_;
        }
    }

private:
    T *ptr_ = nullptr;
};

template <typename T>
class ControlBlockHolder : public ControlBlockBase {
public:
    template <typename... Args>
    ControlBlockHolder(Args &&...args) {
        new (&storage_) T(std::forward<Args>(args)...);
    }

    T *Get() {
        return reinterpret_cast<T *>(&storage_);
    }

    ~ControlBlockHolder() override {
        if (shared_ref_count == 0) {
            reinterpret_cast<T *>(&storage_)->~T();
        }
    }

private:
    typename std::aligned_storage_t<sizeof(T), alignof(T)>::type storage_;
};

template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() = default;

    SharedPtr(std::nullptr_t) {
    }

    template <typename U>
    explicit SharedPtr(U *ptr) : ptr_(ptr), base_(new ControlBlockPointer<U>(ptr)) {
        IncrementRefCount();
        if constexpr (std::is_convertible_v<U *, EnableSharedFromThisBase *>) {
            AddReferenceToSelf(ptr);
        } else {
        }
    }

    template <typename Y>
    SharedPtr(const SharedPtr<Y> &other) : ptr_(other.ptr_), base_(other.base_) {
        IncrementRefCount();
    }

    SharedPtr(const SharedPtr &other) : ptr_(other.ptr_), base_(other.base_) {
        IncrementRefCount();
    }

    template <typename Y>
    SharedPtr(SharedPtr<Y> &&other) : ptr_(other.ptr_), base_(other.base_) {
        other.ptr_ = nullptr;
        other.base_ = nullptr;
    }

    SharedPtr(SharedPtr &&other) : ptr_(other.ptr_), base_(other.base_) {
        other.ptr_ = nullptr;
        other.base_ = nullptr;
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y> &other, T *ptr) : ptr_(ptr), base_(other.base_) {
        IncrementRefCount();
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T> &other) : ptr_(other.ptr_), base_(other.base_) {
        if (other.Expired()) {
            throw BadWeakPtr();
        }
        IncrementRefCount();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr &operator=(const SharedPtr &other) {
        if (this == &other) {
            return *this;
        }
        Unlink();
        ptr_ = other.ptr_;
        base_ = other.base_;
        IncrementRefCount();
        return *this;
    }

    SharedPtr &operator=(SharedPtr &&other) {
        if (this == &other) {
            return *this;
        }
        Unlink();
        ptr_ = other.ptr_;
        base_ = other.base_;
        other.ptr_ = nullptr;
        other.base_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        Unlink();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        Unlink();
        ptr_ = nullptr;
        base_ = nullptr;
    }

    template <typename U>
    void Reset(U *ptr) {
        Unlink();
        ptr_ = ptr;
        base_ = new ControlBlockPointer<U>(ptr);
        IncrementRefCount();
    }

    void Swap(SharedPtr &other) {
        std::swap(ptr_, other.ptr_);
        std::swap(base_, other.base_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T *Get() const {
        return ptr_;
    }

    T &operator*() const {
        return *Get();
    }

    T *operator->() const {
        return Get();
    }

    size_t UseCount() const {
        return base_ ? base_->shared_ref_count : static_cast<size_t>(0);
    }

    explicit operator bool() const {
        return ptr_;
    }

private:
    T *ptr_ = nullptr;
    ControlBlockBase *base_ = nullptr;

private:
    void Unlink() {
        if (!base_) {
            return;
        }
        --base_->shared_ref_count;
        if (base_->shared_ref_count == 0 && base_->weak_ref_count == 0) {
            delete base_;
        } else if (base_->shared_ref_count == 0) {
            ptr_->~T();
        }
        base_ = nullptr;
        ptr_ = nullptr;
    }

    void IncrementRefCount() {
        if (base_) {
            ++base_->shared_ref_count;
        }
    }

    template <typename U>
    void AddReferenceToSelf(EnableSharedFromThis<U> *ptr) {
        ptr->self_ = *this;
    }

    template <typename U, typename... Args>
    friend SharedPtr<U> MakeShared(Args &&...args);

    template <typename U>
    friend class SharedPtr;

    template <typename U>
    friend class WeakPtr;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T> &left, const SharedPtr<U> &right) {
    return left.Get() == right.Get();
}

// Allocate memory only once
template <typename U, typename... Args>
SharedPtr<U> MakeShared(Args &&...args) {
    SharedPtr<U> sp;
    ControlBlockHolder<U> *holder = new ControlBlockHolder<U>(std::forward<Args>(args)...);
    ++holder->shared_ref_count;
    sp.base_ = holder;
    sp.ptr_ = holder->Get();
    if constexpr (std::is_convertible_v<U *, EnableSharedFromThisBase *>) {
        sp.AddReferenceToSelf(sp.ptr_);
    } else {
    }
    return sp;
}

class EnableSharedFromThisBase {};

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis : public EnableSharedFromThisBase {
public:
    SharedPtr<T> SharedFromThis() {
        return SharedPtr<T>(self_);
    }

    SharedPtr<const T> SharedFromThis() const {
        return SharedPtr<const T>(self_);
    }

    WeakPtr<T> WeakFromThis() noexcept {
        return self_;
    }

    WeakPtr<const T> WeakFromThis() const noexcept {
        return self_;
    }

private:
    WeakPtr<T> self_;

private:
    template <typename U>
    friend class SharedPtr;
};