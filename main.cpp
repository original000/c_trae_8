#include "string_algo.h"
#include <iostream>
#include <set>
#include <iomanip>
#include <clocale>

using namespace string_algo;

void example1();
void example2();
void test_all_palindromes();

int main() {
    setlocale(LC_ALL, "zh_CN.UTF-8");
    std::cout << "===== C++14 字符串算法全家桶演示 =====\n" << std::endl;
    
    example1();
    example2();
    test_all_palindromes();
    
    std::cout << "\n===== 所有测试完成 =====" << std::endl;
    return 0;
}

void example1() {
    std::cout << "=== 例题1：最长回文子串与最小周期 ===\n" << std::endl;
    
    std::string s1 = "babadabab";
    int n = s1.size();
    std::cout << "输入字符串: " << s1 << std::endl;
    
    // Manacher找最长回文子串
    auto p = manacher(s1);
    int max_len = 0;
    int center = 0;
    bool is_odd = true;
    
    for (int i = 0; i < p.size(); ++i) {
        int current_len;
        if (i % 2 == 0) {
            current_len = p[i] * 2 + 1;
        } else {
            current_len = p[i] * 2 + 2;
        }
        
        if (current_len > max_len) {
            max_len = current_len;
            center = i;
            is_odd = (i % 2 == 0);
        }
    }
    
    int start = 0, end = 0;
    if (max_len > 0) {
        if (is_odd) {
            int pos = center / 2;
            start = std::max(0, pos - max_len / 2);
            end = std::min(n - 1, pos + max_len / 2);
        } else {
            int pos = center / 2;
            start = std::max(0, pos - max_len / 2 + 1);
            end = std::min(n - 1, pos + max_len / 2);
        }
    }
    
    std::string longest_pal = s1.substr(start, end - start + 1);
    std::cout << "最长回文子串: " << longest_pal << " (长度: " << max_len << ")" << std::endl;
    
    // KMP计算最小周期
    std::string s2 = "abababab";
    std::cout << "\n字符串: " << s2 << std::endl;
    auto pi = kmp_pi(s2);
    int period = minimal_period(s2, pi);
    std::cout << "最小周期长度: " << period << std::endl;
    std::cout << "周期单元: " << s2.substr(0, period) << std::endl;
}

void example2() {
    std::cout << "\n=== 例题2：Z函数匹配与最小字典序旋转 ===\n" << std::endl;
    
    // Z函数字符串匹配
    std::string text = "ababcabcabx";
    std::string pattern = "abc";
    std::cout << "文本串: " << text << std::endl;
    std::cout << "模式串: " << pattern << std::endl;
    
    std::string concat = pattern + "#" + text;
    auto z = z_function(concat);
    std::vector<int> matches;
    
    for (int i = pattern.size() + 1; i < concat.size(); ++i) {
        if (z[i] == pattern.size()) {
            matches.push_back(i - pattern.size() - 1);
        }
    }
    
    std::cout << "匹配位置: ";
    for (size_t i = 0; i < matches.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << matches[i];
    }
    std::cout << std::endl;
    
    // 最小字典序旋转
    std::string s3 = "cbaab";
    std::cout << "\n原始字符串: " << s3 << std::endl;
    int rot = minimal_rotation(s3);
    std::string min_rot = s3.substr(rot) + s3.substr(0, rot);
    std::cout << "最小字典序旋转: " << min_rot << " (旋转起点: " << rot << ")" << std::endl;
}

void test_all_palindromes() {
    std::cout << "\n=== 测试：生成所有回文子串 ===\n" << std::endl;
    std::string s = "aaa";
    
    std::set<std::string> palindromes;
    all_palindromic_substrings(s, [&](const std::pair<int, int> &pos) {
        palindromes.insert(s.substr(pos.first, pos.second - pos.first + 1));
    });
    
    std::cout << "字符串: " << s << std::endl;
    std::cout << "所有回文子串: ";
    for (const auto &p : palindromes) {
        std::cout << "\"" << p << "\" ";
    }
    std::cout << std::endl;
}

