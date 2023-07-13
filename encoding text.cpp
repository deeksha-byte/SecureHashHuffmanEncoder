#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <queue>
#include <bitset>
#include <openssl/sha.h>
#include <string>
#include <iomanip>

using namespace std;

// Structure for the Huffman tree node
struct HuffmanNode {
    char data;
    unsigned frequency;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(char data, unsigned frequency)
        : data(data), frequency(frequency), left(nullptr), right(nullptr) {}
};

// Comparison function for the priority queue
struct Compare {
    bool operator()(const HuffmanNode* lhs, const HuffmanNode* rhs) {
        return lhs->frequency > rhs->frequency;
    }
};

// Function to build the Huffman tree
HuffmanNode* buildHuffmanTree(const unordered_map<char, unsigned>& frequencies) {
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, Compare> pq;

    // Create a leaf node for each character and add it to the priority queue
    for (const auto& pair : frequencies) {
        pq.push(new HuffmanNode(pair.first, pair.second));
    }

    // Build the Huffman tree by combining the nodes with the lowest frequency
    while (pq.size() > 1) {
        HuffmanNode* left = pq.top();
        pq.pop();
        HuffmanNode* right = pq.top();
        pq.pop();

        // Create a new internal node with the combined frequency
        HuffmanNode* newNode = new HuffmanNode('\0', left->frequency + right->frequency);
        newNode->left = left;
        newNode->right = right;

        pq.push(newNode);
    }

    return pq.top();
}

// Function to build the Huffman code table
void buildCodeTable(HuffmanNode* root, string code, unordered_map<char, string>& codes) {
    if (root == nullptr) {
        return;
    }

    // Leaf node found, add the code to the code table
    if (!root->left && !root->right) {
        codes[root->data] = code;
    }

    // Traverse the left and right subtrees recursively
    buildCodeTable(root->left, code + "0", codes);
    buildCodeTable(root->right, code + "1", codes);
}





// Function to encode the input text using Huffman coding
string encodeText(const string& text, const unordered_map<char, string>& codes) {
    string encodedText;
    for (char c : text) {
        if (codes.find(c) == codes.end()) {
            cout << "Key not found: " << c << endl;
            cout << "Available keys: ";
            for (const auto& pair : codes) {
                cout << pair.first << " ";
            }
            cout << endl;
            throw std::out_of_range("Key not found");
        }
        encodedText += codes.at(c);
    }
    return encodedText;
}


// Function to write the encoded text to the output file
void writeEncodedText(const string& encodedText, const string& outputFilename) {
    ofstream outputFile(outputFilename, ios::binary);
    if (!outputFile) {
        cout << "Error opening file: " << outputFilename << endl;
        return;
    }

    // Convert the binary string to bytes and write to the file
    bitset<8> bits;
    for (char bit : encodedText) {
        bits[7] = (bit == '1');
        if (bits[0]) {
            outputFile.put(static_cast<unsigned char>(bits.to_ulong()));
            bits.reset();
        }
        bits >>= 1;
    }

    // Write any remaining bits
    if (bits[0]) {
        outputFile.put(static_cast<unsigned char>(bits.to_ulong()));
    }

    outputFile.close();
}

// Function to compress the SHA-256 hash using Huffman coding
string compressSHA256(const string& sha256Hash) {
    unordered_map<char, unsigned> frequencies;
    for (char c : sha256Hash) {
        frequencies[c]++;
    }

    // Print the frequencies map for debugging
    cout << "Frequencies map:" << endl;
    for (const auto& pair : frequencies) {
        cout << pair.first << ": " << pair.second << endl;
    }

    // Build the Huffman tree
    HuffmanNode* root = buildHuffmanTree(frequencies);

    // Build the Huffman code table
    unordered_map<char, string> codes;
    buildCodeTable(root, "", codes);

    // Encode the SHA-256 hash using Huffman coding
    string encodedText = encodeText(sha256Hash, codes);

    // Clean up memory
    delete root;

    return encodedText;
}


// Function to calculate the SHA-256 hash of a text file
string calculateSHA256Hash(const string& filename) {
    ifstream inputFile(filename, ios::binary);
    if (!inputFile) {
        cout << "Error opening file: " << filename << endl;
        return "";
    }

    ostringstream buffer;
    buffer << inputFile.rdbuf();
    string content = buffer.str();

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(content.c_str()), content.length(), hash);

    stringstream ss;
    ss << hex << setfill('0');
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << setw(2) << static_cast<int>(hash[i]);
    }

    inputFile.close();

    return ss.str();
}

int main() {
    string inputFilename = "input.txt";
    string encodedOutputFilename = "encoded.bin";

    ifstream inputFile(inputFilename);
    if (!inputFile) {
        cout << "Error opening file: " << inputFilename << endl;
        return 1;
    }

    // Read the input text from the file
    string inputText((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());
    inputFile.close();

    // Calculate the SHA-256 hash of the input text
    string sha256Hash = calculateSHA256Hash(inputText);
    cout << "SHA-256 Hash: " << sha256Hash << endl;

    // Compress the SHA-256 hash using Huffman coding
    string encodedText = compressSHA256(sha256Hash);

    // Write the encoded text to the output file
    writeEncodedText(encodedText, encodedOutputFilename);

    cout << "Original Text: " << inputText << endl;
    cout << "Encoded Text: " << encodedText << endl;

    return 0;
}
