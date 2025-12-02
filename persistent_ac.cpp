#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <queue>
#include <array>
#include <algorithm>
#include <iomanip>
#include <cstdint>

using namespace std;

class Compressor {
public:
    virtual vector<uint8_t> compress(string data) = 0;
    virtual string decompress(const vector<uint8_t>& data) = 0;
    virtual string name() const = 0;
    virtual ~Compressor() = default;
};

// ------------------------------ LZ77 + Huffman ------------------------------

struct LZ77Token {
    int offset;
    int length;
    uint8_t next;

    LZ77Token(int o = 0, int l = 0, uint8_t n = 0) : offset(o), length(l), next(n) {}
};

class HuffmanNode {
public:
    int freq;
    uint8_t symbol;
    HuffmanNode *left, *right;

    HuffmanNode(int f, uint8_t s) : freq(f), symbol(s), left(nullptr), right(nullptr) {}
    HuffmanNode(HuffmanNode* l, HuffmanNode* r) : freq(l->freq + r->freq), symbol(0), left(l), right(r) {}
    ~HuffmanNode() {
        delete left;
        delete right;
    }
};

struct HuffmanCompare {
    bool operator()(HuffmanNode* a, HuffmanNode* b) {
        return a->freq > b->freq;
    }
};

class LZ77Compressor : public Compressor {
private:
    static const int WINDOW_SIZE;
    static const int LOOKAHEAD_SIZE;

    vector<LZ77Token> lz77_compress(const string& data) {
        vector<LZ77Token> tokens;
        int n = data.size();
        if (n == 0) return tokens;
        
        int i = 0;

        while (i < n) {
            int best_offset = 0;
            int best_length = 0;
            int window_start = max(0, i - WINDOW_SIZE);

            for (int j = window_start; j < i; ++j) {
                int max_possible = min(LOOKAHEAD_SIZE, min(n - i, n - j));
                int length = 0;
                while (length < max_possible && data[j + length] == data[i + length]) {
                    length++;
                }
                if (length > best_length) {
                    best_length = length;
                    best_offset = i - j;
                }
            }

            uint8_t next_char = (i + best_length < n) ? (uint8_t)data[i + best_length] : 0;
            tokens.emplace_back(best_offset, best_length, next_char);
            
            if (best_length == 0) {
                i++;
            } else {
                i += best_length + 1;
            }
        }

        return tokens;
    }

    string lz77_decompress(const vector<LZ77Token>& tokens) {
        string result;
        for (const auto& token : tokens) {
            if (token.offset > 0 && token.length > 0 && result.size() >= (size_t)token.offset) {
                int start = (int)result.size() - token.offset;
                for (int i = 0; i < token.length; ++i) {
                    if (start + i < (int)result.size()) {
                        result += result[start + i];
                    } else {
                        break;
                    }
                }
            }
            if (token.next != 0) {
                result += (char)token.next;
            }
        }
        return result;
    }

    vector<uint8_t> serialize_tokens(const vector<LZ77Token>& tokens) {
        vector<uint8_t> buffer;
        size_t token_count = tokens.size();
        buffer.push_back((token_count >> 24) & 0xFF);
        buffer.push_back((token_count >> 16) & 0xFF);
        buffer.push_back((token_count >> 8) & 0xFF);
        buffer.push_back(token_count & 0xFF);

        for (const auto& token : tokens) {
            uint16_t offset = token.offset;
            uint8_t length = token.length;

            buffer.push_back((offset >> 8) & 0xFF);
            buffer.push_back(offset & 0xFF);
            buffer.push_back(length);
            buffer.push_back(token.next);
        }

        return buffer;
    }

    vector<LZ77Token> deserialize_tokens(const vector<uint8_t>& buffer) {
        vector<LZ77Token> tokens;
        if (buffer.size() < 4) return tokens;
        
        size_t token_count = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
        size_t pos = 4;
        
        while (pos + 3 < buffer.size() && tokens.size() < token_count) {
            uint16_t offset = (buffer[pos] << 8) | buffer[pos+1];
            uint8_t length = buffer[pos+2];
            uint8_t next_char = buffer[pos+3];
            pos += 4;
            
            tokens.emplace_back(offset, length, next_char);
        }

        return tokens;
    }

public:
    vector<uint8_t> compress(string data) override {
        if (data.empty()) return {};
        auto tokens = lz77_compress(data);
        return serialize_tokens(tokens);
    }

    string decompress(const vector<uint8_t>& data) override {
        if (data.empty()) return {};
        auto tokens = deserialize_tokens(data);
        return lz77_decompress(tokens);
    }

    string name() const override {
        return "LZ77";
    }
};

const int LZ77Compressor::WINDOW_SIZE = 32768;
const int LZ77Compressor::LOOKAHEAD_SIZE = 258;

// ------------------------------ LZ78 ------------------------------

class LZ78Compressor : public Compressor {
private:
    vector<pair<int, uint8_t>> lz78_compress(const string& data) {
        vector<pair<int, uint8_t>> tokens;
        map<string, int> dict;
        dict[""] = 0;
        int next_code = 1;
        string current;

        for (char c : data) {
            string temp = current + c;
            if (dict.count(temp)) {
                current = temp;
            } else {
                tokens.emplace_back(dict[current], (uint8_t)c);
                dict[temp] = next_code++;
                current = "";
            }
        }

        if (!current.empty()) {
            tokens.emplace_back(dict[current], 0);
        }

        return tokens;
    }

    string lz78_decompress(const vector<pair<int, uint8_t>>& tokens) {
        vector<string> dict = {""};
        string result;

        for (const auto& token : tokens) {
            string entry = dict[token.first];
            if (token.second != 0) {
                entry += (char)token.second;
                dict.push_back(entry);
            }
            result += entry;
        }

        return result;
    }

    vector<uint8_t> serialize_tokens(const vector<pair<int, uint8_t>>& tokens) {
        vector<uint8_t> buffer;
        for (const auto& token : tokens) {
            uint16_t code = token.first;
            buffer.push_back((code >> 8) & 0xFF);
            buffer.push_back(code & 0xFF);
            buffer.push_back(token.second);
        }
        return buffer;
    }

    vector<pair<int, uint8_t>> deserialize_tokens(const vector<uint8_t>& buffer) {
        vector<pair<int, uint8_t>> tokens;
        for (size_t i = 0; i < buffer.size(); i += 3) {
            uint16_t code = (buffer[i] << 8) | buffer[i + 1];
            uint8_t next_char = buffer[i + 2];
            tokens.emplace_back(code, next_char);
        }
        return tokens;
    }

public:
    vector<uint8_t> compress(string data) override {
        auto tokens = lz78_compress(data);
        return serialize_tokens(tokens);
    }

    string decompress(const vector<uint8_t>& data) override {
        auto tokens = deserialize_tokens(data);
        return lz78_decompress(tokens);
    }

    string name() const override {
        return "LZ78";
    }
};

// ------------------------------ LZW ------------------------------

class LZWCompressor : public Compressor {
private:
    static const int MAX_DICT_SIZE = 65536;

    vector<uint16_t> lzw_compress(const string& data) {
        map<string, uint16_t> dict;
        for (int i = 0; i < 256; ++i) {
            dict[string(1, (char)i)] = i;
        }

        vector<uint16_t> result;
        string current;
        int next_code = 256;

        for (char c : data) {
            string temp = current + c;
            if (dict.count(temp)) {
                current = temp;
            } else {
                result.push_back(dict[current]);
                if (dict.size() < MAX_DICT_SIZE) {
                    dict[temp] = next_code++;
                }
                current = string(1, c);
            }
        }

        if (!current.empty()) {
            result.push_back(dict[current]);
        }

        return result;
    }

    string lzw_decompress(const vector<uint16_t>& data) {
        if (data.empty()) return {};
        vector<string> dict(MAX_DICT_SIZE);
        for (int i = 0; i < 256; ++i) {
            dict[i] = string(1, (char)i);
        }

        string result;
        string current = dict[data[0]];
        result += current;
        int next_code = 256;

        for (size_t i = 1; i < data.size(); ++i) {
            uint16_t code = data[i];
            string entry;

            if (code < next_code) {
                entry = dict[code];
            } else {
                entry = current + current[0];
            }

            result += entry;
            if (next_code < MAX_DICT_SIZE) {
                dict[next_code++] = current + entry[0];
            }
            current = entry;
        }

        return result;
    }

    vector<uint8_t> serialize_codes(const vector<uint16_t>& codes) {
        vector<uint8_t> buffer;
        for (uint16_t code : codes) {
            buffer.push_back((code >> 8) & 0xFF);
            buffer.push_back(code & 0xFF);
        }
        return buffer;
    }

    vector<uint16_t> deserialize_codes(const vector<uint8_t>& buffer) {
        vector<uint16_t> codes;
        for (size_t i = 0; i < buffer.size(); i += 2) {
            uint16_t code = (buffer[i] << 8) | buffer[i + 1];
            codes.push_back(code);
        }
        return codes;
    }

public:
    vector<uint8_t> compress(string data) override {
        auto codes = lzw_compress(data);
        return serialize_codes(codes);
    }

    string decompress(const vector<uint8_t>& data) override {
        auto codes = deserialize_codes(data);
        return lzw_decompress(codes);
    }

    string name() const override {
        return "LZW";
    }
};

// ------------------------------ Main ------------------------------

#include <windows.h>

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    vector<string> test_datasets = {
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.",
        "aaaaabbbbbcccccdddddeeeeeffffffgggggg",
        "1234567890123456789012345678901234567890",
        "The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog."
    };

    vector<Compressor*> compressors = {
        new LZ77Compressor(),
        new LZ78Compressor(),
        new LZWCompressor()
    };

    cout << left << setw(20) << "数据集" << setw(20) << "原始大小(byte)";
    for (auto* compressor : compressors) {
        cout << setw(20) << compressor->name() + " 压缩大小(byte)" << setw(15) << compressor->name() + " 压缩率";
    }
    cout << endl;
    cout << string(200, '-') << endl;

    for (size_t i = 0; i < test_datasets.size(); ++i) {
        const string& data = test_datasets[i];
        size_t original_size = data.size();
        cout << left << setw(20) << "数据集 " + to_string(i + 1) << setw(20) << original_size;

        for (auto* compressor : compressors) {
            try {
                vector<uint8_t> compressed = compressor->compress(data);
                string decompressed = compressor->decompress(compressed);
                double ratio = (1.0 - (double)compressed.size() / original_size) * 100;

                cout << setw(20) << compressed.size() << setw(15) << fixed << setprecision(2) << ratio << "%";

                if (decompressed != data) {
                    cout << " (解压错误!)";
                }
            } catch (...) {
                cout << setw(20) << "错误" << setw(15) << "N/A";
            }
        }
        cout << endl;
    }

    for (auto* compressor : compressors) {
        delete compressor;
    }

    return 0;
}
