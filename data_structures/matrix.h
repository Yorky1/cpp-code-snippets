#pragma once

#include <array>
#include <vector>
#include <stdexcept>

template <typename T>
struct Base {
    ~Base() = default;
};

template <typename L, typename R>
struct Glue : public Base<Glue<L, R>> {
    explicit Glue(const Base<L>& l, const Base<R>& r)
        : left(static_cast<const L&>(l)), right(static_cast<const R&>(r)) {
    }
    const L& left;
    const R& right;
};

template <typename L, typename R>
constexpr Glue<L, R> operator*(const Base<L>& left, const Base<R>& right) {
    return Glue(static_cast<const L&>(left), static_cast<const R&>(right));
}

template <class T>
class Matrix;

template <class T>
struct GetCount {
    static const size_t kCount = -1;
};

template <class T>
struct GetCount<Matrix<T>> {
    static const size_t kCount = 1;
};

template <class L, class R>
struct GetCount<Glue<L, R>> {
    static const size_t kCount = GetCount<L>::kCount + GetCount<R>::kCount;
};

template <class T, size_t N>
constexpr size_t AddOperands(size_t i, std::array<const Matrix<T>*, N>& arr, const Matrix<T>* now) {
    arr[i] = now;
    return i + 1;
}

template <class T, size_t N, class L, class R>
constexpr size_t AddOperands(size_t i, std::array<const Matrix<T>*, N>& arr,
                             const Glue<L, R>* now) {
    size_t next = AddOperands(i, arr, &now->left);
    return AddOperands(next, arr, &now->right);
}

template <class T>
constexpr Matrix<T> Multiply(const Matrix<T>& a, const Matrix<T>& b) {
    if (a.Columns() != b.Rows()) {
        throw std::runtime_error("Matrices are incompatible!");
    }
    Matrix<T> res(a.Rows(), b.Columns());
    for (size_t i = 0; i < a.Rows(); ++i) {
        for (size_t j = 0; j < b.Columns(); ++j) {
            for (size_t k = 0; k < a.Columns(); ++k) {
                res(i, j) += a(i, k) * b(k, j);
            }
        }
    }
    return res;
}

template <class T, size_t N>
constexpr Matrix<T> GetProd(size_t l, size_t r, std::array<const Matrix<T>*, N>& operands,
                            std::array<std::array<int, N>, N>& p) {
    if (l == r) {
        return *operands[l];
    }
    return Multiply(GetProd(l, p[l][r], operands, p), GetProd(p[l][r] + 1, r, operands, p));
}

template <class T>
class Matrix : public Base<Matrix<T>> {
public:
    Matrix(size_t n, size_t m) : data_(n, std::vector<T>(m)), rows_(n), columns_(m) {
    }

    Matrix(size_t n) : Matrix(n, n) {
    }

    Matrix(const std::vector<std::vector<T>>& data)
        : data_(data), rows_(data.size()), columns_(data.empty() ? 0 : data[0].size()) {
    }

    Matrix(std::initializer_list<std::vector<T>> list) : data_(list) {
        rows_ = data_.size();
        columns_ = data_.empty() ? 0 : data_[0].size();
    }

    size_t Rows() const {
        return rows_;
    }

    size_t Columns() const {
        return columns_;
    }

    T& operator()(size_t i, size_t j) {
        return data_[i][j];
    }

    const T& operator()(size_t i, size_t j) const {
        return data_[i][j];
    }

    template <typename L, typename R>
    constexpr Matrix(const Glue<L, R>& tree) {
        constexpr size_t kCount = GetCount<Glue<L, R>>::kCount;
        std::array<const Matrix<T>*, kCount> operands;
        AddOperands(0, operands, &tree);
        for (size_t i = 1; i < kCount; ++i) {
            if (operands[i]->Rows() != operands[i - 1]->Columns()) {
                throw std::runtime_error("Matrices are incompatible!");
            }
        }
        std::array<std::array<int64_t, kCount>, kCount> dp;
        std::array<std::array<int, kCount>, kCount> p;
        for (size_t len = 2; len <= kCount; ++len) {
            for (size_t l = 0, r = l + len - 1; r < kCount; ++l, ++r) {
                dp[l][r] = INT32_MAX;
                for (size_t i = l; i < r; ++i) {
                    int number =
                        dp[l][i] + dp[i + 1][r] +
                        operands[l]->Rows() * operands[i]->Columns() * operands[r]->Columns();
                    if (number < dp[l][r]) {
                        dp[l][r] = number;
                        p[l][r] = i;
                    }
                }
            }
        }
        *this = GetProd(static_cast<size_t>(0), kCount - 1, operands, p);
    }

    Matrix<T> operator-() const {
        Matrix<T> result(rows_, columns_);
        for (size_t i = 0; i < rows_; ++i) {
            for (size_t j = 0; j < columns_; ++j) {
                result(i, j) = -this->operator()(i, j);
            }
        }
        return result;
    }

private:
    std::vector<std::vector<T>> data_;
    size_t rows_ = 0;
    size_t columns_ = 0;

private:
    friend Matrix<T> operator-(const Matrix<T>& a, const Matrix<T>& b) {
        if (a.rows_ != b.rows_ || a.columns_ != b.columns_) {
            throw std::runtime_error("Matrices are incompatible!");
        }
        return a + (-b);
    }

    friend Matrix<T> operator+(const Matrix<T>& a, const Matrix<T>& b) {
        if (a.rows_ != b.rows_ || a.columns_ != b.columns_) {
            throw std::runtime_error("Matrices are incompatible!");
        }
        Matrix<T> result(a.rows_, a.columns_);
        for (size_t i = 0; i < a.rows_; ++i) {
            for (size_t j = 0; j < a.columns_; ++j) {
                result(i, j) = a(i, j) + b(i, j);
            }
        }
        return result;
    }
};
