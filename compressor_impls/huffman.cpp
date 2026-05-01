#include "../compressor.h"

#include <queue>
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

void delete_tree(Node* root) {
    if (!root) return;
    delete_tree(root->left);
    delete_tree(root->right);
    delete root;
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

// Write the length of the code used for each character
// Only include characters which appear at least once, using a bitmask
void write_freqs(std::vector<uint8_t>& f, std::vector<int> counts) {
    // Mask defines which characters have a non-zero count
    // Interpreted as a 256-bit binary mask
    uint8_t mask[32] = {0};
    for (int c=0; c<256; c++) {
        if (counts[c] > 0)
            mask[c / 8] |= (1 << (c % 8));
    }
    for (uint8_t m : mask) f.push_back(m);

    for (uint32_t c = 0; c < 256; c++) {
        if (mask[c / 8] & (1 << (c % 8))) {
            for (int j = 0; j < 4; ++j) {
                f.push_back(static_cast<uint8_t>((counts[c] >> (j * 8)) & 0xFF));
            }
        }
    }
}

void write_lengths(std::vector<uint8_t>& f, const std::vector<Code>& enc) {
    // 1. Write the 32-byte mask as you do now
    uint8_t mask[32] = {0};
    for (int i = 0; i < 256; i++) {
        if (enc[i].length > 0) mask[i / 8] |= (1 << (i % 8));
    }
    for (uint8_t m : mask) f.push_back(m);

    // 2. Write 1 byte for the length of each active character
    for (int i = 0; i < 256; i++) {
        if (enc[i].length > 0) {
            f.push_back(static_cast<uint8_t>(enc[i].length));
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
    
        delete_tree(root);
        return out;
    }
    
    std::vector<uint8_t> decode(const std::vector<uint8_t>& txt) const override {
        std::vector<uint8_t> out;
        if (txt.empty()) return out;

        // Get the original file size
        uint64_t original_sz = 0;
        for (int i=0; i<8; i++) {
            uint8_t byte = txt.at(i);
            original_sz |= (static_cast<uint64_t>(byte) << (i * 8));
        }

        size_t currentPos = 8;

        // Read the encoder mask
        uint8_t mask[32] = {0};
        for (int i = 0; i < 32; i++) {
            mask[i] = txt.at(currentPos++);
        }

        std::vector<int> counts(256, 0);

        for (int charIdx = 0; charIdx < 256; charIdx++) {
            int freq = 0;
            if (mask[charIdx / 8] & (1 << (charIdx % 8))) {
                for (int byteIdx = 0; byteIdx < 4; byteIdx++) {
                    uint8_t b = txt.at(currentPos++);
                    freq |= (static_cast<int>(b) << (byteIdx * 8));
                }
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
    
        delete_tree(root);
        return out;
    }
};

static bool registered = []() {
    Compressor::getRegistry()["Huffman"] = []() { return std::make_unique<Huffman>(); };
    return true;
}();

