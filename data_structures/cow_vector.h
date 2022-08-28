#pragma once

#include <memory>
#include <vector>
#include <string>

struct State {
    int ref_count;

    std::vector<std::string> values;
};

class COWVector {
public:
    COWVector(){};
    ~COWVector() {
        if (state_->ref_count == 1) {
            delete state_;
        } else {
            --state_->ref_count;
        }
    }

    COWVector(const COWVector& other) {
        state_ = other.state_;
        ++state_->ref_count;
    };
    COWVector& operator=(const COWVector& other) {
        state_ = other.state_;
        ++state_->ref_count;
        return *this;
    };

    size_t Size() const {
        return state_->values.size();
    };

    void Resize(size_t size) {
        Relocate();
        state_->values.resize(size);
    };

    const std::string& Get(size_t at) {
        return state_->values[at];
    }
    const std::string& Back() {
        return state_->values.back();
    }

    void PushBack(const std::string& value) {
        Relocate();
        state_->values.push_back(value);
    }

    void Set(size_t at, const std::string& value) {
        Relocate();
        state_->values[at] = value;
    }

private:
    State* state_;

    void Relocate() {
        if (state_->ref_count != 1) {
            --state_->ref_count;
            State* state = new State;
            state_ = state;
            state_->ref_count = 1;
        }
    }
};
