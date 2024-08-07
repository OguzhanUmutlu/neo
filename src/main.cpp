#include "lexer.h"
#include "parser.h"
#include <sstream>
#include <fstream>
#include <string>
#include <iostream>


using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "usage: neolang <file>" << endl;
        return 1;
    }
    auto filename = argv[1];
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "error: could not open file '" << filename << "'" << endl;
        return 1;
    }

    stringstream buffer;
    buffer << file.rdbuf();

    auto lexer = new Lexer(filename, buffer.str());
    lexer->tokenize();
    lexer->groupTokens();
    auto parser = new Parser(lexer);
    parser->parse();

    lexer->dump();

    lexer->free();
    parser->free();

    file.close();
    return 0;
}