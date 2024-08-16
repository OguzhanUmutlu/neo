#include "lexer.hpp"
#include "parser.hpp"
#include "compiler.hpp"
#include "error.hpp"
#include <sstream>
#include <fstream>
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

    auto lexer = Lexer(buffer.str(), filename);
    lexer.tokenize();
    lexer.groupTokens();

    auto parser = Parser(lexer);
    parser.parse();

    auto compiler = Compiler(parser);
    compiler.compile();

    lexer.freeTokens();

    file.close();
    return 0;
}