#include <iostream>
#include <vector>
#include <fstream>
#include <memory>
#include <stack>
#include <cassert>

using namespace std;

const int MAX_NODE = 400010;
const int MAX_VERSION = 100010;

// 持久化线段树节点
struct Node {
    int left, right;
    int val;
    Node() : left(0), right(0), val(0) {}
};

class PersistentSegmentTree {
private:
    vector<Node> tree;
    vector<int> roots;
    int cnt;
    int n;

    int build(int l, int r, const vector<int>& init_val) {
        int node = ++cnt;
        if (l == r) {
            tree[node].val = init_val[l];
            return node;
        }
        int mid = (l + r) / 2;
        tree[node].left = build(l, mid, init_val);
        tree[node].right = build(mid + 1, r, init_val);
        return node;
    }

    int update(int prev, int l, int r, int pos, int val) {
        int node = ++cnt;
        tree[node] = tree[prev];
        if (l == r) {
            tree[node].val = val;
            return node;
        }
        int mid = (l + r) / 2;
        if (pos <= mid) {
            tree[node].left = update(tree[prev].left, l, mid, pos, val);
        } else {
            tree[node].right = update(tree[prev].right, mid + 1, r, pos, val);
        }
        return node;
    }

    int query(int node, int l, int r, int pos) {
        if (l == r) {
            return tree[node].val;
        }
        int mid = (l + r) / 2;
        if (pos <= mid) {
            return query(tree[node].left, l, mid, pos);
        } else {
            return query(tree[node].right, mid + 1, r, pos);
        }
    }

public:
    PersistentSegmentTree(int size, const vector<int>& init_val) {
        n = size;
        cnt = 0;
        tree.resize(MAX_NODE * 20);
        roots.push_back(build(1, n, init_val));
    }

    int get_latest_root() {
        return roots.back();
    }

    int get_root(int version) {
        return roots[version];
    }

    int update_version(int prev_version, int pos, int val) {
        int new_root = update(roots[prev_version], 1, n, pos, val);
        roots.push_back(new_root);
        return roots.size() - 1;
    }

    int query_version(int version, int pos) {
        return query(roots[version], 1, n, pos);
    }

    int get_version_count() {
        return roots.size();
    }

    void save(ofstream& ofs) {
        int version_count = roots.size();
        ofs.write((char*)&version_count, sizeof(int));
        ofs.write((char*)roots.data(), sizeof(int) * version_count);
        ofs.write((char*)&cnt, sizeof(int));
        ofs.write((char*)tree.data(), sizeof(Node) * (cnt + 1));
    }

    void load(ifstream& ifs) {
        int version_count;
        ifs.read((char*)&version_count, sizeof(int));
        roots.resize(version_count);
        ifs.read((char*)roots.data(), sizeof(int) * version_count);
        ifs.read((char*)&cnt, sizeof(int));
        tree.resize(cnt + 1);
        ifs.read((char*)tree.data(), sizeof(Node) * (cnt + 1));
    }
};

class PersistentUnionFind {
private:
    int n;
    unique_ptr<PersistentSegmentTree> parent;
    unique_ptr<PersistentSegmentTree> rank;
    unique_ptr<PersistentSegmentTree> weight; // 0: same, 1: eat parent, 2: be eaten by parent
    stack<int> version_stack;

    pair<int, int> find_internal(int x, int version) {
        int p = parent->query_version(version, x);
        if (p == x) {
            return make_pair(x, 0);
        }
        pair<int, int> res = find_internal(p, version);
        int root = res.first;
        int w = res.second;
        int dist = (weight->query_version(version, x) + w) % 3;
        return make_pair(root, dist);
    }

public:
    PersistentUnionFind(int size) : n(size) {
        vector<int> init_parent(size + 1);
        vector<int> init_rank(size + 1, 1);
        vector<int> init_weight(size + 1, 0);
        for (int i = 1; i <= size; ++i) {
            init_parent[i] = i;
        }
        parent = make_unique<PersistentSegmentTree>(size, init_parent);
        rank = make_unique<PersistentSegmentTree>(size, init_rank);
        weight = make_unique<PersistentSegmentTree>(size, init_weight);
        version_stack.push(0);
    }

    int union_sets(int a, int b, int relation) {
        // relation: 0 same, 1 a eat b
        int current_version = version_stack.top();
        pair<int, int> res_a = find_internal(a, current_version);
        int root_a = res_a.first;
        int w_a = res_a.second;
        pair<int, int> res_b = find_internal(b, current_version);
        int root_b = res_b.first;
        int w_b = res_b.second;

        if (root_a == root_b) {
            // no change, return current version
            return current_version;
        }

        int rank_a = rank->query_version(current_version, root_a);
        int rank_b = rank->query_version(current_version, root_b);

        int new_version_parent = current_version;
        int new_version_rank = current_version;
        int new_version_weight = current_version;

        int final_version = current_version;
        if (rank_a <= rank_b) {
            new_version_parent = parent->update_version(current_version, root_a, root_b);
            int w = (w_b + relation - w_a + 3) % 3;
            new_version_weight = weight->update_version(new_version_parent, root_a, w);
            if (rank_a == rank_b) {
                new_version_rank = rank->update_version(new_version_weight, root_b, rank_b + 1);
                final_version = new_version_rank;
            } else {
                final_version = new_version_weight;
            }
        } else {
            new_version_parent = parent->update_version(current_version, root_b, root_a);
            int w = (w_a - relation - w_b + 3) % 3;
            new_version_weight = weight->update_version(new_version_parent, root_b, w);
            final_version = new_version_weight;
        }

        version_stack.push(final_version);
        return version_stack.top();
    }

    int find(int x, int version) {
        pair<int, int> res = find_internal(x, version);
        return res.first;
    }

    int get_relation(int a, int b, int version) {
        pair<int, int> res_a = find_internal(a, version);
        int root_a = res_a.first;
        int w_a = res_a.second;
        pair<int, int> res_b = find_internal(b, version);
        int root_b = res_b.first;
        int w_b = res_b.second;
        if (root_a != root_b) {
            return -1;
        }
        return (w_a - w_b + 3) % 3;
    }

    void undo() {
        assert(version_stack.size() > 1);
        version_stack.pop();
    }

    void time_travel(int version) {
        assert(version >= 0 && version < parent->get_version_count());
        while (version_stack.top() != version) {
            version_stack.pop();
        }
    }

    int get_current_version() {
        return version_stack.top();
    }

    void save(const string& filename) {
        ofstream ofs(filename, ios::binary);
        ofs.write((char*)&n, sizeof(int));
        int stack_size = version_stack.size();
        ofs.write((char*)&stack_size, sizeof(int));
        stack<int> temp_stack = version_stack;
        vector<int> stack_data;
        while (!temp_stack.empty()) {
            stack_data.push_back(temp_stack.top());
            temp_stack.pop();
        }
        ofs.write((char*)stack_data.data(), sizeof(int) * stack_size);
        parent->save(ofs);
        rank->save(ofs);
        weight->save(ofs);
        ofs.close();
    }

    void load(const string& filename) {
        ifstream ifs(filename, ios::binary);
        ifs.read((char*)&n, sizeof(int));
        int stack_size;
        ifs.read((char*)&stack_size, sizeof(int));
        vector<int> stack_data(stack_size);
        ifs.read((char*)stack_data.data(), sizeof(int) * stack_size);
        while (!version_stack.empty()) {
            version_stack.pop();
        }
        for (int i = stack_size - 1; i >= 0; --i) {
            version_stack.push(stack_data[i]);
        }
        parent->load(ifs);
        rank->load(ifs);
        weight->load(ifs);
        ifs.close();
    }
};

// POJ 1182 食物链问题演示
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    cout << "=== 食物链问题演示 ===" << endl;
    cout << "请输入动物数量和操作数量: ";
    cin >> n >> k;

    PersistentUnionFind uf(n);
    int ans = 0;

    for (int i = 1; i <= k; ++i) {
        int op, a, b;
        cout << "\n操作 " << i << ": ";
        cin >> op >> a >> b;

        if (a > n || b > n) {
            ans++;
            cout << "错误: 动物编号超出范围，回答错误，累计错误数: " << ans << endl;
            continue;
        }

        if (op == 1) {
            // 声明a和b是同类
            int rel = uf.get_relation(a, b, uf.get_current_version());
            if (rel == -1) {
                uf.union_sets(a, b, 0);
                cout << "成功合并，当前版本: " << uf.get_current_version() << endl;
            } else if (rel != 0) {
                ans++;
                cout << "错误: 与之前信息矛盾，回答错误，累计错误数: " << ans << endl;
            } else {
                cout << "正确: a和b确实是同类" << endl;
            }
        } else if (op == 2) {
            // 声明a吃b
            int rel = uf.get_relation(a, b, uf.get_current_version());
            if (rel == -1) {
                uf.union_sets(a, b, 1);
                cout << "成功合并，当前版本: " << uf.get_current_version() << endl;
            } else if (rel != 1) {
                ans++;
                cout << "错误: 与之前信息矛盾，回答错误，累计错误数: " << ans << endl;
            } else {
                cout << "正确: a确实吃b" << endl;
            }
        }

        // 演示撤销功能
        char choice;
        cout << "是否撤销此操作? (y/n): ";
        cin >> choice;
        if (choice == 'y' || choice == 'Y') {
            uf.undo();
            cout << "已撤销，当前版本回到: " << uf.get_current_version() << endl;
        }
    }

    cout << "\n最终错误数: " << ans << endl;

    // 演示保存和加载
    uf.save("uf_save.bin");
    cout << "\n已保存当前状态到 uf_save.bin" << endl;

    PersistentUnionFind uf2(1);
    uf2.load("uf_save.bin");
    cout << "已从 uf_save.bin 加载状态，当前版本: " << uf2.get_current_version() << endl;

    // 演示时间旅行
    int target_version;
    cout << "\n请输入要穿越到的版本号: ";
    cin >> target_version;
    try {
        uf2.time_travel(target_version);
        cout << "成功穿越到版本 " << target_version << endl;
    } catch (...) {
        cout << "版本号无效" << endl;
    }

    return 0;
}
