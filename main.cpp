#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <bitset>

using namespace std;

// Node structure for Huffman Tree
struct HuffmanNode {
    char character;
    int frequency;
    HuffmanNode *left, *right;

    HuffmanNode(char c, int f) : character(c), frequency(f), left(nullptr), right(nullptr) {}

    bool operator<(const HuffmanNode &other) const {
        return frequency > other.frequency;
    }
};

// Function to build the min-heap
priority_queue<HuffmanNode> buildMinHeap(vector<char> &characters, vector<int> &frequencies) {
    priority_queue<HuffmanNode> minHeap;
    for (int i = 0; i < characters.size(); ++i) {
        HuffmanNode *node = new HuffmanNode(characters[i], frequencies[i]);
        minHeap.push(*node);
    }
    return minHeap;
}

// Function to build the Huffman Tree
HuffmanNode *buildHuffmanTree(priority_queue<HuffmanNode> minHeap) {
    while (minHeap.size() > 1) {
        HuffmanNode *left = new HuffmanNode(minHeap.top());
        minHeap.pop();
        HuffmanNode *right = new HuffmanNode(minHeap.top());
        minHeap.pop();

        HuffmanNode *mergedNode = new HuffmanNode('$', left->frequency + right->frequency);
        mergedNode->left = left;
        mergedNode->right = right;
        minHeap.push(*mergedNode);
    }
    return new HuffmanNode(minHeap.top());
}

// Function to generate Huffman Codes
unordered_map<char, string> generateHuffmanCodes(HuffmanNode *root, string code = "") {
    unordered_map<char, string> huffmanCodes;
    if (root == nullptr) {
        return huffmanCodes;
    }
    if (root->character != '$') {
        huffmanCodes[root->character] = code;
    }
    unordered_map<char, string> leftCodes = generateHuffmanCodes(root->left, code + "0");
    unordered_map<char, string> rightCodes = generateHuffmanCodes(root->right, code + "1");
    huffmanCodes.insert(leftCodes.begin(), leftCodes.end());
    huffmanCodes.insert(rightCodes.begin(), rightCodes.end());
    return huffmanCodes;
}

// Function to compress the file
void compressFile(const string &inputFileName, const string &outputFileName, unordered_map<char, string> &huffmanCodes) {
    ifstream inputFile(inputFileName);
    ofstream outputFile(outputFileName, ios::binary);
    if (!outputFile.is_open()) {
        cerr << "Error opening output file!" << endl;
        return;
    }

    size_t huffmanCodesSize = huffmanCodes.size();
    outputFile.write(reinterpret_cast<const char *>(&huffmanCodesSize), sizeof(size_t));

    for (const auto &entry : huffmanCodes) {
        outputFile.put(entry.first);
        outputFile.put(entry.second.length());
        outputFile << entry.second;
    }

    outputFile.put('\n');

    string encodedBits;
    string text;
    while (getline(inputFile, text)) {
        for (char ch : text) {
            encodedBits += huffmanCodes.at(ch);
        }
    }

    size_t padding = 8 - (encodedBits.length() % 8);
    encodedBits += string(padding, '0');

    size_t encodedBitsSize = encodedBits.size();
    outputFile.write(reinterpret_cast<const char *>(&encodedBitsSize), sizeof(size_t));

    for (size_t i = 0; i < encodedBitsSize; i += 8) {
        bitset<8> bits(encodedBits.substr(i, 8));
        outputFile.put(bits.to_ulong());
    }

    outputFile.close();
    cout << "File compressed successfully." << endl;
}

// Function to decompress the file
void decompressFile(const string &inputFileName) {
    ifstream inputFile(inputFileName, ios::binary);
    if (!inputFile.is_open()) {
        cerr << "Error opening input file!" << endl;
        return;
    }

    size_t huffmanCodesSize;
    inputFile.read(reinterpret_cast<char *>(&huffmanCodesSize), sizeof(size_t));

    unordered_map<char, string> huffmanCodes;
    for (size_t i = 0; i < huffmanCodesSize; ++i) {
        char character;
        inputFile.get(character);

        char codeLength;
        inputFile.get(codeLength);

        string code;
        for (int j = 0; j < codeLength; ++j) {
            code += inputFile.get();
        }

        huffmanCodes[character] = code;
    }

    inputFile.get();

    size_t encodedBitsSize;
    inputFile.read(reinterpret_cast<char *>(&encodedBitsSize), sizeof(size_t));

    string encodedBits;
    char byte;
    while (inputFile.get(byte)) {
        encodedBits += bitset<8>(byte).to_string();
    }

    encodedBits.resize(encodedBitsSize);

    inputFile.close();

    string decodedText;
    string currentCode;

    for (char bit : encodedBits) {
        currentCode += bit;
        for (const auto &entry : huffmanCodes) {
            if (entry.second == currentCode) {
                decodedText += entry.first;
                currentCode.clear();
                break;
            }
        }
    }

    ofstream outputFile("decompressed.txt");
    outputFile << decodedText;
    cout << "File decompressed successfully." << endl;
}

// Function to count characters and their frequencies in the file
unordered_map<char, int> countCharacters(const string &fileName) {
    unordered_map<char, int> charCount;
    ifstream file(fileName);
    if (!file) {
        cout << "Unable to open file" << endl;
        return charCount;
    }
    string line;
    while (getline(file, line)) {
        for (char c : line) {
            charCount[c]++;
        }
    }
    file.close();
    return charCount;
}

int main() {
    int choice;
    string fileName;
    string inputFileName;

    cout << "____________________" << endl;
    cout << "|     OPTIONS      |" << endl;
    cout << "_________________" << endl;
    cout << "| 1.Compress    |" << endl;
    cout << "| 2.Decompress  |" << endl;
    cout << " " << endl;
    cout << "Enter choice :";
    cin >> choice;

    if (choice == 1) {
        cout << "Enter Filename :";
        cin >> fileName;

        unordered_map<char, int> characterFrequencies = countCharacters(fileName);

        vector<char> characters;
        vector<int> frequencies;
        for (const auto &pair : characterFrequencies) {
            characters.push_back(pair.first);
            frequencies.push_back(pair.second);
        }

        priority_queue<HuffmanNode> minHeap = buildMinHeap(characters, frequencies);
        HuffmanNode *root = buildHuffmanTree(minHeap);
        unordered_map<char, string> huffmanCodes = generateHuffmanCodes(root);

        // Print Huffman Codes
        for (const auto &code : huffmanCodes) {
            cout << code.first << " " << code.second << endl;
        }

        compressFile(fileName, "compressed.bin", huffmanCodes);
    }
    else if (choice == 2) {
        cout << "Enter filename: ";
        cin >> inputFileName;
        decompressFile(inputFileName);
    }
    else {
        cout << "Invalid choice!" << endl;
    }

    return 0;
}
