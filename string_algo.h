#pragma once

#include <vector>
#include <string>
#include <utility>
#include <iterator>
#include <algorithm>

namespace string_algo {

// Manacher算法，返回奇偶回文半径数组
// 回文半径定义为：以i为中心，能扩展的最大回文长度的一半（向下取整）
// 例如 "abcba" 中心c的半径是2
std::vector<int> manacher(const std::string &s);

// Z函数，返回Z数组
// Z[i]表示s[0..]与s[i..]的最长公共前缀长度
std::vector<int> z_function(const std::string &s);

// KMP的π数组（部分匹配表），增强版支持最小周期计算
std::vector<int> kmp_pi(const std::string &s);

// 计算字符串的最小周期
// 例如 "ababab" 的最小周期是2
inline int minimal_period(const std::string &s, const std::vector<int> &pi) {
    int n = s.size();
    int period = n - pi.back();
    return (n % period == 0) ? period : n;
}

// 生成所有回文子串的生成器模板
template<typename Func>
void all_palindromic_substrings(const std::string &s, Func callback) {
    int n = s.size();
    auto p = manacher(s);
    
    // 处理奇数长度回文
    for (int i = 0; i < n; ++i) {
        int r = p[i * 2];
        for (int d = 0; d <= r; ++d) {
            callback(std::make_pair(i - d, i + d));
        }
    }
    
    // 处理偶数长度回文
    for (int i = 0; i < n - 1; ++i) {
        int r = p[i * 2 + 1];
        for (int d = 0; d <= r; ++d) {
            callback(std::make_pair(i - d + 1, i + d));
        }
    }
}

// 最小字典序旋转，返回旋转起点下标
int minimal_rotation(const std::string &s);

} // namespace string_algo