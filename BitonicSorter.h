#pragma once

#include <algorithm>
#include <iterator>
#include <functional>
#include <thread>
#include <chrono>
#include <cmath>
#include "ThreadPoolSimple.h"

#include <future>
#include <vector>

template <size_t PowerOfTwo, typename RandomIt, typename Compare = std::less<>>
void parallel_bitonic_sort(RandomIt first, Compare comp = Compare{}) {
    static_assert((PowerOfTwo & (PowerOfTwo - 1)) == 0, "PowerOfTwo must be a power of two");
    static_assert(PowerOfTwo <= 256, "PowerOfTwo must be <= 256 (2^8)");
    
    if (PowerOfTwo <= 1) {
        return;
    }
    
    ThreadPoolSimple pool;
    
    // 排序网络的每一层
    for (size_t stage = 1; stage <= static_cast<size_t>(std::log2(PowerOfTwo)); ++stage) {
        const size_t stage_size = 1ULL << stage;
        
        // 每个块
        for (size_t block = 0; block < PowerOfTwo; block += stage_size) {
            // 每一对比较器的距离
            for (size_t pair_dist = stage_size >> 1; pair_dist > 0; pair_dist >>= 1) {
                std::vector<std::future<void>> futures;
                
                // 提交所有比较交换任务到线程池
                for (size_t i = block; i < block + stage_size; i += 2 * pair_dist) {
                    for (size_t j = 0; j < pair_dist; ++j) {
                        const size_t idx1 = i + j;
                        const size_t idx2 = i + j + pair_dist;
                        
                        if (idx2 >= PowerOfTwo) {
                            continue;
                        }
                        
                        // 确定比较顺序，保持bitonic特性
                        bool should_swap = ((idx1 / stage_size) % 2 == 0);
                        
                        futures.push_back(pool.enqueue([first, comp, idx1, idx2, should_swap]() {
                            if (should_swap) {
                                if (comp(*(first + idx2), *(first + idx1))) {
                                    std::iter_swap(first + idx1, first + idx2);
                                }
                            } else {
                                if (comp(*(first + idx1), *(first + idx2))) {
                                    std::iter_swap(first + idx1, first + idx2);
                                }
                            }
                        }));
                    }
                }
                
                // 等待当前层所有任务完成
                for (auto& future : futures) {
                    future.get();
                }
            }
        }
    }
}

template <typename RandomIt, typename Compare = std::less<>>
void parallel_bitonic_sort(RandomIt first, RandomIt last, Compare comp = Compare{}) {
    const size_t size = std::distance(first, last);
    
    // 检查是否是2的幂
    if ((size & (size - 1)) != 0) {
        throw std::invalid_argument("Size must be a power of two for bitonic sort");
    }
    
    if (size <= 256) {
        switch(size) {
            case 2:    parallel_bitonic_sort<2>(first, comp); break;
            case 4:    parallel_bitonic_sort<4>(first, comp); break;
            case 8:    parallel_bitonic_sort<8>(first, comp); break;
            case 16:   parallel_bitonic_sort<16>(first, comp); break;
            case 32:   parallel_bitonic_sort<32>(first, comp); break;
            case 64:   parallel_bitonic_sort<64>(first, comp); break;
            case 128:  parallel_bitonic_sort<128>(first, comp); break;
            case 256:  parallel_bitonic_sort<256>(first, comp); break;
            default: break;
        }
    } else {
        throw std::invalid_argument("Size must be <= 256 (2^8)");
    }
}
