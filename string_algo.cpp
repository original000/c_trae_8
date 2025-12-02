#include "string_algo.h"

namespace string_algo {

std::vector<int> manacher(const std::string &s) {
    int n = s.size();
    if (n == 0) return {};
    
    std::string t = "#";
    for (char c : s) {
        t += c;
        t += "#";
    }
    
    int m = t.size();
    std::vector<int> p(m, 0);
    int C = 0, R = 0;
    
    for (int i = 0; i < m; ++i) {
        int mirror = 2 * C - i;
        if (i < R) {
            p[i] = std::min(R - i, p[mirror]);
        }
        
        int a = i + p[i] + 1;
        int b = i - p[i] - 1;
        while (a < m && b >= 0 && t[a] == t[b]) {
            p[i]++;
            a++;
            b--;
        }
        
        if (i + p[i] > R) {
            C = i;
            R = i + p[i];
        }
    }
    
    // 转换回原始格式
    std::vector<int> res(2 * n);
    for (int i = 0; i < n; ++i) {
        res[2 * i] = p[2 * i + 1] / 2;
    }
    for (int i = 0; i < n - 1; ++i) {
        res[2 * i + 1] = p[2 * i + 2] / 2;
    }
    
    return res;
}

std::vector<int> z_function(const std::string &s) {
    int n = s.size();
    std::vector<int> z(n);
    
    for (int i = 1, l = 0, r = 0; i < n; ++i) {
        if (i <= r) {
            z[i] = std::min(r - i + 1, z[i - l]);
        }
        
        while (i + z[i] < n && s[z[i]] == s[i + z[i]]) {
            ++z[i];
        }
        
        if (i + z[i] - 1 > r) {
            l = i;
            r = i + z[i] - 1;
        }
    }
    
    z[0] = n;
    return z;
}

std::vector<int> kmp_pi(const std::string &s) {
    int n = s.size();
    std::vector<int> pi(n);
    
    for (int i = 1; i < n; ++i) {
        int j = pi[i - 1];
        while (j > 0 && s[i] != s[j]) {
            j = pi[j - 1];
        }
        if (s[i] == s[j]) {
            ++j;
        }
        pi[i] = j;
    }
    
    return pi;
}

int minimal_rotation(const std::string &s) {
    int n = s.size();
    if (n == 0) return 0;
    
    std::string t = s + s;
    std::vector<int> f(2 * n, -1);
    int k = 0;
    
    for (int j = 1; j < 2 * n; ++j) {
        char sj = t[j];
        int i = f[j - k - 1];
        
        while (i != -1 && sj != t[k + i + 1]) {
            if (sj < t[k + i + 1]) {
                k = j - i - 1;
            }
            i = f[i];
        }
        
        if (sj != t[k + i + 1]) {
            if (sj < t[k]) {
                k = j;
            }
            f[j - k] = -1;
        } else {
            f[j - k] = i + 1;
        }
    }
    
    return k;
}

} // namespace string_algo