#include <QCoreApplication>
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <vector>
#include "stdlib.h"

using namespace std;


class Node{
public:
    int freq;           // the frequency of occurrences of the character
    char symbol;        // symbol that is found in the text
    Node *left, *right; // a pointer to the left and right child

    Node(char ch, int weigth):
        symbol(ch), freq(weigth){
        left = NULL;
        right = NULL;
    }

    Node(Node* first, Node* second){
        this->freq = first->freq + second->freq;
        this->left = first;
        this->right = second;
        this->symbol = '\0';
    }

    ~Node(){
        delete left;
        delete right;
    }

};

//function determines on which field compare nodes
//param lefs - first Node
//param right - second Node
bool myCompareNodes(Node* left, Node* right){
    return (left->freq < right->freq);
}

ifstream* openFileForRead(char* fileName){
    ifstream * result = new ifstream(fileName, ios::binary);
    if (!result->is_open()){
        cout<< "File not open! ERROR!";
        exit(EXIT_FAILURE);
    }
    //    cout<< "File ready!"<<endl;
    return result;
}

ofstream* openFileForWrite(char* outputCodedFileName){
    ofstream * result = new ofstream(outputCodedFileName, ios::binary);
    if (!result->is_open()){
        cout<< "File not open! ERROR!";
        exit(EXIT_FAILURE);
    }
    return result;
}


void closeFile(ifstream* sourceFile){
    sourceFile->close();
    delete sourceFile;
}

void closeFile(ofstream* sourceFile){
    sourceFile->close();
    delete sourceFile;
}


/*Creating a frequency table of characters from a source file
 * param sourceFileStream - a pointer to the source file
 * return - the symbol table and the frequency of their repeat
*/
map<char, int>* createSymbolFrequencyInFile(ifstream* sourceFileStream){
    map<char, int>* result = new map<char, int>;
    char symbol;

    while (sourceFileStream->get(symbol))
        (*result)[symbol]++;

    closeFile(sourceFileStream);
    return result;
}

/*Function builds the Huffman tree from the table of frequency repeat of symbols
 * param symbolFrequencyInFile - the symbol table and the frequency of their repeat
 * return - a pointer to the root node of the tree
*/
Node* buildTree(map<char, int>* symbolFrequencyInFile){
    list<Node*> nodeSymbolList;
    map<char, int>::iterator it;
    //Filled with a list of nodes to enable sorting
    for (it = symbolFrequencyInFile->begin(); it != symbolFrequencyInFile->end(); it++){
        Node* newNode = new Node(it->first, it->second);
        nodeSymbolList.push_back(newNode);
    }

    //Building a Huffman tree
    while(nodeSymbolList.size() > 1){
        nodeSymbolList.sort(myCompareNodes);

        Node *firstSymbolNode = nodeSymbolList.front();
        nodeSymbolList.pop_front();

        Node *secondSymbolNode = nodeSymbolList.front();
        nodeSymbolList.pop_front();

        Node *parent = new Node(firstSymbolNode, secondSymbolNode);
        nodeSymbolList.push_back(parent);

    }

    return nodeSymbolList.front();
}

/*
 * param rootNode - a pointer to the root node of the tree
 *
*/

void recursiveFillTable(Node* rootNode, map<char, vector<bool> >* symbolCodeTable, vector<bool> &codeSymbol){
    if (rootNode->left){
        codeSymbol.push_back(0);
        recursiveFillTable(rootNode->left, symbolCodeTable, codeSymbol);
    }
    if (rootNode->right){
        codeSymbol.push_back(1);
        recursiveFillTable(rootNode->right, symbolCodeTable, codeSymbol);
    }
    if ((!rootNode->left) && (!rootNode->right))
        (*symbolCodeTable)[rootNode->symbol] = codeSymbol;

    codeSymbol.pop_back();

}

/*The function fills the table of character codes of the Huffman tree
 *param rootNode - a pointer to the root node of the tree
 *return - table of pointers character codes Huffman
*/
map<char, vector<bool> >*  buildSymbolCodeTable(Node *rootNode){

    vector<bool> codeSymbol;
    map<char, vector<bool> >* result = new map<char, vector<bool> >();

    recursiveFillTable(rootNode, result, codeSymbol);

    delete rootNode;
    return result;
}

void createCompresedFile(char* sourceFileName, map<char, vector<bool> > *symbolCodeTable, char* outputCodedFileName){

    ifstream* sourceFileStream = openFileForRead(sourceFileName);

    vector<bool>* buffer = new vector<bool>();
    map<char, vector<bool> >::iterator tableIt;

    char ch;
    while (sourceFileStream->get(ch)) {

        vector<bool> codeToBuffer = (*symbolCodeTable)[ch];

        for (int i = 0; i < codeToBuffer.size(); i++)
            buffer->push_back(codeToBuffer[i]);

    }

    closeFile(sourceFileStream);

    int bufferSize = buffer->size();

    while(buffer->size() % 8 != 0)
        buffer->push_back(0);

    ofstream* resultCodedFileStream = openFileForWrite(outputCodedFileName);

    int sizeSymbolCodeTable = symbolCodeTable->size();

    resultCodedFileStream->write((char*) &sizeSymbolCodeTable, sizeof(int));

    for(tableIt = symbolCodeTable->begin(); tableIt != symbolCodeTable->end(); tableIt++){
        //1 byte writing char from map
        resultCodedFileStream->put((*tableIt).first);

        vector<bool> symbolCode = (*tableIt).second;

        //2 byte write size of symbolCode vector
        resultCodedFileStream->put(symbolCode.size());
        while(symbolCode.size() != sizeof(int) * 8){
            symbolCode.push_back(0);
        }

        int symbolCodeInIneger = 0;
        for(int i = 0; i < symbolCode.size(); i++){
            symbolCodeInIneger = (symbolCodeInIneger << 1) | symbolCode[i];
        }

        resultCodedFileStream->write((char*) &symbolCodeInIneger, sizeof(int));
    }

    resultCodedFileStream->write((char*) &bufferSize, sizeof(int));

    int counter = 0;
    char codedByte = 0;
    for (int i = 0; i < buffer->size(); i++){
        codedByte = (codedByte << 1) | (*buffer)[i];
        counter++;
        if (counter  == 8) {
            resultCodedFileStream->put(codedByte);
            codedByte = 0;
            counter = 0;
        }
    }

    closeFile(resultCodedFileStream);

}

void compressFile(char* sourceFileName, char* outputCodedFileName){

    ifstream* sourceFileStream = openFileForRead(sourceFileName);
    cout<<"compress start..."<<endl;

    map<char, int>* symbolFrequencyInFile = createSymbolFrequencyInFile(sourceFileStream);

    Node* rootNodeHaffmanTree = buildTree(symbolFrequencyInFile);

    map<char, vector<bool> >* symbolCodeTable = buildSymbolCodeTable(rootNodeHaffmanTree);

    createCompresedFile(sourceFileName, symbolCodeTable, outputCodedFileName);
    cout<<"compress completed..."<<endl<<endl;
}

map<vector<bool>, char>* restoreCodeTable(char* sourceCodedFileName){

    map<vector<bool>, char>* result = new map<vector<bool>, char>();
    ifstream* sourceDecompresFileStream = openFileForRead(sourceCodedFileName);

    // Read the table size from the input file
    int sizeSymbolCodeTable = 0;
    sourceDecompresFileStream->read((char*) & sizeSymbolCodeTable, sizeof(int));

    // Read the Huffman table from the input file(6 bytes for each element)
    for (int i = 0; i < sizeSymbolCodeTable; i++) {
        // 1st byte - The current char ASCII-code
        char symbol;
        sourceDecompresFileStream->get(symbol);

        // 2nd byte - The current char Huffman code size
        char codeLength = 0;
        sourceDecompresFileStream->get(codeLength);
        // 3rd - 6th bytes - The filled with zeros current char Huffman code

        unsigned int codedInt = 0;
        sourceDecompresFileStream->read((char*) &codedInt, sizeof(int));

        // Restore the current char Huffman code to original size
        vector<bool> code;
        for (int j = 0; j < codeLength; j++) {
            code.push_back(codedInt & 0b10000000000000000000000000000000);
            codedInt <<= 1;
        }

        (*result)[code] = symbol;

    }
    closeFile(sourceDecompresFileStream);
    return result;
}

void createDecompressFile(char* sourceCodedFileName, char* resultDecodedFileName, map<vector<bool>, char >* symbolCodeTable){

    ifstream* sourceDecompresFileStream = openFileForRead(sourceCodedFileName);

    int sizeSymbolCodeTable = symbolCodeTable->size();
    sourceDecompresFileStream->seekg(sizeSymbolCodeTable * 6 + 4);

    unsigned int bufferSize = 0;
    sourceDecompresFileStream->read((char*) &bufferSize, sizeof(int));

    vector<bool> decodeMessage;
    char oneByte = 0;
    while(!sourceDecompresFileStream->eof()){
        sourceDecompresFileStream->get(oneByte);
        for (int i = 0; i < 8; i++){
            decodeMessage.push_back(oneByte & 0b10000000);
            oneByte <<= 1;
        }
    }
    closeFile(sourceDecompresFileStream);

    while(decodeMessage.size() != bufferSize)
        decodeMessage.pop_back();

    ofstream* resultFileStream = openFileForWrite(resultDecodedFileName);

    vector<bool> codeSymbol;

    for(vector<bool>::iterator it = decodeMessage.begin(); it != decodeMessage.end(); it++){
        codeSymbol.push_back(*it);
        if (symbolCodeTable->count(codeSymbol)){
            resultFileStream->put((*symbolCodeTable)[codeSymbol]);
            codeSymbol.clear();
        }
    }

    delete symbolCodeTable;
    closeFile(resultFileStream);
}

void decompressFile(char* sourceCodedFileName, char* resultDecodedFileName){

    cout<< "decompress start..."<< endl;

    map<vector<bool>, char>* symbolCodeTable = restoreCodeTable(sourceCodedFileName);

    createDecompressFile(sourceCodedFileName, resultDecodedFileName, symbolCodeTable);

    cout<< "decompress end."<< endl<<endl;
}
int main(int argc, char *argv[])
{
    compressFile("000057.jpg","output.cmp");
    decompressFile("output.cmp", "New_000057.jpg");
    cout<< "end program...";
    return 0;
}
