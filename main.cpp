#include "suffix_automaton.h"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>

// 全局恶意特征库
std::vector<std::string> malicious_paths = {
    "/admin", "/login", "/api", "/wp-admin", "/phpmyadmin",
    "/shell", "/cmd", "/exec", "/upload", "/download",
    "/config", ".env", "/backup", "/dump", "/sql",
    "/cgi-bin", "/bin", "/tmp", "/var", "/root"
};

std::vector<std::string> malicious_params = {
    "?id=", "?user=", "?pass=", "?cmd=", "?exec=",
    "?shell=", "?php=", "?file=", "?path=", "?download="
};

// 生成100条恶意URL特征
std::vector<std::string> generate_malicious_urls() {
    std::vector<std::string> urls;
    
    // 生成组合URL
    for (int i = 0; i < 100; ++i) {
        std::string url = "http://malicious-" + std::to_string(i) + ".com";
        url += malicious_paths[i % malicious_paths.size()];
        url += malicious_params[i % malicious_params.size()] + "evil_payload";
        urls.push_back(url);
    }
    
    return urls;
}

int main() {
    std::cout << "=== Suffix Automaton Test ===" << std::endl;
    
    // 创建后缀自动机
    SuffixAutomaton sa;
    
    // 生成并添加100条恶意URL
    auto urls = generate_malicious_urls();
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (const auto &url : urls) {
        sa.add_string(url);
    }
    
    sa.build();
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Added 100 malicious URLs, built automaton in " << duration.count() << "ms" << std::endl;
    std::cout << "Automaton states: " << sa.size() << std::endl;
    
    // 测试1: 统计出现次数
    std::cout << "\n=== Test 1: Occurrence Counting ===" << std::endl;
    std::string test_pattern = "/admin";
    int count = sa.count_occurrences(test_pattern);
    std::cout << "Pattern \"" << test_pattern << "\" appears " << count << " times" << std::endl;
    
    test_pattern = "evil_payload";
    count = sa.count_occurrences(test_pattern);
    std::cout << "Pattern \"" << test_pattern << "\" appears " << count << " times" << std::endl;
    
    // 测试2: 查找所有出现位置
    std::cout << "\n=== Test 2: Find All Occurrences ===" << std::endl;
    test_pattern = "?cmd=";
    auto positions = sa.find_all_occurrences(test_pattern);
    std::cout << "Pattern \"" << test_pattern << "\" found at positions: ";
    for (size_t i = 0; i < std::min(positions.size(), (size_t)5); ++i) {
        std::cout << positions[i] << " ";
    }
    if (positions.size() >5) {
        std::cout << "... and more";
    }
    std::cout << std::endl;
    
    // 测试3: 最长公共子串
    std::cout << "\n=== Test 3: Longest Common Substring ===" << std::endl;
    std::string test_string = "http://example.com/admin?id=123";
    std::string lcs = sa.longest_common_substring(test_string);
    std::cout << "Longest common substring with \"" << test_string << "\" is: \"" << lcs << "\"" << std::endl;
    
    // 测试4: 不存在的模式
    std::cout << "\n=== Test 4: Non-existent Pattern ===" << std::endl;
    test_pattern = "good_payload";
    count = sa.count_occurrences(test_pattern);
    std::cout << "Pattern \"" << test_pattern << "\" appears " << count << " times" << std::endl;
    
    // 压测实时查找
    std::cout << "\n=== Test 5: Real-time Search Benchmark ===" << std::endl;
    start_time = std::chrono::high_resolution_clock::now();
    int total_queries = 1000;
    for (int i =0; i < total_queries; ++i) {
        sa.count_occurrences(malicious_paths[i % malicious_paths.size()]);
    }
    end_time = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << total_queries << " queries completed in " << duration.count() << "ms (" << (total_queries * 1000 / duration.count()) << " QPS)" << std::endl;
    
    return 0;
}
