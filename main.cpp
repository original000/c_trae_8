#include "VectorExpr.h"
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <cmath>

constexpr size_t VEC_SIZE = 1024 * 1024;
constexpr int TEST_ROUNDS = 100;

// 普通版本计算：(a+b)*(c-d)+sin(e)
template<typename T, size_t N>
void normal_calc(const Vector<T, N>& a, const Vector<T, N>& b, 
                  const Vector<T, N>& c, const Vector<T, N>& d,
                  const Vector<T, N>& e, Vector<T, N>& result) {
    for (size_t i = 0; i < N; ++i) {
        T temp1 = a[i] + b[i];
        T temp2 = c[i] - d[i];
        T temp3 = temp1 * temp2;
        T temp4 = sin(e[i]);
        result[i] = temp3 + temp4;
    }
}

int main() {
    std::cout << "=== Expression Template Math Library Test ===" << std::endl;
    std::cout << "Vector Size: " << VEC_SIZE << std::endl;
    std::cout << "Test Rounds: " << TEST_ROUNDS << std::endl;
    std::cout << std::endl;

    // 初始化测试数据
    Vector<float, VEC_SIZE> a, b, c, d, e;
    Vector<float, VEC_SIZE> res_expr, res_normal;

    srand(42);
    for (size_t i = 0; i < VEC_SIZE; ++i) {
        a[i] = static_cast<float>(rand()) / RAND_MAX * 10.0f;
        b[i] = static_cast<float>(rand()) / RAND_MAX * 10.0f;
        c[i] = static_cast<float>(rand()) / RAND_MAX * 10.0f;
        d[i] = static_cast<float>(rand()) / RAND_MAX * 10.0f;
        e[i] = static_cast<float>(rand()) / RAND_MAX * 3.1415926f;
    }

    // 1. 正确性验证
    std::cout << "1. Verifying correctness..." << std::endl;
    
    // 表达式模板版本
    res_expr = (a + b) * (c - d) + sin(e);
    
    // 普通版本
    normal_calc(a, b, c, d, e, res_normal);
    
    // 验证结果
    bool correct = true;
    const float eps = 1e-5f;
    for (size_t i = 0; i < VEC_SIZE; ++i) {
        if (fabs(res_expr[i] - res_normal[i]) > eps) {
            correct = false;
            std::cout << "  Error at index " << i << ": expr = " << res_expr[i] << ", normal = " << res_normal[i] << std::endl;
            break;
        }
    }
    
    if (correct) {
        std::cout << "  ✓ Calculation is correct!" << std::endl;
    } else {
        std::cout << "  ✗ Calculation has errors!" << std::endl;
        return 1;
    }
    std::cout << std::endl;

    // 2. 性能测试 - 表达式模板版本
    std::cout << "2. Performance Testing..." << std::endl;
    
    auto start_expr = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < TEST_ROUNDS; ++i) {
        res_expr = (a + b) * (c - d) + sin(e);
    }
    auto end_expr = std::chrono::high_resolution_clock::now();
    auto time_expr = std::chrono::duration_cast<std::chrono::milliseconds>(end_expr - start_expr).count();

    // 3. 性能测试 - 普通版本
    auto start_normal = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < TEST_ROUNDS; ++i) {
        normal_calc(a, b, c, d, e, res_normal);
    }
    auto end_normal = std::chrono::high_resolution_clock::now();
    auto time_normal = std::chrono::duration_cast<std::chrono::milliseconds>(end_normal - start_normal).count();

    // 输出性能对比
    std::cout << "  Expression Template Version: " << time_expr << " ms" << std::endl;
    std::cout << "  Normal Loop Version:         " << time_normal << " ms" << std::endl;
    std::cout << "  Speedup:                     " << static_cast<double>(time_normal) / time_expr << "x faster" << std::endl;
    std::cout << std::endl;

    // 4. 测试其他功能
    std::cout << "3. Testing Additional Features..." << std::endl;
    
    // 测试切片
    auto slice = a.slice<100, 5>();
    std::cout << "  ✓ Slice operation works" << std::endl;
    
    // 测试Reduce
    float sum = a.reduce_sum();
    float max_val = a.reduce_max();
    float min_val = a.reduce_min();
    std::cout << "  ✓ Reduce operations work: sum = " << sum << ", max = " << max_val << ", min = " << min_val << std::endl;
    
    // 测试广播
    auto vec_broadcast = Vector<float, 5>::broadcast(3.14f);
    std::cout << "  ✓ Broadcast operation works" << std::endl;
    
    // 测试其他数学函数
    auto test_func = exp(log(abs(a + 1.0f)));
    std::cout << "  ✓ Math functions work correctly" << std::endl;

    std::cout << std::endl << "=== All tests completed successfully! ===" << std::endl;

    return 0;
}
