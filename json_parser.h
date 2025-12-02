#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>
#include <array>
#include <type_traits>
#include <cstring>
#include <tuple>

// C++14 compatible string_view implementation
namespace detail {
    struct string_view {
        const char* data_;
        size_t size_;

        constexpr string_view() noexcept : data_(nullptr), size_(0) {}
        constexpr string_view(const char* str) noexcept : data_(str), size_(constexpr_strlen(str)) {}
        constexpr string_view(const char* str, size_t len) noexcept : data_(str), size_(len) {}

        constexpr const char* data() const noexcept { return data_; }
        constexpr size_t size() const noexcept { return size_; }
        constexpr bool empty() const noexcept { return size_ == 0; }

        constexpr const char& operator[](size_t pos) const noexcept { return data_[pos]; }

        std::string to_string() const noexcept {
            return std::string(data_, size_);
        }

        constexpr int compare(string_view other) const noexcept {
            size_t min_len = size_ < other.size_ ? size_ : other.size_;
            for (size_t i = 0; i < min_len; ++i) {
                if (data_[i] != other.data_[i]) {
                    return data_[i] < other.data_[i] ? -1 : 1;
                }
            }
            if (size_ < other.size_) return -1;
            if (size_ > other.size_) return 1;
            return 0;
        }
    };

    constexpr bool operator==(string_view a, string_view b) noexcept {
        return a.compare(b) == 0;
    }

    constexpr bool operator!=(string_view a, string_view b) noexcept {
        return a.compare(b) != 0;
    }
}

namespace Json {
    using detail::string_view;
    namespace detail {
        // 编译期字符串工具
        template <char... Cs>
        struct StringLiteral {
            static constexpr const char value[] = {Cs...};
            static constexpr std::size_t size = sizeof...(Cs);
            static constexpr string_view view() noexcept {
                return {value, size};
            }
        };

        template <char... Cs>
        constexpr const char StringLiteral<Cs...>::value[];

        using StringLiteral = ::Json::detail::StringLiteral;

        // 编译期字符串长度计算
        constexpr std::size_t detail_strlen(const char* s) noexcept {
            std::size_t i =0;
            while(s[i] != '\0') i++;
            return i;
        }

        // 编译期字符串长度计算
        constexpr std::size_t constexpr_strlen(const char* str) noexcept {
            std::size_t len = 0;
            while (str[len] != '\0') {
                len++;
            }
            return len;
        }

        // 编译期trim空白字符
        constexpr std::size_t skip_whitespace(const char* str, std::size_t pos) noexcept {
              while (pos < constexpr_strlen(str) && (str[pos] == ' ' || str[pos] == '\n' || str[pos] == '\t' || str[pos] == '\r')) {
                   pos++;
                  }
            return pos;
        }

        // JSON类型枚举
        enum class JsonType {
            Null,
            Boolean,
            Number,
            String,
            Array,
            Object
        };

        // 编译期错误信息
        template <const char* Msg, typename T>
        struct CompileError {
            static_assert(false, Msg);
            static constexpr T value = T{};
        };

        // Optional包装器
        template <typename T>
        struct optional {
            bool has_value = false;
            T value{};

            constexpr optional() noexcept = default;
            constexpr optional(T v) noexcept : has_value(true), value(v) {}

            constexpr const T& operator*() const noexcept { return value; }
            constexpr const T* operator->() const noexcept { return &value; }
            constexpr explicit operator bool() const noexcept { return has_value; }
        };

        // 主Parser模板，默认不支持
        template <typename T, typename Enable = void>
        struct Parser;

        // JSON值基类
        struct JsonValue {
            JsonType type;
            constexpr JsonValue(JsonType t) : type(t) {}
        };

        // JSON字符串
        struct JsonString : JsonValue {
            string_view str;
            constexpr JsonString(string_view s) : JsonValue(JsonType::String), str(s) {}
        };

        // JSON数字

        // JSON布尔值
        struct JsonBoolean : JsonValue {
            bool value;
            constexpr JsonBoolean(bool v) : JsonValue(JsonType::Boolean), value(v) {}
        };

        // JSON空值
        struct JsonNull : JsonValue {
            constexpr JsonNull() : JsonValue(JsonType::Null) {}
        };
        struct JsonNumber : JsonValue {
            long long int_value;
            double float_value;
            bool is_float;

            constexpr JsonNumber(long long v) : JsonValue(JsonType::Number), int_value(v), float_value(0.0), is_float(false) {}
            constexpr JsonNumber(double v) : JsonValue(JsonType::Number), int_value(0), float_value(v), is_float(true) {}
        };

        // JSON布尔值
        struct JsonBoolean : JsonValue {
            bool value;
            constexpr JsonBoolean(bool v) : JsonValue(JsonType::Boolean), value(v) {}
        };

        // JSON Null
        struct JsonNull : JsonValue {
            constexpr JsonNull() : JsonValue(JsonType::Null) {}
        };

        // 编译期JSON解析器核心
        template <typename T>
        struct Parser;

        // 解析null
        template <>
        struct Parser<JsonNull> {
            static constexpr JsonNull parse(const char* str, std::size_t& pos) noexcept {
                pos = skip_whitespace(str, pos);
            auto str_len = constexpr_strlen(str);
            if (pos +3 < str_len && str[pos] == 'n' && str[pos+1] == 'u' && str[pos+2] == 'l' && str[pos+3] == 'l') {
                    pos +=4;
                    return {};
                }
                return CompileError<(const char*)"Expected null", JsonNull>::value;
            }
        };

        // 解析布尔值
        template <>
        struct Parser<JsonBoolean> {
            static constexpr JsonBoolean parse(const char* str, std::size_t& pos) noexcept {
                pos = skip_whitespace(str, pos);
                auto str_len = constexpr_strlen(str);
                if (pos +3 < str_len && str[pos] == 't' && str[pos+1] == 'r' && str[pos+2] == 'u' && str[pos+3] == 'e') {
                    pos +=4;
                    return JsonBoolean(true);
                } else if (pos +4 < str_len && str[pos] == 'f' && str[pos+1] == 'a' && str[pos+2] == 'l' && str[pos+3] == 's' && str[pos+4] == 'e') {
                    pos +=5;
                    return JsonBoolean(false);
                }
                return CompileError<(const char*)"Expected boolean", JsonBoolean>::value;
            }
        };

        // 解析bool类型
        template <>
        struct Parser<bool> {
            static constexpr bool parse(const char* str, std::size_t& pos) noexcept {
                pos = skip_whitespace(str, pos);
                auto str_len = constexpr_strlen(str);
                if (pos +3 < str_len && str[pos] == 't' && str[pos+1] == 'r' && str[pos+2] == 'u' && str[pos+3] == 'e') {
                    pos +=4;
                    return true;
                } else if (pos +4 < str_len && str[pos] == 'f' && str[pos+1] == 'a' && str[pos+2] == 'l' && str[pos+3] == 's' && str[pos+4] == 'e') {
                    pos +=5;
                    return false;
                }
                return CompileError<(const char*)"Expected boolean", bool>::value;
            }
        }; 

        // 解析int类型
        template <>
        struct Parser<int> {
            static constexpr int parse(const char* str, std::size_t& pos) noexcept {
                pos = skip_whitespace(str, pos);
                auto str_len = constexpr_strlen(str);
                bool neg = false;
                if (str[pos] == '-') {
                    neg = true;
                    pos++;
                }
                
                if (pos >= str_len || str[pos] < '0' || str[pos] > '9') {
                    return CompileError<(const char*)"Expected digit", int>::value;
                }
                
                int val = 0;
                while (pos < str_len && str[pos] >= '0' && str[pos] <= '9') {
                    val = val * 10 + (str[pos] - '0');
                    pos++;
                }
                
                return neg ? -val : val;
            }
        }; 

        // 解析字符串
        template <>
        struct Parser<JsonString> {
            static constexpr JsonString parse(const char* str, std::size_t& pos) noexcept {
                pos = skip_whitespace(str, pos);
                auto str_len = constexpr_strlen(str);
                if (str[pos] != '"') {
                    return CompileError<(const char*)"Expected string quote", JsonString>::value;
                }
                pos++;
                std::size_t start = pos;
                while (pos < str_len && str[pos] != '"') {
                    if (str[pos] == '\\') pos++;
                    pos++;
                }
                if (pos >= str_len) {
                    return CompileError<(const char*)"Unterminated string", JsonString>::value;
                }
                auto view = string_view(str + start, pos - start);
                pos++;
                return JsonString(view);
            }
        };

        // 解析数字
        template <>
        struct Parser<JsonNumber> {
            static constexpr JsonNumber parse(const char* str, std::size_t& pos) noexcept {
                pos = skip_whitespace(str, pos);
                std::size_t start = pos;
                bool is_float = false;
                
                auto str_len = constexpr_strlen(str);
                if (str[pos] == '-') pos++;
                if (str[pos] >= '0' && str[pos] <= '9') {
                    while (pos < str_len && str[pos] >= '0' && str[pos] <= '9') pos++;
                } else {
                    return CompileError<(const char*)"Expected digit", JsonNumber>::value;
                }
                
                bool is_float = false;
                if (pos < str_len && str[pos] == '.') {
                    is_float = true;
                    pos++;
                    while (pos < str_len && str[pos] >= '0' && str[pos] <= '9') pos++;
                }
                
                if (pos < str_len && (str[pos] == 'e' || str[pos] == 'E')) {
                    is_float = true;
                    pos++;
                    if (str[pos] == '+' || str[pos] == '-') pos++;
                    while (pos < strlen(str) && str[pos] >= '0' && str[pos] <= '9') pos++;
                }
                
                auto num_str = string_view(str + start, pos - start);
                
                if (is_float) {
                    double val = 0.0;
                    double frac = 0.1;
                    bool neg = false;
                    std::size_t i = 0;
                    if (num_str[i] == '-') { neg = true; i++; }
                    
                    while (i < num_str.size() && num_str[i] != '.' && num_str[i] != 'e' && num_str[i] != 'E') {
                        val = val *10 + (num_str[i] - '0');
                        i++;
                    }
                    
                    if (i < num_str.size() && num_str[i] == '.') {
                        i++;
                        while (i < num_str.size() && num_str[i] != 'e' && num_str[i] != 'E') {
                            val += (num_str[i] - '0') * frac;
                            frac *=0.1;
                            i++;
                        }
                    }
                    
                    if (i < num_str.size() && (num_str[i] == 'e' || num_str[i] == 'E')) {
                        i++;
                        bool exp_neg = false;
                        int exp =0;
                        if (num_str[i] == '-') { exp_neg = true; i++; }
                        else if (num_str[i] == '+') { i++; }
                        
                        while (i < num_str.size()) {
                            exp = exp *10 + (num_str[i] - '0');
                            i++;
                        }
                        
                        double exp_val = 1.0;
                        while (exp--) exp_val *=10;
                        val = exp_neg ? val / exp_val : val * exp_val;
                    }
                    
                    return JsonNumber(neg ? -val : val);
                } else {
                    long long val =0;
                    bool neg = false;
                    std::size_t i=0;
                    if (num_str[i] == '-') { neg = true; i++; }
                    while (i < num_str.size()) {
                        val = val *10 + (num_str[i] - '0');
                        i++;
                    }
                    return JsonNumber(neg ? -val : val);
                }
            }
        };

        // 字段描述结构体
        template <typename NameSL, typename T, T MemberPtr>
        struct FieldDesc {
            static constexpr NameSL name = NameSL{};
            using Type = T;
            static constexpr T member_ptr = MemberPtr;
        };

        // 辅助宏定义
        #define JSON_FIELD(StructType, MemberName, FieldType) \
        FieldDesc<#MemberName##_sl, FieldType, &StructType::MemberName>{}

        // 结构体反射基础
        template <typename T, typename = void>
        struct has_fields : std::false_type {};

        template <typename T>
        struct has_fields<T, decltype((void)T::fields, void())> : std::true_type {};

        // 字段匹配辅助函数
        template <typename T, typename FieldDescT>
        constexpr void match_and_set(T& result, string_view name, const char* str, std::size_t& pos, bool& matched, FieldDescT) noexcept {
            if (name == FieldDescT::name.view()) {
                result.*FieldDescT::member_ptr = Parser<typename FieldDescT::Type>::parse(str, pos);
                matched = true;
            }
        }

        // 字段解析辅助函数
        template <typename FieldDescT>
        constexpr typename FieldDescT::Type parse_field(const char* str, std::size_t& pos, string_view target_name) noexcept {
             auto str_len = constexpr_strlen(str);
             
             while (true) {
                pos = skip_whitespace(str, pos);
              
                // 检查是否到达字符串末尾
                if (pos >= str_len || str[pos] == '}') {
                     return CompileError<(const char*)"Field not found", typename FieldDescT::Type>::value;
                }
                
                // 查找匹配的字段
                auto name = Parser<JsonString>::parse(str, pos).str;
                pos = skip_whitespace(str, pos);
                
                if (name == FieldDescT::name.view()) {
                    if (str[pos] != ':') {
                        return CompileError<(const char*)"Expected ':' after field name", typename FieldDescT::Type>::value;
                    }
                    pos++;
                    auto value = Parser<typename FieldDescT::Type>::parse(str, pos);
                    pos = skip_whitespace(str, pos);
                    
                    // 跳过逗号（如果存在）
                    if (pos < str_len && str[pos] == ',') {
                        pos++;
                    }
                    
                    return value;
                } else {
                // 跳过不匹配的字段
                if (str[pos] != ':') {
                    return CompileError<(const char*)"Expected ':' after field name", typename FieldDescT::Type>::value;
                }
                pos++;
                
                // 跳过字段值
                 while (pos < str_len && str[pos] != ',' && str[pos] != '}') {
                    if (str[pos] == '"') {
                        // 跳过字符串
                        pos++;
                        while (pos < str_len && str[pos] != '"') {
                            if (str[pos] == '\\') pos++;
                            pos++;
                        }
                        pos++;
                    } else if (str[pos] == '{' || str[pos] == '[') {
                        // 跳过嵌套对象/数组
                        char end_char = (str[pos] == '{') ? '}' : ']';
                        int depth = 1;
                        pos++;
                        while (pos < str_len && depth > 0) {
                            if (str[pos] == str[pos-1]) depth++;
                            if (str[pos] == end_char) depth--;
                            pos++;
                        }
                    } else {
                        // 跳过普通值
                        pos++;
                    }
                }
                
                if (pos < str_len && str[pos] == ',') {
                    pos++;
                }
                
                // 跳过不匹配的字段后继续循环查找
                continue;
            }
        }

        // 解析对象到结构体（使用聚合初始化）
        template <typename T, typename... FieldDescs>
        constexpr T parse_object(const char* str, std::size_t& pos, std::tuple<FieldDescs...>) noexcept {
            auto str_len = constexpr_strlen(str);
            pos = skip_whitespace(str, pos);
            if (str[pos] != '{') {
                return CompileError<(const char*)"Expected object start '{'", T>::value;
            }
            pos++;
            pos = skip_whitespace(str, pos);
            
            // 解析每个字段，按顺序聚合初始化结构体，直接使用pos跟踪解析位置
            T result{
                parse_field<FieldDescs>(str, pos, FieldDescs::name.view())...
            };
            
            pos = skip_whitespace(str, pos);
            if (pos >= str_len || str[pos] != '}') {
                return CompileError<(const char*)"Expected object end '}'", T>::value;
            }
            pos++;
            
            return result;
        }

        // 解析单个数组元素
        template <typename T>
        constexpr T parse_single_array_element(const char* str, std::size_t& pos) noexcept {
            auto str_len = constexpr_strlen(str);
            auto value = Parser<T>::parse(str, pos);
            pos = skip_whitespace(str, pos);
            if (pos < str_len && str[pos] == ',') {
                pos++;
                pos = skip_whitespace(str, pos);
            }
            return value;
        }

        // 解析数组（兼容C++14 constexpr）
        template <typename T, std::size_t N, std::size_t... Indices>
        constexpr std::array<T, N> parse_array_impl(const char* str, std::size_t& pos, std::index_sequence<Indices...>) noexcept {
            return {
                parse_single_array_element<T>(str, pos)...
            };
        }

        // 解析数组对外接口
        template <typename T, std::size_t N>
        constexpr std::array<T, N> parse_array(const char* str, std::size_t& pos) noexcept {
            auto str_len = constexpr_strlen(str);
            pos = skip_whitespace(str, pos);
            if (str[pos] != '[') {
                return CompileError<(const char*)"Expected array start '['", std::array<T, N>>::value;
            }
            pos++;
            pos = skip_whitespace(str, pos);
            
            auto arr = parse_array_impl<T, N>(str, pos, std::make_index_sequence<N>{});
            
            pos = skip_whitespace(str, pos);
            if (pos >= str_len || str[pos] != ']') {
                return CompileError<(const char*)"Expected array end ']'", std::array<T, N>>::value;
            }
            pos++;
            return arr;
        }

        // 基础类型解析特化
        template <>
        struct Parser<bool> {
            static constexpr bool parse(const char* str, std::size_t& pos) noexcept {
                return Parser<JsonBoolean>::parse(str, pos).value;
            }
        };

        template <>
        struct Parser<long long> {
            static constexpr long long parse(const char* str, std::size_t& pos) noexcept {
                auto num = Parser<JsonNumber>::parse(str, pos);
                if (num.is_float) {
                    return CompileError<(const char*)"Expected integer number">::value;
                }
                return num.int_value;
            }
        };

        template <>
        struct Parser<int> {
            static constexpr int parse(const char* str, std::size_t& pos) noexcept {
                auto val = Parser<long long>::parse(str, pos);
                if (val < INT32_MIN || val > INT32_MAX) {
                    return CompileError<(const char*)"Integer out of int range">::value;
                }
                return static_cast<int>(val);
            }
        };

        template <>
        struct Parser<double> {
            static constexpr double parse(const char* str, std::size_t& pos) noexcept {
                return Parser<JsonNumber>::parse(str, pos).float_value;
            }
        };

        template <>
        struct Parser<string_view> {
            static constexpr string_view parse(const char* str, std::size_t& pos) noexcept {
                return Parser<JsonString>::parse(str, pos).str;
            }
        };

        template <typename T>
        struct Parser<optional<T>> {
            static constexpr optional<T> parse(const char* str, std::size_t& pos) noexcept {
                // 在constexpr函数中不能使用try/catch，直接解析
                auto value = Parser<T>::parse(str, pos);
                return optional<T>(value);
            }
        };

        template <typename T, std::size_t N>
        struct Parser<std::array<T, N>> {
            static constexpr std::array<T, N> parse(const char* str, std::size_t& pos) noexcept {
                return parse_array<T, N>(str, pos);
            }
        };

        // 结构体解析特化，仅适用于具有fields成员的结构体
        template <typename T>
        struct Parser<T, typename std::enable_if<has_fields<T>::value>::type> {
            static constexpr T parse(const char* str, std::size_t& pos) noexcept {
                return parse_object<T>(str, pos, T::fields);
            }
        };
    }

    template <typename T, std::size_t N>
    constexpr T parse(const char (&JsonStr)[N]) noexcept {
        std::size_t pos =0;
        return detail::Parser<T>::parse(JsonStr, pos);
    }

} // namespace detail

    template <typename T, std::size_t N>
    constexpr T parse(const char (&JsonStr)[N]) noexcept {
        std::size_t pos =0;
        return detail::Parser<T>::parse(JsonStr, pos);
    }

    using detail::optional;
    using detail::Parser;
} // namespace Json

#define JSON_STRUCT(StructName, ...) \
struct StructName { \
    static constexpr auto fields = std::make_tuple(__VA_ARGS__); \
    __VA_ARGS__; \
}

#include "string_literal.h"