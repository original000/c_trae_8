#pragma once

#include <cstddef>

namespace Json {
namespace detail {
    template <std::size_t N>
    struct StringLiteral {
        const char* data;
        std::size_t size;

        constexpr StringLiteral(const char* str, std::size_t n) : data(str), size(n) {}

        constexpr const char* view() const noexcept { return data; }
        constexpr std::size_t length() const noexcept { return size; }
    };
}
}

template <std::size_t N>
constexpr Json::detail::StringLiteral<N> operator""_sl(const char (&str)[N]) noexcept {
    return Json::detail::StringLiteral<N>(str, N - 1);
}