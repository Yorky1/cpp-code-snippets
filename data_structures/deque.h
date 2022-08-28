#pragma once

#include <memory>
#include <algorithm>
#include <iterator>
#include <utility>
#include <initializer_list>
#include <array>

const size_t kSize = 128;

class Deque {
public:
    Deque() : size_(0), capacity_(0) {
    }
    Deque(const Deque& rhs) : size_(rhs.size_), capacity_(rhs.capacity_) {
        if (capacity_) {
            start_ = new Block[capacity_];
            end_ = start_ + capacity_;
            for (size_t idx = 0; idx < capacity_; ++idx) {
                start_[idx] = Block(new Container(*rhs.start_[idx]));
            }
            front_ = {.ptr_ = start_, .id_ = rhs.front_.id_};
            back_ = {.ptr_ = start_ + (rhs.back_.GetPtr() - rhs.start_), .id_ = rhs.back_.id_};
        }
    }
    Deque(Deque&& rhs)
        : size_(rhs.size_),
          capacity_(rhs.capacity_),
          start_(rhs.start_),
          end_(rhs.end_),
          front_(rhs.front_),
          back_(rhs.back_) {
        rhs.start_ = rhs.end_ = nullptr;
        rhs.front_ = rhs.back_ = {nullptr, 0};
        rhs.size_ = rhs.capacity_ = 0;
    }

    explicit Deque(size_t size) : size_(size) {
        capacity_ = std::max(static_cast<size_t>(2), (size + kSize - 1) / kSize);
        Build(capacity_);
        for (size_t i = 0; i < size; ++i) {
            PushBack(0);
        }
    }

    Deque(std::initializer_list<int> list) {
        capacity_ = std::max(static_cast<size_t>(2), (list.size() + kSize - 1) / kSize);
        Build(capacity_);
        for (int x : list) {
            PushBack(x);
        }
    }

    Deque& operator=(Deque rhs) {
        Swap(rhs);
        return *this;
    }

    void Swap(Deque& rhs) {
        std::swap(size_, rhs.size_);
        std::swap(capacity_, rhs.capacity_);
        std::swap(start_, rhs.start_);
        std::swap(end_, rhs.end_);
        std::swap(front_, rhs.front_);
        std::swap(back_, rhs.back_);
    }

    void PushBack(int value) {
        Reallocate();
        back_.GetValue() = value;
        back_ = Advance(back_, 1);
        ++size_;
    }

    void PopBack() {
        back_ = Prev(back_);
        --size_;
    }

    void PushFront(int value) {
        Reallocate();
        front_.GetValue() = value;
        front_ = Prev(front_);
        ++size_;
    }

    void PopFront() {
        front_ = Advance(front_, 1);
        --size_;
    }

    int& operator[](size_t ind) {
        return Advance(front_, ind + 1).GetValue();
    }

    int operator[](size_t ind) const {
        return Advance(front_, ind + 1).GetValue();
    }

    size_t Size() const {
        return size_;
    }

    void Clear() {
        size_ = 0;
        front_.ptr_ = start_;
        front_.id_ = kSize - 1;
        back_.ptr_ = start_ + 1;
        back_.id_ = 0;
    }

    ~Deque() {
        delete[] start_;
    }

private:
    void Reallocate() {
        if (capacity_ == 0) {
            Build(2);
            return;
        }
        if ((Prev(front_).GetPtr() == front_.GetPtr() || Prev(front_).GetPtr() != back_.GetPtr()) &&
            (Advance(back_, 1).GetPtr() == back_.GetPtr() ||
             Advance(back_, 1).GetPtr() != front_.GetPtr())) {
            return;
        }
        capacity_ = capacity_ * 2;
        Block* new_start = new Block[capacity_];
        size_t idx = 0;
        for (Block* now = front_.ptr_; now != back_.ptr_; now = Advance(now, 1), ++idx) {
            new_start[idx] = std::move(*now);
        }
        new_start[idx++] = std::move(*back_.ptr_);
        delete[] start_;
        start_ = new_start;
        end_ = start_ + capacity_;
        front_.ptr_ = start_;
        back_.ptr_ = start_ + idx - 1;
        for (; idx < capacity_; ++idx) {
            new_start[idx] = Block(new Container);
        }
    }

    void Build(size_t size) {
        start_ = new Block[size];
        end_ = start_ + size;
        front_ = {.ptr_ = start_, .id_ = kSize - 1u};
        back_ = {.ptr_ = start_ + 1, .id_ = 0u};
        size_ = 0;
        capacity_ = size;
        for (size_t i = 0; i < size; ++i) {
            start_[i] = Block(new Container);
        }
    }

private:
    using Container = std::array<int, kSize>;
    using Block = std::unique_ptr<Container>;

    Block* Prev(Block* ptr) const {
        if (ptr == start_) {
            ptr = end_ - 1;
        } else {
            --ptr;
        }
        return ptr;
    }

    Block* Advance(Block* ptr, size_t pls) const {
        if (static_cast<size_t>(end_ - ptr) > pls) {
            ptr += pls;
            return ptr;
        }
        pls -= (end_ - ptr);
        ptr = start_ + pls % capacity_;
        return ptr;
    }

    struct DequeIterator {
        Block* ptr_ = nullptr;
        size_t id_ = 0;

        Block* GetPtr() const {
            return ptr_;
        }

        int GetValue() const {
            return (*ptr_)->at(id_);
        }

        int& GetValue() {
            return (*ptr_)->at(id_);
        }
    };

    Deque::DequeIterator Prev(Deque::DequeIterator it) const {
        if (it.id_ == 0) {
            it.ptr_ = Prev(it.ptr_);
            it.id_ = kSize - 1;
        } else {
            --it.id_;
        }
        return it;
    }

    Deque::DequeIterator Advance(Deque::DequeIterator it, size_t pls) const {
        if (kSize - it.id_ > pls) {
            it.id_ += pls;
            return it;
        }
        pls -= (kSize - it.id_);
        it.ptr_ = Advance(it.ptr_, pls / kSize + 1);
        it.id_ = pls % kSize;
        return it;
    }

private:
    size_t size_ = 0;
    size_t capacity_ = 0;
    Block* start_ = nullptr;
    Block* end_ = nullptr;
    DequeIterator front_;
    DequeIterator back_;
};
