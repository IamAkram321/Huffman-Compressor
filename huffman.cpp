#include <emscripten.h>
#include <unordered_map>
#include <queue>
#include <string>

using namespace std;

class Node {
public:
    char character;
    int freq;
    Node *left, *right;
    Node(char c, int f) { character = c; freq = f; left = right = nullptr; }
    ~Node() { delete left; delete right; }  // Destructor for cleanup
};

struct Compare {
    bool operator()(Node* a, Node* b) { return a->freq > b->freq; }
};

unordered_map<char, int> getFrequency(const string& text) {
    unordered_map<char, int> freqTable;
    for (char c : text) freqTable[c]++;
    return freqTable;
}

Node* buildHuffmanTree(unordered_map<char, int>& freq) {
    priority_queue<Node*, vector<Node*>, Compare> minHeap;
    for (auto& pair : freq) minHeap.push(new Node(pair.first, pair.second));
    while (minHeap.size() > 1) {
        Node* left = minHeap.top(); minHeap.pop();
        Node* right = minHeap.top(); minHeap.pop();
        Node* parent = new Node('\0', left->freq + right->freq);
        parent->left = left;
        parent->right = right;
        minHeap.push(parent);
    }
    return minHeap.top();
}

void generateCodes(Node* root, string code, unordered_map<char, string>& huffmanCodes) {
    if (!root) return;
    if (!root->left && !root->right) huffmanCodes[root->character] = code.empty() ? "0" : code;
    generateCodes(root->left, code + "0", huffmanCodes);
    generateCodes(root->right, code + "1", huffmanCodes);
}

string compress(const string& text, unordered_map<char, string>& huffmanCodes) {
    string compressed = "";
    for (char c : text) compressed += huffmanCodes[c];
    return compressed;
}

// Exported function for WebAssembly
extern "C" {
    // Returns a pointer to the compressed string in memory
    EMSCRIPTEN_KEEPALIVE
    char* huffmanCompress(const char* input) {
        if (!input || !input[0]) return nullptr;  // Handling the empty input

        string text(input);
        unordered_map<char, int> freq = getFrequency(text);
        Node* root = buildHuffmanTree(freq);
        unordered_map<char, string> huffmanCodes;
        generateCodes(root, "", huffmanCodes);

        string compressed = compress(text, huffmanCodes);

        // Allocating memory for the result and copy the string
        char* result = (char*)malloc(compressed.length() + 1);
        strcpy(result, compressed.c_str());

        delete root;  // Cleaning up the tree
        return result;
    }
}