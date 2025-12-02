#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <memory>
#include <cstdint>

struct State {
    int len;            // 最长子串的长度
    int link;           // 后缀链接
    std::unordered_map<char, int> next; // 转移
    int cnt;            // 出现次数
    std::vector<int> end_positions; // 所有结束位置

    State() : len(0), link(-1), cnt(0) {}
};

class SuffixAutomaton {
private:
    std::vector<State> states;
    int last;
    std::vector<std::string> added_strings;
    size_t total_length;

    void sa_extend(char c, int current_end_pos) {
        int p = last;
        int curr = states.size();
        states.emplace_back();
        states[curr].len = states[last].len + 1;
        states[curr].end_positions.push_back(current_end_pos);
        states[curr].cnt = 1;

        while (p != -1 && !states[p].next.count(c)) {
            states[p].next[c] = curr;
            p = states[p].link;
        }

        if (p == -1) {
            states[curr].link = 0;
        } else {
            int q = states[p].next[c];
            if (states[p].len + 1 == states[q].len) {
                states[curr].link = q;
            } else {
                int clone = states.size();
                states.push_back(states[q]);
                states[clone].len = states[p].len + 1;
                states[clone].cnt = 0;
                states[clone].end_positions.clear();

                while (p != -1 && states[p].next[c] == q) {
                    states[p].next[c] = clone;
                    p = states[p].link;
                }
                states[q].link = clone;
                states[curr].link = clone;
            }
        }
        last = curr;
    }

    void calculate_occurrences() {
        std::vector<int> order(states.size());
        for (int i = 0; i < states.size(); ++i) {
            order[i] = i;
        }

        std::sort(order.begin(), order.end(), [this](int a, int b) {
            return states[a].len > states[b].len;
        });

        for (int u : order) {
            if (states[u].link != -1) {
                states[states[u].link].cnt += states[u].cnt;
                // 合并结束位置
                states[states[u].link].end_positions.insert(
                    states[states[u].link].end_positions.end(),
                    states[u].end_positions.begin(),
                    states[u].end_positions.end()
                );
            }
        }
    }

public:
    SuffixAutomaton() : last(0), total_length(0) {
        states.emplace_back();
    }

    void add_string(const std::string &s) {
        added_strings.push_back(s);
        int start_pos = total_length;
        last = 0;
        for (size_t i = 0; i < s.size(); ++i) {
            sa_extend(s[i], start_pos + i);
        }
        total_length += s.size();
    }

    void build() {
        calculate_occurrences();
    }

    // 查找模式串的所有出现位置
    std::vector<int> find_all_occurrences(const std::string &pattern) {
        int curr = 0;
        for (char c : pattern) {
            if (!states[curr].next.count(c)) {
                return {};
            }
            curr = states[curr].next[c];
        }

        std::vector<int> positions;
        for (int pos : states[curr].end_positions) {
            positions.push_back(pos - pattern.size() + 1);
        }
        std::sort(positions.begin(), positions.end());
        return positions;
    }

    // 统计模式串出现次数
    int count_occurrences(const std::string &pattern) {
        int curr = 0;
        for (char c : pattern) {
            if (!states[curr].next.count(c)) {
                return 0;
            }
            curr = states[curr].next[c];
        }
        return states[curr].cnt;
    }

    // 最长公共子串
    std::string longest_common_substring(const std::string &t) {
        int curr = 0;
        int current_len = 0;
        int max_len = 0;
        int end_pos = 0;

        for (size_t i = 0; i < t.size(); ++i) {
            while (curr != 0 && !states[curr].next.count(t[i])) {
                curr = states[curr].link;
                current_len = states[curr].len;
            }

            if (states[curr].next.count(t[i])) {
                curr = states[curr].next[t[i]];
                current_len++;
            }

            if (current_len > max_len) {
                max_len = current_len;
                end_pos = i;
            }
        }

        return t.substr(end_pos - max_len + 1, max_len);
    }

    size_t size() const {
        return states.size();
    }
};
