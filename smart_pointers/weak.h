#pragma once

#include "sw_fwd.h"  // Forward declaration
#include "shared.h"

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() = default;

    WeakPtr(const WeakPtr& other) : ptr_(other.ptr_), base_(other.base_) {
        IncrementRefCount();
    }

    template <typename U>
    WeakPtr(const WeakPtr<U>& other) : ptr_(other.ptr_), base_(other.base_) {
        IncrementRefCount();
    }

    WeakPtr(WeakPtr&& other) : ptr_(other.ptr_), base_(other.base_) {
        other.ptr_ = nullptr;
        other.base_ = nullptr;
    }

    template <typename U>
    WeakPtr(WeakPtr<U>&& other) : ptr_(other.ptr_), base_(other.base_) {
        other.ptr_ = nullptr;
        other.base_ = nullptr;
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    template <typename U>
    WeakPtr(const SharedPtr<U>& other) : ptr_(other.ptr_), base_(other.base_) {
        IncrementRefCount();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (this == &other) {
            return *this;
        }
        Unlink();
        ptr_ = other.ptr_;
        base_ = other.base_;
        IncrementRefCount();
        return *this;
    }

    template <typename U>
    WeakPtr& operator=(const WeakPtr<U>& other) {
        Unlink();
        ptr_ = other.ptr_;
        base_ = other.base_;
        IncrementRefCount();
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) {
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

    template <typename U>
    WeakPtr& operator=(WeakPtr<U>&& other) {
        Unlink();
        ptr_ = other.ptr_;
        base_ = other.base_;
        other.ptr_ = nullptr;
        other.base_ = nullptr;
        return *this;
    }

    template <typename U>
    WeakPtr& operator=(const SharedPtr<U>& other) {
        Unlink();
        ptr_ = other.ptr_;
        base_ = other.base_;
        IncrementRefCount();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        Unlink();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        Unlink();
        ptr_ = nullptr;
        base_ = nullptr;
    }
    void Swap(WeakPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(base_, other.base_);
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        return base_ ? base_->shared_ref_count : static_cast<size_t>(0);
    }
    bool Expired() const {
        return !base_ || !base_->shared_ref_count;
    }
    SharedPtr<T> Lock() const {
        if (Expired()) {
            return SharedPtr<T>();
        }
        SharedPtr<T> sp;
        sp.ptr_ = ptr_;
        sp.base_ = base_;
        sp.IncrementRefCount();
        return sp;
    }

private:
    T* ptr_ = nullptr;
    ControlBlockBase* base_ = nullptr;

private:
    void Unlink() {
        if (!base_) {
            return;
        }
        --base_->weak_ref_count;
        if (base_->shared_ref_count + base_->weak_ref_count == 0) {
            delete base_;
        }
    }
    void IncrementRefCount() {
        if (base_) {
            ++base_->weak_ref_count;
        }
    }

private:
    template <typename U>
    friend class SharedPtr;

    template <typename U>
    friend class WeakPtr;
};