#pragma once

#include "MathFunctions.h"
#include <cstddef>
#include <algorithm>
#include <cstring>

// 检测类型是否对齐
template<size_t N, size_t Align>
struct IsAligned {
    static constexpr bool value = (N % Align) == 0;
};

// 向量类，支持表达式模板
template<typename T, size_t N>
class Vector : public Expression<Vector<T, N>> {
public:
    using value_type = T;

    // 默认构造函数
    Vector() : data(new T[N]()) {
    }

    // 值初始化
    explicit Vector(T val) : data(new T[N]) {
        for (size_t i = 0; i < N; ++i) {
            data[i] = val;
        }
    }

    // 初始化列表构造
    Vector(std::initializer_list<T> init) : data(new T[N]()) {
        size_t i = 0;
        for (auto it = init.begin(); it != init.end() && i < N; ++it, ++i) {
            data[i] = *it;
        }
    }

    // 拷贝构造函数
    Vector(const Vector& other) : data(new T[N]) {
        for (size_t i = 0; i < N; ++i) {
            data[i] = other.data[i];
        }
    }

    // 移动构造函数
    Vector(Vector&& other) noexcept : data(other.data) {
        other.data = nullptr;
    }

    // 析构函数
    ~Vector() {
        delete[] data;
    }

    // 表达式模板赋值
    template<typename Expr>
    Vector& operator=(const Expression<Expr>& expr) {
        const auto& e = expr();

        constexpr size_t UnrollFactor = (sizeof(T) == 8) ? 8 : ((sizeof(T) == 4) ? 4 : 1);
        constexpr size_t BlockSize = N / UnrollFactor;
        constexpr size_t Remainder = N % UnrollFactor;

        // 循环展开 + 预取
        for (size_t i = 0; i < BlockSize * UnrollFactor; i += UnrollFactor) {
            __builtin_prefetch(&data[i + UnrollFactor * 2], 1, 1);
            
            if (UnrollFactor >= 8) {
                data[i] = e[i];
                data[i + 1] = e[i + 1];
                data[i + 2] = e[i + 2];
                data[i + 3] = e[i + 3];
                data[i + 4] = e[i + 4];
                data[i + 5] = e[i + 5];
                data[i + 6] = e[i + 6];
                data[i + 7] = e[i + 7];
            } else if (UnrollFactor >= 4) {
                data[i] = e[i];
                data[i + 1] = e[i + 1];
                data[i + 2] = e[i + 2];
                data[i + 3] = e[i + 3];
            } else {
                data[i] = e[i];
            }
        }

        // 处理剩余元素
        for (size_t i = BlockSize * UnrollFactor; i < N; ++i) {
            data[i] = e[i];
        }

        return *this;
    }

    // 拷贝赋值
    Vector& operator=(const Vector& other) {
        if (this != &other) {
            for (size_t i = 0; i < N; ++i) {
                data[i] = other.data[i];
            }
        }
        return *this;
    }

    // 移动赋值
    Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            delete[] data;
            data = other.data;
            other.data = nullptr;
        }
        return *this;
    }

    // 元素访问
    T& operator[](size_t i) {
        return data[i];
    }

    const T& operator[](size_t i) const {
        return data[i];
    }

    // 获取大小
    size_t size() const noexcept {
        return N;
    }

    // 切片操作
    template<size_t Start, size_t Len>
    Vector<T, Len> slice() const {
        static_assert(Start + Len <= N, "Slice out of bounds");
        Vector<T, Len> res(T(0));
        for (size_t i = 0; i < Len; ++i) {
            res[i] = data[Start + i];
        }
        return res;
    }

    // Reduce操作
    T reduce_sum() const {
        T sum = T(0);
        for (size_t i = 0; i < N; ++i) {
            sum += data[i];
        }
        return sum;
    }

    T reduce_max() const {
        T max_val = data[0];
        for (size_t i = 1; i < N; ++i) {
            if (data[i] > max_val) {
                max_val = data[i];
            }
        }
        return max_val;
    }

    T reduce_min() const {
        T min_val = data[0];
        for (size_t i = 1; i < N; ++i) {
            if (data[i] < min_val) {
                min_val = data[i];
            }
        }
        return min_val;
    }

    // 广播操作：标量转向量
    static Vector<T, N> broadcast(T val) {
        return Vector<T, N>(val);
    }

private:
    T* data;
};

// Matrix类实现
template<typename T>
class Matrix : public Expression<Matrix<T>> {
private:
    size_t rows_;
    size_t cols_;
    T* data_;

public:
    // 构造函数
    Matrix(size_t rows, size_t cols) : rows_(rows), cols_(cols) {
        data_ = new T[rows * cols];
    }

    // 析构函数
    ~Matrix() {
        delete[] data_;
    }

    // 禁止拷贝
    Matrix(const Matrix&) = delete;
    Matrix& operator=(const Matrix&) = delete;

    // 移动构造
    Matrix(Matrix&& other) noexcept : rows_(other.rows_), cols_(other.cols_), data_(other.data_) {
        other.data_ = nullptr;
    }

    // 元素访问
    T& operator()(size_t i, size_t j) noexcept {
        return data_[i * cols_ + j];
    }

    const T& operator()(size_t i, size_t j) const noexcept {
        return data_[i * cols_ + j];
    }

    // 行切片
    Vector<T, -1> row(size_t i) const noexcept {
        // 为简化实现，返回一个轻量级视图
        Vector<T, -1> res;
        // 实际实现中应该返回视图而不是拷贝
        for (size_t j = 0; j < cols_; ++j) {
            res[j] = data_[i * cols_ + j];
        }
        return res;
    }

    // 获取大小
    size_t rows() const noexcept { return rows_; }
    size_t cols() const noexcept { return cols_; }
    size_t size() const noexcept { return rows_ * cols_; }
};

// 特化变长向量支持
template<typename T>
class Vector<T, static_cast<size_t>(-1)> : public Expression<Vector<T, static_cast<size_t>(-1)>> {
private:
    T* data_;
    size_t size_;

public:
    Vector(size_t size) : size_(size) {
        data_ = new T[size];
    }

    ~Vector() {
        delete[] data_;
    }

    T& operator[](size_t i) noexcept { return data_[i]; }
    const T& operator[](size_t i) const noexcept { return data_[i]; }
    size_t size() const noexcept { return size_; }
};
