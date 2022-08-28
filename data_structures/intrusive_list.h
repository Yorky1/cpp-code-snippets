#pragma once

#include <algorithm>

class ListHook {
public:
    ListHook() = default;

    bool IsLinked() const {
        return linked_;
    }

    void Unlink() {
        if (linked_) {
            prev_->next_ = next_;
            next_->prev_ = prev_;
            next_ = prev_ = this;
            linked_ = false;
        }
    }

    // Must unlink element from list
    ~ListHook() {
        Unlink();
    }

    ListHook(const ListHook&) = delete;

private:
    template <class T>
    friend class List;

    ListHook* prev_ = this;
    ListHook* next_ = this;
    bool linked_ = false;

    // that helper function might be useful
    void LinkBefore(ListHook* other) {
        other->prev_->next_ = this;
        prev_ = other->prev_;
        other->prev_ = this;
        next_ = other;
        linked_ = true;
    }
};

template <typename T>
class List {
public:
    class Iterator : public std::iterator<std::bidirectional_iterator_tag, T> {
    public:
        Iterator(ListHook* ptr) : now_(ptr) {
        }
        Iterator& operator++() {
            now_ = now_->next_;
            return *this;
        }
        Iterator operator++(int) {
            Iterator result = *this;
            ++(*this);
            return result;
        }

        T& operator*() const {
            return static_cast<T&>(*now_);
        }
        T* operator->() const {
            return static_cast<T*>(now_);
        }

        bool operator==(const Iterator& rhs) const {
            return now_ == rhs.now_;
        }
        bool operator!=(const Iterator& rhs) const {
            return now_ != rhs.now_;
        }

    private:
        ListHook* now_;
    };

    List() : start_(new ListHook()) {
        start_->linked_ = true;
    }
    List(const List&) = delete;
    List(List&& other) : start_(new ListHook()) {
        start_->LinkBefore(other.start_->next_);
        other.start_->Unlink();
    }

    // must unlink all elements from list
    ~List() {
        UnlinkAll();
        delete start_;
    }

    List& operator=(const List&) = delete;
    List& operator=(List&& other) {
        if (start_ == other.start_) {
            return *this;
        }
        UnlinkAll();
        start_->LinkBefore(other.start_->next_);
        other.start_->Unlink();
        return *this;
    }

    bool IsEmpty() const {
        return start_ == start_->next_;
    }
    // that method is allowed to be O(n)
    size_t Size() const {
        size_t size = 0;
        ListHook* now = start_->next_;
        while (now != start_) {
            ++size;
            now = now->next_;
        }
        return size;
    }

    // note that IntrusiveList doesn't own elements,
    // and never copies or moves T
    void PushBack(T* elem) {
        elem->LinkBefore(start_);
    }
    void PushFront(T* elem) {
        elem->LinkBefore(start_->next_);
    }

    T& Front() {
        return static_cast<T&>(*start_->next_);
    }
    const T& Front() const {
        return static_cast<const T&>(*start_->next_);
    }

    T& Back() {
        return static_cast<T&>(*start_->prev_);
    }
    const T& Back() const {
        return static_cast<const T&>(*start_->prev_);
    }

    void PopBack() {
        start_->prev_->Unlink();
    }
    void PopFront() {
        start_->next_->Unlink();
    }

    Iterator Begin() {
        return Iterator(start_->next_);
    }
    Iterator End() {
        return Iterator(start_);
    }

    // complexity of this function must be O(1)
    Iterator IteratorTo(T* element) {
        return Iterator(element);
    }

private:
    void UnlinkAll() {
        ListHook* now = start_->next_;
        while (now != start_) {
            ListHook* next = now->next_;
            now->Unlink();
            now = next;
        }
    }
    ListHook* start_;
};

template <typename T>
typename List<T>::Iterator begin(List<T>& list) {  // NOLINT
    return list.Begin();
}

template <typename T>
typename List<T>::Iterator end(List<T>& list) {  // NOLINT
    return list.End();
}
