#include "../compressor.h"

#include <queue>
#include <iostream>
#include <numeric>
#include <map>
#include <fstream>

struct Node {
    uint8_t data;
    int freq;

    Node *left = nullptr, *right = nullptr;

    Node(uint8_t byte, int f) : data(byte), freq(f) {}
};

struct NodeCompare {
    bool operator()(Node* a, Node* b) const {
        return a->freq > b->freq;
    }
};

std::vector<int> get_freqs(const std::vector<uint8_t>& txt) {
    std::vector<int> counts(256, 0);

    for (uint8_t c : txt) {
        counts[c]++;
    }

    return counts;
}

std::priority_queue<Node*, std::vector<Node*>, NodeCompare> init_queue(std::vector<int>& counts) {
    std::priority_queue<Node*, std::vector<Node*>, NodeCompare> q;
    for (int i=0; i<256; i++) {
        if (counts[i] > 0) {
            Node* n = new Node(static_cast<uint8_t>(i), counts[i]);
            q.push(n);
        }
    }

    return q;
}

Node* create_tree(std::priority_queue<Node*, std::vector<Node*>, NodeCompare> q) {
    if (q.empty()) return nullptr;

    if (q.size() == 1) {
        Node* leaf = q.top(); q.pop();

        Node* parent = new Node(0, leaf->freq);
        parent->left = leaf; 
        return parent; 
    }

    while (q.size() > 1) {
        Node* a = q.top(); q.pop();
        Node* b = q.top(); q.pop();

        Node* parent = new Node(0, a->freq + b->freq);
        parent->left = a;
        parent->right = b;

        q.push(parent);
    }

    Node* root = q.top();
    q.pop(); 
    return root;
}

struct Code {
    uint64_t bits = 0;
    size_t length = 0;
};

void get_encoding(Node *root, uint64_t current_code, size_t depth, std::vector<Code>& encoding) {
    if (!root) return;

    if (!root->left && !root->right) {
        encoding.at(root->data) = {current_code, depth};
        return;
    }

    get_encoding(root->left, (current_code << 1) | 0, depth+1, encoding);
    get_encoding(root->right, (current_code << 1) | 1, depth+1, encoding);
}

// Write the original size of the input, little-endian
void write_size(std::vector<uint8_t>& f, uint64_t sz) {
    for (int i=0; i<8; i++) {
        f.push_back(static_cast<uint8_t>((sz >> (i * 8)) & 0xFF));
    }
}

// Write the frequency of each character, to allow reconstruction later
// little-endian
void write_freqs(std::vector<uint8_t>& f, std::vector<int> counts) {
    for (int freq : counts) {
        for (int i = 0; i < 4; ++i) {
            f.push_back(static_cast<uint8_t>((freq >> (i * 8)) & 0xFF));
        }
    }
}

class Huffman : public Compressor {
public:
    std::vector<uint8_t> encode(const std::vector<uint8_t>& txt) const override {
        std::vector<uint8_t> out;
        if (txt.empty()) return out;

        std::vector<int> counts = get_freqs(txt);
        write_size(out, static_cast<uint64_t>(txt.size()));
        write_freqs(out, counts);

        std::priority_queue<Node*, std::vector<Node*>, NodeCompare> q = init_queue(counts);
        Node* root = create_tree(q);

        std::vector<Code> enc(256);
        get_encoding(root, 0, 0, enc);

        // This could be improved by only writing the characters used in the message
        uint8_t buf = 0;
        size_t buf_len = 0;
        for (uint8_t c : txt) {
            Code code = enc[c];

            for (int i=0; i<code.length; i++) {
                bool bit = (code.bits >> (code.length - 1 - i)) & 1;
                buf = (buf << 1) | bit;
                buf_len++;

                if (buf_len == 8) {
                    out.push_back(buf);
                    buf = 0; buf_len = 0;
                }
            }

        }

        // Write remaining bits
        if (buf_len > 0) {
            buf = buf << (8 - buf_len);
            out.push_back(buf);
        }
    
        return out;
    }
    
    std::vector<uint8_t> decode(const std::vector<uint8_t>& txt) const override {
        std::vector<uint8_t> out;
        if (txt.empty()) return out;

        uint64_t original_sz = 0;
        for (int i=0; i<8; i++) {
            uint8_t byte = txt.at(i);
            original_sz |= (static_cast<uint64_t>(byte) << (i * 8));
        }

        std::vector<int> counts(256, 0);
        size_t currentPos = 8;

        for (int charIdx = 0; charIdx < 256; charIdx++) {
            int freq = 0;
            for (int byteIdx = 0; byteIdx < 4; byteIdx++) {
                uint8_t b = txt.at(currentPos++);
                freq |= (static_cast<int>(b) << (byteIdx * 8));
            }
            counts[charIdx] = freq;
        }

        // Recreate the tree given the counts from the header
        auto q = init_queue(counts);
        Node* root = create_tree(q);

        Node *cur = root;

        for (size_t i = currentPos; i < txt.size() && out.size() < original_sz; i++) {
            uint8_t byte = txt.at(i);
            
            for (int bitIdx = 7; bitIdx >= 0 && out.size() < original_sz; bitIdx--) {
                bool bit = (byte >> bitIdx) & 1;
                
                cur = (bit == 0) ? cur->left : cur->right;
                
                // Character found
                if (!cur->left && !cur->right) {
                    out.push_back(cur->data);
                    cur = root;
                }
            }
        }
    
        return out;
    }
};

static bool registered = []() {
    Compressor::getRegistry()["Huffman"] = []() { return std::make_unique<Huffman>(); };
    return true;
}();
