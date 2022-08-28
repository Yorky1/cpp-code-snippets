#pragma once

#include <atomic>
#include <optional>
#include <stdexcept>
#include <utility>

template <class T>
class MPSCStack {
public:
    // Push adds one element to stack top.
    //
    // Safe to call from multiple threads.
    void Push(const T& value) {
        Node* old_head = head_;
        Node* new_head = new Node{.value = value, .next = old_head};
        while (!head_.compare_exchange_weak(old_head, new_head)) {
            new_head->next = old_head;
        }
    }

    // Pop removes top element from the stack.
    //
    // Not safe to call concurrently.
    std::optional<T> Pop() {
        Node* old_head = head_;
        if (!old_head) {
            return std::nullopt;
        }
        while (!head_.compare_exchange_weak(old_head, old_head->next)) {
        }
        std::optional<T> result = std::move(old_head->value);
        delete old_head;
        return result;
    }

    // DequeuedAll Pop's all elements from the stack and calls cb() for each.
    //
    // Not safe to call concurrently with Pop()
    template <class TFn>
    void DequeueAll(const TFn& cb) {
        Node* old_head = head_;
        while (old_head) {
            while (!head_.compare_exchange_weak(old_head, old_head->next)) {
            }
            cb(old_head->value);
            delete old_head;
            old_head = head_;
        }
    }

    ~MPSCStack() {
        DequeueAll([](const T&) {});
    }

private:
    struct Node {
        T value;
        Node* next;
    };

private:
    std::atomic<Node*> head_;
};
