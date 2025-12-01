#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include "BitonicSorter.h"

int main() {
    constexpr size_t test_size = 16; // 2^4
    
    // 生成随机double数组
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(0.0, 1000.0);
    
    std::vector<double> arr1(test_size);
    for (auto& val : arr1) {
        val = dis(gen);
    }
    
    std::vector<double> arr2 = arr1;
    
    // 打印原始数组
    std::cout << "Original array: " << std::endl;
    for (const auto& val : arr1) {
        std::cout << val << " ";
    }
    std::cout << std::endl << std::endl;
    
    // 测试并行bitonic排序
    auto start = std::chrono::high_resolution_clock::now();
    parallel_bitonic_sort<test_size>(arr1.begin());
    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double, std::milli> bitonic_time = end - start;
    
    std::cout << "Parallel Bitonic Sort result: " << std::endl;
    for (const auto& val : arr1) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    std::cout << "Bitonic Sort time: " << bitonic_time.count() << " ms" << std::endl << std::endl;
    
    // 测试标准库sort
    start = std::chrono::high_resolution_clock::now();
    std::sort(arr2.begin(), arr2.end());
    end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double, std::milli> std_sort_time = end - start;
    
    std::cout << "Standard std::sort result: " << std::endl;
    for (const auto& val : arr2) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    std::cout << "std::sort time: " << std_sort_time.count() << " ms" << std::endl << std::endl;
    
    // 验证排序结果是否一致
    bool equal = std::equal(arr1.begin(), arr1.end(), arr2.begin());
    std::cout << "Sort results are " << (equal ? "EQUAL" : "DIFFERENT") << std::endl;
    
    // 测试自定义比较器（降序排序）
    std::vector<double> arr3 = arr2;
    parallel_bitonic_sort<test_size>(arr3.begin(), std::greater<double>());
    
    std::cout << "\nDescending sort result: " << std::endl;
    for (const auto& val : arr3) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
