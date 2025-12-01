#pragma once

#include <cmath>
#include <type_traits>

// 基础表达式模板基类
template<typename Expr>
struct Expression {
    constexpr const Expr& operator()() const {
        return *static_cast<const Expr*>(this);
    }
};

// 一元操作类型枚举
enum class UnaryOp {
    Sin,
    Cos,
    Tan,
    Exp,
    Log,
    Sqrt,
    Pow2,
    Abs
};

// 一元表达式模板
template<typename Expr, UnaryOp Op>
struct UnaryExpression : public Expression<UnaryExpression<Expr, Op>> {
    using value_type = typename Expr::value_type;
    const Expr& expr;

    explicit constexpr UnaryExpression(const Expr& e) : expr(e) {}

    constexpr value_type operator[](size_t i) const {
        value_type val = expr[i];
        switch (Op) {
            case UnaryOp::Sin: return static_cast<value_type>(sin(val));
            case UnaryOp::Cos: return static_cast<value_type>(cos(val));
            case UnaryOp::Tan: return static_cast<value_type>(tan(val));
            case UnaryOp::Exp: return static_cast<value_type>(exp(val));
            case UnaryOp::Log: return static_cast<value_type>(log(val));
            case UnaryOp::Sqrt: return static_cast<value_type>(sqrt(val));
            case UnaryOp::Pow2: return val * val;
            case UnaryOp::Abs: return static_cast<value_type>(fabs(val));
            default: return val;
        }
    }

    constexpr size_t size() const { return expr.size(); }
};

// 二元操作类型枚举
enum class BinaryOp {
    Add,
    Sub,
    Mul,
    Div
};

// 二元表达式模板
template<typename Lhs, typename Rhs, BinaryOp Op>
struct BinaryExpression : public Expression<BinaryExpression<Lhs, Rhs, Op>> {
    using value_type = typename Lhs::value_type;
    const Lhs& lhs;
    const Rhs& rhs;

    constexpr BinaryExpression(const Lhs& l, const Rhs& r) : lhs(l), rhs(r) {}

    constexpr value_type operator[](size_t i) const {
        typename Lhs::value_type l_val = lhs[i];
        typename Rhs::value_type r_val = rhs[i];
        switch (Op) {
            case BinaryOp::Add: return static_cast<value_type>(l_val + r_val);
            case BinaryOp::Sub: return static_cast<value_type>(l_val - r_val);
            case BinaryOp::Mul: return static_cast<value_type>(l_val * r_val);
            case BinaryOp::Div: return static_cast<value_type>(l_val / r_val);
            default: return static_cast<value_type>(l_val);
        }
    }

    constexpr size_t size() const { return lhs.size(); }
};

// 标量广播表达式
template<typename T>
struct ScalarExpression : public Expression<ScalarExpression<T>> {
    using value_type = T;
    T val;

    explicit constexpr ScalarExpression(T v) : val(v) {}

    constexpr T operator[](size_t) const { return val; }

    constexpr size_t size() const { return static_cast<size_t>(-1); }
};

// 二元操作重载
template<typename Lhs, typename Rhs>
constexpr auto operator+(const Expression<Lhs>& lhs, const Expression<Rhs>& rhs) {
    return BinaryExpression<Lhs, Rhs, BinaryOp::Add>(lhs(), rhs());
}

template<typename Lhs, typename T>
constexpr auto operator+(const Expression<Lhs>& lhs, T rhs)
    -> typename std::enable_if<std::is_arithmetic<T>::value, BinaryExpression<Lhs, ScalarExpression<T>, BinaryOp::Add>>::type {
    return BinaryExpression<Lhs, ScalarExpression<T>, BinaryOp::Add>(lhs(), ScalarExpression<T>(rhs));
}

template<typename T, typename Rhs>
constexpr auto operator+(T lhs, const Expression<Rhs>& rhs)
    -> typename std::enable_if<std::is_arithmetic<T>::value, BinaryExpression<ScalarExpression<T>, Rhs, BinaryOp::Add>>::type {
    return BinaryExpression<ScalarExpression<T>, Rhs, BinaryOp::Add>(ScalarExpression<T>(lhs), rhs());
}

template<typename Lhs, typename Rhs>
constexpr auto operator-(const Expression<Lhs>& lhs, const Expression<Rhs>& rhs) {
    return BinaryExpression<Lhs, Rhs, BinaryOp::Sub>(lhs(), rhs());
}

template<typename Lhs, typename T>
constexpr auto operator-(const Expression<Lhs>& lhs, T rhs)
    -> typename std::enable_if<std::is_arithmetic<T>::value, BinaryExpression<Lhs, ScalarExpression<T>, BinaryOp::Sub>>::type {
    return BinaryExpression<Lhs, ScalarExpression<T>, BinaryOp::Sub>(lhs(), ScalarExpression<T>(rhs));
}

template<typename T, typename Rhs>
constexpr auto operator-(T lhs, const Expression<Rhs>& rhs)
    -> typename std::enable_if<std::is_arithmetic<T>::value, BinaryExpression<ScalarExpression<T>, Rhs, BinaryOp::Sub>>::type {
    return BinaryExpression<ScalarExpression<T>, Rhs, BinaryOp::Sub>(ScalarExpression<T>(lhs), rhs());
}

template<typename Lhs, typename Rhs>
constexpr auto operator*(const Expression<Lhs>& lhs, const Expression<Rhs>& rhs) {
    return BinaryExpression<Lhs, Rhs, BinaryOp::Mul>(lhs(), rhs());
}

template<typename Lhs, typename T>
constexpr auto operator*(const Expression<Lhs>& lhs, T rhs)
    -> typename std::enable_if<std::is_arithmetic<T>::value, BinaryExpression<Lhs, ScalarExpression<T>, BinaryOp::Mul>>::type {
    return BinaryExpression<Lhs, ScalarExpression<T>, BinaryOp::Mul>(lhs(), ScalarExpression<T>(rhs));
}

template<typename T, typename Rhs>
constexpr auto operator*(T lhs, const Expression<Rhs>& rhs)
    -> typename std::enable_if<std::is_arithmetic<T>::value, BinaryExpression<ScalarExpression<T>, Rhs, BinaryOp::Mul>>::type {
    return BinaryExpression<ScalarExpression<T>, Rhs, BinaryOp::Mul>(ScalarExpression<T>(lhs), rhs());
}

template<typename Lhs, typename Rhs>
constexpr auto operator/(const Expression<Lhs>& lhs, const Expression<Rhs>& rhs) {
    return BinaryExpression<Lhs, Rhs, BinaryOp::Div>(lhs(), rhs());
}

template<typename Lhs, typename T>
constexpr auto operator/(const Expression<Lhs>& lhs, T rhs)
    -> typename std::enable_if<std::is_arithmetic<T>::value, BinaryExpression<Lhs, ScalarExpression<T>, BinaryOp::Div>>::type {
    return BinaryExpression<Lhs, ScalarExpression<T>, BinaryOp::Div>(lhs(), ScalarExpression<T>(rhs));
}

template<typename T, typename Rhs>
constexpr auto operator/(T lhs, const Expression<Rhs>& rhs)
    -> typename std::enable_if<std::is_arithmetic<T>::value, BinaryExpression<ScalarExpression<T>, Rhs, BinaryOp::Div>>::type {
    return BinaryExpression<ScalarExpression<T>, Rhs, BinaryOp::Div>(ScalarExpression<T>(lhs), rhs());
}

// 数学函数重载
template<typename Expr>
constexpr auto sin(const Expression<Expr>& expr) {
    return UnaryExpression<Expr, UnaryOp::Sin>(expr());
}

template<typename Expr>
constexpr auto cos(const Expression<Expr>& expr) {
    return UnaryExpression<Expr, UnaryOp::Cos>(expr());
}

template<typename Expr>
constexpr auto tan(const Expression<Expr>& expr) {
    return UnaryExpression<Expr, UnaryOp::Tan>(expr());
}

template<typename Expr>
constexpr auto exp(const Expression<Expr>& expr) {
    return UnaryExpression<Expr, UnaryOp::Exp>(expr());
}

template<typename Expr>
constexpr auto log(const Expression<Expr>& expr) {
    return UnaryExpression<Expr, UnaryOp::Log>(expr());
}

template<typename Expr>
constexpr auto sqrt(const Expression<Expr>& expr) {
    return UnaryExpression<Expr, UnaryOp::Sqrt>(expr());
}

template<typename Expr>
constexpr auto abs(const Expression<Expr>& expr) {
    return UnaryExpression<Expr, UnaryOp::Abs>(expr());
}

template<typename Expr>
constexpr auto pow2(const Expression<Expr>& expr) {
    return UnaryExpression<Expr, UnaryOp::Pow2>(expr());
}