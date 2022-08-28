#pragma once

#include "compressed_pair.h"

#include <algorithm>
#include <cstddef>  // std::nullptr_t
#include <memory>

struct Slug {};

// Primary template
template <typename T, typename Deleter = std::default_delete<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

<<<<<<< Updated upstream
    explicit UniquePtr(T* ptr = nullptr);
    UniquePtr(T* ptr, Deleter deleter);
=======
    explicit UniquePtr(T* ptr = nullptr) {
        data_.GetFirst() = ptr;
    }
>>>>>>> Stashed changes

    template <typename D>
    UniquePtr(T* ptr, D&& deleter) : data_(ptr, std::forward<D>(deleter)) {
    }

    UniquePtr(UniquePtr& other) = delete;

    template <typename U, typename D>
    UniquePtr(UniquePtr<U, D>&& other) noexcept {
        data_.GetFirst() = other.Release();
        GetDeleter() = std::forward<D>(other.GetDeleter());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr& other) noexcept = delete;

    template <class U, class D>
    UniquePtr& operator=(UniquePtr<U, D>&& other) noexcept {
        if (data_.GetFirst() == other.Get()) {
            return *this;
        }
        DeleteObject(data_);
        data_.GetFirst() = other.Release();
        GetDeleter() = std::forward<D>(other.GetDeleter());
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        DeleteObject(data_);
        data_.GetFirst() = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        DeleteObject(data_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* result = data_.GetFirst();
        data_.GetFirst() = nullptr;
        return result;
    }
    void Reset(T* ptr = nullptr) {
        auto to_delete = std::move(data_);
        data_.GetFirst() = ptr;
        DeleteObject(to_delete);
    }
    void Swap(UniquePtr& other) {
        std::swap(data_, other.data_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return data_.GetFirst();
    }
    Deleter& GetDeleter() {
        return data_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return data_.GetSecond();
    }
    explicit operator bool() const {
        return Get();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    auto& operator*() const {
        if constexpr (std::is_same<T, void>()) {
            return Get();
        } else {
            return *Get();
        }
    }

    T* operator->() const {
        return Get();
    }

private:
    void DeleteObject(CompressedPair<T*, Deleter>& data) {
        data.GetSecond()(data.GetFirst());
        data.GetFirst() = nullptr;
    }

    CompressedPair<T*, Deleter> data_;
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) {
        data_.GetFirst() = ptr;
    }

    template <typename D>
    UniquePtr(T* ptr, D&& deleter) : data_(ptr, std::forward<D>(deleter)) {
    }

    UniquePtr(UniquePtr& other) = delete;

    template <typename U, typename D>
    UniquePtr(UniquePtr<U, D>&& other) noexcept {
        data_.GetFirst() = other.Release();
        GetDeleter() = std::forward<D>(other.GetDeleter());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr& other) noexcept = delete;

    template <class U, class D>
    UniquePtr& operator=(UniquePtr<U, D>&& other) noexcept {
        if (data_.GetFirst() == other.Get()) {
            return *this;
        }
        DeleteObject(data_);
        data_.GetFirst() = other.Release();
        GetDeleter() = std::forward<D>(other.GetDeleter());
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        DeleteObject(data_);
        data_.GetFirst() = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        DeleteObject(data_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* result = data_.GetFirst();
        data_.GetFirst() = nullptr;
        return result;
    }
    void Reset(T* ptr = nullptr) {
        auto to_delete = std::move(data_);
        data_.GetFirst() = ptr;
        DeleteObject(to_delete);
    }
    void Swap(UniquePtr& other) {
        std::swap(data_, other.data_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return data_.GetFirst();
    }
    Deleter& GetDeleter() {
        return data_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return data_.GetSecond();
    }
    explicit operator bool() const {
        return Get();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    auto& operator*() const {
        if constexpr (std::is_same<T, void>()) {
            return Get();
        } else {
            return *Get();
        }
    }

    T& operator[](size_t i) {
        return Get()[i];
    }

    const T& operator[](size_t i) const {
        return Get()[i];
    }

    T* operator->() const {
        return Get();
    }

private:
    void DeleteObject(CompressedPair<T*, Deleter>& data) {
        data.GetSecond()(data.GetFirst());
    }

    CompressedPair<T*, Deleter> data_;
};
