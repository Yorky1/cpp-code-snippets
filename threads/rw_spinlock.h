#pragma once

#include <atomic>

struct RWSpinLock {
    void LockRead() {
        uint32_t old = counter;
        while (true) {
            if (old & 1) {
                old = counter;
                continue;
            }
            if (!counter.compare_exchange_weak(old, old + 2)) {
                continue;
            }
            break;
        }
    }

    void UnlockRead() {
        counter -= 2;
    }

    void LockWrite() {
        uint32_t old = counter;
        while (true) {
            if (old != 0) {
                old = counter;
                continue;
            }
            if (!counter.compare_exchange_weak(old, old + 1)) {
                continue;
            }
            break;
        }
    }

    void UnlockWrite() {
        counter--;
    }

    std::atomic_uint32_t counter;
};
