#include <iostream>
#include <vector>
#include <queue>
#include <cstring>
#include <algorithm>
#include <memory>

using namespace std;

const int ALPHABET_SIZE = 26;

struct Node {
    vector<int> output; // 存储模式串ID
    Node* children[ALPHABET_SIZE];
    Node* fail;
    int dfs_in, dfs_out;

    Node() : fail(nullptr), dfs_in(-1), dfs_out(-1) {
        memset(children, 0, sizeof(children));
    }

    // 拷贝构造函数，用于可持久化创建新节点
    Node(const Node& other) : output(other.output), fail(other.fail), dfs_in(other.dfs_in), dfs_out(other.dfs_out) {
        memcpy(children, other.children, sizeof(children));
    }
};

class PersistentAC {
private:
    vector<Node*> versions; // 存储各版本根节点
    vector<bool> pattern_valid; // 标记模式串是否有效
    int dfs_timer;
    vector<Node*> all_nodes; // 用于内存管理

    // 创建新节点（拷贝或新建）
    Node* new_node(Node* old = nullptr) {
        Node* node;
        if (old) {
            node = new Node(*old);
        } else {
            node = new Node();
        }
        all_nodes.push_back(node);
        return node;
    }

    // DFS生成dfs序
    void dfs(Node* u) {
        u->dfs_in = dfs_timer++;
        for (int i = 0; i < ALPHABET_SIZE; ++i) {
            if (u->children[i]) {
                dfs(u->children[i]);
            }
        }
        u->dfs_out = dfs_timer - 1;
    }

public:
    PersistentAC() : dfs_timer(0) {
        // 初始版本空根节点
        versions.push_back(new_node());
    }

    ~PersistentAC() {
        for (Node* node : all_nodes) {
            delete node;
        }
    }

    // 插入模式串，返回新版本根节点索引
    int insert(int prev_version, const string& pattern, int pattern_id) {
        Node* prev_root = versions[prev_version];
        Node* new_root = new_node(prev_root);
        Node* curr = new_root;

        // 确保模式串ID有效
        if (pattern_id >= (int)pattern_valid.size()) {
            pattern_valid.resize(pattern_id + 1, true);
        } else {
            pattern_valid[pattern_id] = true;
        }

        for (char c : pattern) {
            int idx = tolower(c) - 'a';
            // 创建新节点
            Node* next_node = new_node(curr->children[idx]);
            curr->children[idx] = next_node;
            curr = next_node;
        }

        // 添加输出ID
        curr->output.push_back(pattern_id);
        versions.push_back(new_root);
        return versions.size() - 1;
    }

    // 构建fail树和DFS序
    void build_fail(int version) {
        Node* root = versions[version];
        queue<Node*> q;

        root->fail = nullptr;
        q.push(root);

        while (!q.empty()) {
            Node* u = q.front();
            q.pop();

            for (int i = 0; i < ALPHABET_SIZE; ++i) {
                Node* v = u->children[i];
                if (!v) continue;

                Node* f = u->fail;
                while (f && !f->children[i]) {
                    f = f->fail;
                }
                v->fail = f ? f->children[i] : root;
                
                // 合并fail节点的输出
                v->output.insert(v->output.end(), v->fail->output.begin(), v->fail->output.end());
                q.push(v);
            }
        }

        // 生成DFS序
        dfs_timer = 0;
        dfs(root);
    }

    // 匹配文本，返回所有匹配到的有效模式串ID
    vector<int> match(int version, const string& text) {
        Node* root = versions[version];
        Node* curr = root;
        vector<int> result;

        for (char c : text) {
            int idx = tolower(c) - 'a';
            while (curr && !curr->children[idx]) {
                curr = curr->fail;
            }
            curr = curr ? curr->children[idx] : root;

            // 收集所有有效输出
            for (int id : curr->output) {
                if (pattern_valid[id]) {
                    result.push_back(id);
                }
            }
        }

        return result;
    }

    // 删除指定ID的模式串
    void delete_pattern(int pattern_id) {
        if (pattern_id < (int)pattern_valid.size()) {
            pattern_valid[pattern_id] = false;
        }
    }

    // 获取版本数量
    int version_count() const {
        return versions.size();
    }
};

int main() {
    PersistentAC pac;

    // 版本1：添加 "he" (id=0) 和 "she" (id=1)
    int v1 = pac.insert(0, "he", 0);
    v1 = pac.insert(v1, "she", 1);
    pac.build_fail(v1);

    // 版本2：在版本1基础上添加 "his" (id=2)
    int v2 = pac.insert(v1, "his", 2);
    pac.build_fail(v2);

    cout << "=== 版本1 匹配测试：\"ushers\" ===" << endl;
    vector<int> res1 = pac.match(v1, "ushers");
    for (int id : res1) {
        if (id == 0) cout << "匹配到模式串 ID 0: he" << endl;
        if (id == 1) cout << "匹配到模式串 ID 1: she" << endl;
    }

    cout << "\n=== 版本2 匹配测试：\"ushers\" ===" << endl;
    vector<int> res2 = pac.match(v2, "ushers");
    for (int id : res2) {
        if (id == 0) cout << "匹配到模式串 ID 0: he" << endl;
        if (id == 1) cout << "匹配到模式串 ID 1: she" << endl;
        if (id == 2) cout << "匹配到模式串 ID 2: his" << endl;
    }

    cout << "\n=== 删除模式串 ID 0 (he) 后版本2 匹配测试 ===" << endl;
    pac.delete_pattern(0);
    vector<int> res3 = pac.match(v2, "ushers");
    for (int id : res3) {
        if (id == 0) cout << "匹配到模式串 ID 0: he" << endl;
        if (id == 1) cout << "匹配到模式串 ID 1: she" << endl;
        if (id == 2) cout << "匹配到模式串 ID 2: his" << endl;
    }

    return 0;
}